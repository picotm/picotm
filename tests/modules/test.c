/* Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "test.h"
#include <assert.h>
#include <stdlib.h>
#include <picotm/picotm.h>
#include "taputils.h"
#include "test_state.h"
#include "safe_pthread.h"
#include "safe_stdlib.h"
#include "safe_sys_time.h"

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

    /* Test result */
    int test_aborted;
};

/* Returns the number of milliseconds since the epoch */
static unsigned long long
getmsofday(void* tzp)
{
    struct timeval t;
    safe_gettimeofday(&t, tzp);

    return t.tv_sec * 1000 + t.tv_usec / 1000;
}

static void
cleanup_picotm_cb(void* data)
{
    picotm_release();
}

static unsigned long long
run_threads(struct thread_state* state, unsigned long nthreads,
            void* (*thread_func)(void*))
{
    struct thread_state*       beg = state;
    const struct thread_state* end = state + nthreads;

    while (beg < end) {
        beg->ntx = 0;
        beg->test_aborted = 0;
        safe_pthread_create(&beg->thread, NULL, thread_func, beg);
        ++beg;
    }

    unsigned long long ntx = 0;
    int test_aborted = 0;

    beg = state;
    end = state + nthreads;

    while (beg < end) {
        safe_pthread_join(beg->thread, NULL);
        ntx += beg->ntx;
        test_aborted |= beg->test_aborted;
        ++beg;
    }

    if (test_aborted) {
        test_abort();
    }

    return ntx;
}

/* Inner loops
 *
 * In inner-loop mode, threads syncronize before and after each
 * running n iterations of the transaction.
 */

static void
inner_loop_func_cycles(struct thread_state* state)
{
    for (; state->ntx < state->bound; ++state->ntx) {
        state->test->call(state->tid);
    }
}

static void
inner_loop_func_time(struct thread_state* state)
{
    const unsigned long long beg_ms = getmsofday(NULL);

    for (unsigned long long ms = 0;
                            ms < state->bound;
                            ms = getmsofday(NULL) - beg_ms) {
        state->test->call(state->tid);
        ++state->ntx;
    }
}

static void
inner_loop_func(struct thread_state* state)
{
    static void (* const btype_func[])(struct thread_state*) = {
        inner_loop_func_cycles,
        inner_loop_func_time
    };

    pthread_cleanup_push(cleanup_picotm_cb, NULL);

    test_begin_on_thread(state->test_aborted)

        safe_pthread_barrier_wait(state->sync_begin);
        btype_func[state->btype](state);

    test_end_on_thread

    pthread_cleanup_pop(1);
}

static void*
inner_loop_func_cb(void* arg)
{
    inner_loop_func(arg);
    return NULL;
}

static unsigned long long
run_inner_loop(enum boundary_type btype, unsigned long long bound,
               struct thread_state* state, unsigned long nthreads)
{
    return run_threads(state, nthreads, inner_loop_func_cb);
}

/* Outer loops
 *
 * In outer-loop mode, threads syncronize before and after each
 * iteration of the transaction.
 */

static void
outer_loop_func(struct thread_state* state)
{
    pthread_cleanup_push(cleanup_picotm_cb, NULL);

    test_begin_on_thread(state->test_aborted)

        safe_pthread_barrier_wait(state->sync_begin);
        state->test->call(state->tid);
        state->ntx = 1;

    test_end_on_thread

    pthread_cleanup_pop(1);
}

static void*
outer_loop_func_cb(void* arg)
{
    outer_loop_func(arg);
    return NULL;
}

static unsigned long long
run_outer_loop_cycles(unsigned long long cycles, struct thread_state* state,
                      unsigned long nthreads)
{
    unsigned long long ntx = 0;

    for (unsigned long long i = 0; i < cycles; ++i) {
        ntx += run_threads(state, nthreads, outer_loop_func_cb);
    }

    return ntx;
}

static unsigned long long
run_outer_loop_time(unsigned long long ival_ms, struct thread_state* state,
                    unsigned long nthreads)
{
    unsigned long long ntx = 0;

    const unsigned long long beg_ms = getmsofday(NULL);

    for (unsigned long long ms = 0;
                            ms < ival_ms;
                            ms = getmsofday(NULL) - beg_ms) {
        ntx += run_threads(state, nthreads, outer_loop_func_cb);
    }

    return ntx;
}

static unsigned long long
run_outer_loop(enum boundary_type btype, unsigned long long bound,
               struct thread_state* state, unsigned long nthreads)
{
    static unsigned long long (* const btype_func[])(unsigned long long,
                                                     struct thread_state*,
                                                     unsigned long) = {
        run_outer_loop_cycles,
        run_outer_loop_time
    };

    return btype_func[btype](bound, state, nthreads);
}

long long
run_test(const struct test_func* test, unsigned long nthreads,
         enum loop_mode loop, enum boundary_type btype,
         unsigned long long bound)
{
    static unsigned long long (* const loop_func[])(enum boundary_type,
                                                    unsigned long long,
                                                    struct thread_state*,
                                                    unsigned long) = {
        run_inner_loop,
        run_outer_loop
    };

    assert(test);

    /* Helgrind 3.3 does not support barriers, so you might
     * get a warning here. */
    pthread_barrier_t sync_begin;
    safe_pthread_barrier_init(&sync_begin, NULL, nthreads);

    struct thread_state* state = safe_malloc(nthreads * sizeof(state[0]));

    for (struct thread_state* s = state; s < state + nthreads; ++s) {
        s->sync_begin = &sync_begin;
        s->test = test;
        s->tid = s - state;
        s->btype = btype;
        s->bound = bound;
        s->ntx = 0;
        s->test_aborted = 0;
    }

    tap_info("Running test %s...", test->name);

    if (test->pre) {
        test->pre(nthreads, loop, btype, bound);
    }

    unsigned long long ntx = loop_func[loop](btype, bound, state, nthreads);

    if (test->post) {
        test->post(nthreads, loop, btype, bound);
    }

    free(state);
    safe_pthread_barrier_destroy(&sync_begin);

    return ntx;
}
