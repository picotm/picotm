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

#include "safe_pthread.h"
#include "safeblk.h"
#include "taputils.h"

int
safe_pthread_barrier_destroy(pthread_barrier_t* barrier)
{
    int err = pthread_barrier_destroy(barrier);
    if (err) {
        tap_error_errno("pthread_barrier_destroy()", err);
        abort_safe_block();
    }
    return err;
}

int
safe_pthread_barrier_init(pthread_barrier_t* restrict barrier,
                          const pthread_barrierattr_t* restrict attr,
                          unsigned count)
{
    int err = pthread_barrier_init(barrier, attr, count);
    if (err) {
        tap_error_errno("pthread_barrier_init()", err);
        abort_safe_block();
    }
    return err;
}

int
safe_pthread_barrier_wait(pthread_barrier_t* barrier)
{
    int err = pthread_barrier_wait(barrier);
    if (err && err != PTHREAD_BARRIER_SERIAL_THREAD) {
        tap_error_errno("pthread_barrier_wait()", err);
        abort_safe_block();
    }
    return err;
}

int
safe_pthread_create(pthread_t* thread, const pthread_attr_t* attr,
                    void* (*start_routine)(void*), void* arg)
{
    int err = pthread_create(thread, attr, start_routine, arg);
    if (err) {
        tap_error_errno("pthread_create()", err);
        abort_safe_block();
    }
    return err;
}

int
safe_pthread_join(pthread_t thread, void** retval)
{
    int err = pthread_join(thread, retval);
    if (err) {
        tap_error_errno("pthread_join()", err);
        abort_safe_block();
    }
    return err;
}

int
safe_pthread_mutex_lock(pthread_mutex_t* mutex)
{
    int err = pthread_mutex_lock(mutex);
    if (err) {
        tap_error_errno("pthread_mutex_lock()", err);
        abort_safe_block();
    }
    return err;
}

int
safe_pthread_mutex_unlock(pthread_mutex_t* mutex)
{
    int err = pthread_mutex_unlock(mutex);
    if (err) {
        tap_error_errno("pthread_mutex_unlock()", err);
        abort_safe_block();
    }
    return err;
}

int
safe_pthread_yield()
{
    int err = pthread_yield();
    if (err) {
        tap_error_errno("pthread_mutex_yield()", err);
        abort_safe_block();
    }
    return err;
}
