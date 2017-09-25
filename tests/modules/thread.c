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

#include "thread.h"
#include <picotm/picotm.h>
#include <stdbool.h>
#include <stdlib.h>
#include "safeblk.h"
#include "safe_pthread.h"
#include "safe_stdlib.h"
#include "safe_sys_time.h"

struct thread_state {

    /* OS threads */

    pthread_t thread;
    pthread_barrier_t* sync_begin;

    /* Test */

    void (*func)(unsigned int, void*);
    void* data;

    /* Test arguments */

    unsigned int       tid; /* Logical thread ID*/
    enum boundary_type btype; /* Boundary type */
    unsigned long long limit; /* Time (ms) or cycles to run */

    /* Test result */

    bool test_aborted;
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

static void
run_threads(struct thread_state* state, unsigned long nthreads,
            void* (*thread_func)(void*))
{
    struct thread_state*       beg = state;
    const struct thread_state* end = state + nthreads;

    while (beg < end) {
        beg->test_aborted = false;
        safe_pthread_create(&beg->thread, NULL, thread_func, beg);
        ++beg;
    }

    bool test_aborted = false;

    beg = state;
    end = state + nthreads;

    while (beg < end) {
        safe_pthread_join(beg->thread, NULL);
        test_aborted |= beg->test_aborted;
        ++beg;
    }

    if (test_aborted) {
        abort_safe_block();
    }
}

/* Inner loops
 *
 * In inner-loop mode, threads syncronize before and after each
 * running n iterations of the transaction.
 */

static void
inner_loop_func_cycles(struct thread_state* state)
{
    for (unsigned long long i = 0; i < state->limit; ++i) {
        state->func(state->tid, state->data);
    }
}

static void
inner_loop_func_time(struct thread_state* state)
{
    const unsigned long long beg_ms = getmsofday(NULL);

    for (unsigned long long ms = 0;
                            ms < state->limit;
                            ms = getmsofday(NULL) - beg_ms) {
        state->func(state->tid, state->data);
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

    begin_safe_block(state->test_aborted)

        safe_pthread_barrier_wait(state->sync_begin);
        btype_func[state->btype](state);

    end_safe_block

    pthread_cleanup_pop(1);
}

static void*
inner_loop_func_cb(void* arg)
{
    inner_loop_func(arg);
    return NULL;
}

static void
run_inner_loop(enum boundary_type btype, unsigned long long limit,
               struct thread_state* state, unsigned long nthreads)
{
    run_threads(state, nthreads, inner_loop_func_cb);
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

    begin_safe_block(state->test_aborted)

        safe_pthread_barrier_wait(state->sync_begin);
        state->func(state->tid, state->data);

    end_safe_block

    pthread_cleanup_pop(1);
}

static void*
outer_loop_func_cb(void* arg)
{
    outer_loop_func(arg);
    return NULL;
}

static void
run_outer_loop_cycles(unsigned long long limit_cycles,
                      struct thread_state* state,
                      unsigned long nthreads)
{
    for (unsigned long long i = 0; i < limit_cycles; ++i) {
        run_threads(state, nthreads, outer_loop_func_cb);
    }
}

static void
run_outer_loop_time(unsigned long long limit_ms, struct thread_state* state,
                    unsigned long nthreads)
{
    const unsigned long long beg_ms = getmsofday(NULL);

    for (unsigned long long ms = 0;
                            ms < limit_ms;
                            ms = getmsofday(NULL) - beg_ms) {
        run_threads(state, nthreads, outer_loop_func_cb);
    }
}

static void
run_outer_loop(enum boundary_type btype, unsigned long long limit,
               struct thread_state* state, unsigned long nthreads)
{
    static void (* const btype_func[])(unsigned long long,
                                       struct thread_state*,
                                       unsigned long) = {
        run_outer_loop_cycles,
        run_outer_loop_time
    };

    btype_func[btype](limit, state, nthreads);
}

void
spawn_threads(unsigned long nthreads,
              void (*func)(unsigned int, void*), void* data,
              enum loop_mode loop,
              enum boundary_type btype, unsigned long long limit)
{
    static void (* const loop_func[])(enum boundary_type,
                                      unsigned long long,
                                      struct thread_state*,
                                      unsigned long) = {
        run_inner_loop,
        run_outer_loop
    };

    /* Helgrind 3.3 does not support barriers, so you might
     * get a warning here. */
    pthread_barrier_t sync_begin;
    safe_pthread_barrier_init(&sync_begin, NULL, nthreads);

    struct thread_state* state = safe_malloc(nthreads * sizeof(state[0]));

    for (struct thread_state* s = state; s < state + nthreads; ++s) {
        s->sync_begin = &sync_begin;
        s->func = func;
        s->data = data;
        s->tid = s - state;
        s->btype = btype;
        s->limit = limit;
        s->test_aborted = false;
    }

    loop_func[loop](btype, limit, state, nthreads);

    free(state);
    safe_pthread_barrier_destroy(&sync_begin);
}
