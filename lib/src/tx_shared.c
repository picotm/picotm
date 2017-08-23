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

#include "tx_shared.h"
#include <assert.h>
#include "picotm/picotm-error.h"

void
tx_shared_init(struct tx_shared* self, struct picotm_error* error)
{
    assert(self);

    int err = pthread_rwlock_init(&self->exclusive_tx_lock, NULL);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
    self->exclusive_tx = NULL;
}

void
tx_shared_uninit(struct tx_shared* self, struct picotm_error* error)
{
    assert(self);

    int err = pthread_rwlock_destroy(&self->exclusive_tx_lock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

void
tx_shared_make_irrevocable(struct tx_shared* self, struct tx* exclusive_tx,
                           struct picotm_error* error)
{
    assert(self);
    assert(exclusive_tx);

    int err = pthread_rwlock_wrlock(&self->exclusive_tx_lock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
    self->exclusive_tx = exclusive_tx;
}

void
tx_shared_wait_irrevocable(struct tx_shared* self, struct picotm_error* error)
{
    assert(self);

    int err = pthread_rwlock_rdlock(&self->exclusive_tx_lock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

void
tx_shared_release_irrevocability(struct tx_shared* self,
                                 struct picotm_error* error)
{
    assert(self);

    int err = pthread_rwlock_unlock(&self->exclusive_tx_lock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
    self->exclusive_tx = NULL;
}
