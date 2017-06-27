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

#include "picotm/picotm-lib-rwlock.h"
#include <assert.h>
#include <stdint.h>
#include "picotm/picotm-error.h"

PICOTM_EXPORT
void
picotm_rwlock_init(struct picotm_rwlock* self)
{
    assert(self);

    atomic_init(&self->n, 0);
}

PICOTM_EXPORT
void
picotm_rwlock_uninit(struct picotm_rwlock* self)
{
    assert(self);
}

PICOTM_EXPORT
void
picotm_rwlock_try_rdlock(struct picotm_rwlock* self,
                         struct picotm_error* error)
{
    assert(self);

    do {
        uint8_t n = atomic_load_explicit(&self->n, memory_order_relaxed);

        if (n == UINT8_MAX) {
            /* writer present; cannot read-lock */
            picotm_error_set_conflicting(error, NULL);
            return;
        } else if (n == (UINT8_MAX - 1)) {
            /* maximum number of readers reached; cannot read-lock */
            picotm_error_set_conflicting(error, NULL);
            return;
        }

        /* The 'weak compare-exchange' might be faster on some platforms. */
        bool succ =
            atomic_compare_exchange_weak_explicit(&self->n, &n, n + 1,
                                                  memory_order_relaxed,
                                                  memory_order_relaxed);
        if (succ) {
            return;
        }

    } while (true);
}

PICOTM_EXPORT
void
picotm_rwlock_try_wrlock(struct picotm_rwlock* self, bool upgrade,
                         struct picotm_error* error)
{
    assert(self);

    /* Expect us to be the only reader if we're upgrading, otherwise expect
     * us to be the only transaction at all. */
    uint8_t expected = upgrade ? 1 : 0;

    bool succ = atomic_compare_exchange_strong_explicit(&self->n,
                                                        &expected, UINT8_MAX,
                                                        memory_order_relaxed,
                                                        memory_order_relaxed);
    if (!succ) {
        picotm_error_set_conflicting(error, NULL);
        return;
    }
}

PICOTM_EXPORT
void
picotm_rwlock_unlock(struct picotm_rwlock* self)
{
    assert(self);

    uint8_t n = atomic_load_explicit(&self->n, memory_order_relaxed);

    if (n == UINT8_MAX) {
        /* We're a writer; set counter to zero. */
        atomic_store_explicit(&self->n, 0, memory_order_relaxed);
        return;
    }

    assert(n != 0);

    /* We're a reader; decrement counter. */
    atomic_fetch_sub_explicit(&self->n, 1, memory_order_relaxed);
}
