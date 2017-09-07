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

#include "rwcounter.h"
#include <assert.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-rwlock.h>
#include <stdbool.h>

enum {
    RWCOUNTER_WRITTEN = 0x8000000, /* flags write-lock counters */
    RWCOUNTER_BITS    = 0x7ffffff  /* counter bits */
};

void
rwcounter_init(struct rwcounter* self)
{
    assert(self);

    self->state = 0;
}

void
rwcounter_rdlock(struct rwcounter* self, struct picotm_rwlock* rwlock,
                 struct picotm_error* error)
{
    assert(self);

    if ( !(self->state & RWCOUNTER_BITS) ) {

        /* not yet read-locked */

        picotm_rwlock_try_rdlock(rwlock, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    self->state = (self->state & RWCOUNTER_WRITTEN) |
                 ((self->state & RWCOUNTER_BITS) + 1);
}

void
rwcounter_wrlock(struct rwcounter* self, struct picotm_rwlock* rwlock,
                 struct picotm_error* error)
{
    assert(self);

    if ( !(self->state & RWCOUNTER_WRITTEN) ) {

        /* not yet write-locked */

        bool upgrade = !!(self->state & RWCOUNTER_BITS);

        picotm_rwlock_try_wrlock(rwlock, upgrade, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    self->state = RWCOUNTER_WRITTEN | ((self->state & RWCOUNTER_BITS) + 1);
}

void
rwcounter_unlock(struct rwcounter* self, struct picotm_rwlock* rwlock)
{
    assert(self);
    assert(self->state & RWCOUNTER_BITS);

    self->state = (self->state & RWCOUNTER_WRITTEN) |
                 ((self->state & RWCOUNTER_BITS) - 1);

    if (self->state & RWCOUNTER_BITS) {
        return;
    }

    /* last lock of this transaction; unlock */
    picotm_rwlock_unlock(rwlock);
    self->state = 0;
}
