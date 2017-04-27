/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "test.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <picotm/picotm.h>

struct thread_state {

    /* OS threads */

    pthread_t thread;
    pthread_barrier_t* sync_begin;

    /* Test */

    const struct test_func* test;

    /* Test arguments */

    unsigned int       tid; /* Logical thread ID*/
    enum boundary_type btype; /* Boundary type */
    unsigned long      bound; /* Time (ms) or Cycles to run */
    unsigned long long ntx; /* Number of succesful transactions, return value */
};

/* Returns the number of milliseconds since the epoch */
static unsigned long long
getmsofday(void* tzp)
{
    struct timeval t;
    gettimeofday(&t, tzp);

    return t.tv_sec * 1000 + t.tv_usec / 1000;
}

static void
cleanup_picotm_cb(void* data)
{
    picotm_release();
}

static long long
run_loop_iteration(const struct test_func *test,
                   enum boundary_type btype, int bound,
                   struct thread_state* state, unsigned long nthreads,
                   void* (*thread_func)(void*))
{
    /* Helgrind 3.3 does not support barriers, so you might
     * get a warning here. */
    pthread_barrier_t sync_begin;
    int err = pthread_barrier_init(&sync_begin, NULL, nthreads);
    if (err) {
        errno = err;
        perror("pthread_barrier_init");
        return -err;
    }

    for (struct thread_state* s = state; s < state + nthreads; ++s) {

        s->sync_begin = &sync_begin;
        s->test = test;
        s->tid = s - state;
        s->btype = btype;
        s->bound = bound;
        s->ntx = 0;

        int err = pthread_create(&s->thread, NULL, thread_func, s);
        if (err) {
            errno = err;
            perror("pthread_create");
            abort();
        }
    }

    long long ntx = 0;

    for (struct thread_state* s = state; s < state + nthreads; ++s) {
        int err = pthread_join(s->thread, NULL);
        if (err) {
            errno = err;
            perror("pthread_join");
            abort();
        }
        ntx += s->ntx;
    }

    err = pthread_barrier_destroy(&sync_begin);
    if (err) {
        errno = err;
        perror("pthread_barrier_destroy");
    }

    return ntx;
}

/* Inner loops
 */

static int
inner_loop_func_cycles(struct thread_state* state)
{
    for (state->ntx = 0; state->ntx < state->bound; ++state->ntx) {
        state->test->call(state->tid);
    }

    return 0;
}

static int
inner_loop_func_time(struct thread_state* state)
{
    state->ntx = 0;

    const unsigned long long beg_ms = getmsofday(NULL);

    for (unsigned long long ms = 0;
                            ms < state->bound;
                            ms = getmsofday(NULL) - beg_ms) {
        state->test->call(state->tid);
        ++state->ntx;
    }

    return 0;
}

static int
inner_loop_func(struct thread_state* state)
{
    static int (* const btype_func[])(struct thread_state* state) = {
        inner_loop_func_cycles,
        inner_loop_func_time
    };

    int res;

    pthread_cleanup_push(cleanup_picotm_cb, NULL);

        res = pthread_barrier_wait(state->sync_begin);
        if (res && res != PTHREAD_BARRIER_SERIAL_THREAD) {
            errno = res;
            perror("pthread_barrier_wait");
            return -res;
        }

        res = btype_func[state->btype](state);

    pthread_cleanup_pop(1);

    return res;
}

static void*
inner_loop_func_cb(void* arg)
{
    int res = inner_loop_func(arg);
    if (res < 0) {
        abort();
    }
    return NULL;
}

static long long
run_inner_loop(const struct test_func *test, enum boundary_type btype,
               unsigned long long bound, struct thread_state* state,
               unsigned long nthreads, int (*logmsg)(const char*, ...))
{
    logmsg("Running test %s...\n", test->name);

    long long res = run_loop_iteration(test, btype, bound, state,
                                       nthreads, inner_loop_func_cb);
    if (res < 0) {
        abort();
    }
    long long ntx = res;

    return ntx;
}

/* Outer loops
 */

static int
outer_loop_func(struct thread_state* state)
{
    pthread_cleanup_push(cleanup_picotm_cb, NULL);

        int res = pthread_barrier_wait(state->sync_begin);
        if (res && res != PTHREAD_BARRIER_SERIAL_THREAD) {
            errno = res;
            perror("pthread_barrier_wait");
            return -res;
        }

        state->test->call(state->tid);
        state->ntx = 1;

    pthread_cleanup_pop(1);

    return 0;
}

static void*
outer_loop_func_cb(void* arg)
{
    int res = outer_loop_func(arg);
    if (res < 0) {
        abort();
    }
    return NULL;
}

static long long
run_outer_loop_iteration(const struct test_func *test,
                         enum boundary_type btype, int bound,
                         struct thread_state* state, unsigned long nthreads)
{
    return run_loop_iteration(test, btype, bound, state, nthreads,
                              outer_loop_func_cb);
}

static long
run_outer_loop_cycles(const struct test_func *test, unsigned long long cycles,
                      struct thread_state* state, unsigned long nthreads,
                      int (*logmsg)(const char*, ...))
{
    long long ntx = 0;

    for (unsigned long long i = 0; i < cycles; ++i) {

        logmsg("Running test %s [%llu of %llu]...\n", test->name, 1 + i, cycles);

        long long res = run_outer_loop_iteration(test, BOUND_CYCLES, cycles,
                                                 state, nthreads);
        if (res < 0) {
            abort();
        }
        ntx += res;
    }

    return ntx;
}

static long
run_outer_loop_time(const struct test_func *test, unsigned long long ival_ms,
                    struct thread_state* state, unsigned long nthreads,
                    int (*logmsg)(const char*, ...))
{
    logmsg("Running test %s [for next %d ms]...\n", test->name, ival_ms);

    long long ntx = 0;

    const unsigned long long beg_ms = getmsofday(NULL);

    for (unsigned long long ms = 0;
                            ms < ival_ms;
                            ms = getmsofday(NULL) - beg_ms) {

        long long res = run_outer_loop_iteration(test, BOUND_TIME, ival_ms,
                                                 state, nthreads);
        if (res < 0) {
            abort();
        }
        ntx += res;
    }

    return ntx;
}

static long long
run_outer_loop(const struct test_func* test, enum boundary_type btype,
               unsigned long long bound,  struct thread_state* state,
               unsigned long nthreads, int (*logmsg)(const char*, ...))
{
    static long (* const btype_func[])(const struct test_func*,
                                       unsigned long long,
                                       struct thread_state*,
                                       unsigned long,
                                       int (*)(const char*, ...)) = {
        run_outer_loop_cycles,
        run_outer_loop_time
    };

    return btype_func[btype](test, bound, state, nthreads, logmsg);
}

long long
run_test(const struct test_func* test, unsigned long nthreads,
         enum loop_mode loop, enum boundary_type btype,
         unsigned long long bound, int (*logmsg)(const char*, ...))
{
    static long long (* const loop_func[])(const struct test_func*,
                                           enum boundary_type,
                                           unsigned long long,
                                           struct thread_state*,
                                           unsigned long,
                                           int (*)(const char*, ...)) = {
        run_inner_loop,
        run_outer_loop
    };

    struct thread_state* state = malloc(nthreads * sizeof(state[0]));
    if (!state) {
        perror("malloc");
        return -errno;
    }

    if (test->pre) {
        test->pre(nthreads, loop, btype, bound, logmsg);
    }

    long long res = loop_func[loop](test, btype, bound, state, nthreads,
                                    logmsg);
    if (res < 0) {
        goto err_loop_func;
    }
    long long ntx = res;

    if (test->post) {
        test->post(nthreads, loop, btype, bound, logmsg);
    }

    free(state);

    return ntx;

err_loop_func:
    free(state);
    return res;
}
