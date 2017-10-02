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
#include "picotm/picotm-module.h"

void
tx_shared_init(struct tx_shared* self, struct picotm_error* error)
{
    assert(self);

    picotm_os_rwlock_init(&self->exclusive_tx_lock, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    self->exclusive_tx = NULL;

    picotm_lock_manager_init(&self->lm, error);
    if (picotm_error_is_set(error)) {
        goto err_lock_manager_init;
    }

    return;

err_lock_manager_init:
    picotm_os_rwlock_uninit(&self->exclusive_tx_lock);
}

void
tx_shared_uninit(struct tx_shared* self)
{
    assert(self);

    picotm_lock_manager_uninit(&self->lm);
    picotm_os_rwlock_uninit(&self->exclusive_tx_lock);
}

void
tx_shared_make_irrevocable(struct tx_shared* self, struct tx* exclusive_tx,
                           struct picotm_error* error)
{
    assert(self);
    assert(exclusive_tx);

    picotm_os_rwlock_wrlock(&self->exclusive_tx_lock, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    self->exclusive_tx = exclusive_tx;
}

void
tx_shared_wait_irrevocable(struct tx_shared* self, struct picotm_error* error)
{
    assert(self);

    picotm_os_rwlock_rdlock(&self->exclusive_tx_lock, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
tx_shared_release_irrevocability(struct tx_shared* self)
{
    assert(self);

    self->exclusive_tx = NULL;
    picotm_os_rwlock_unlock(&self->exclusive_tx_lock);
}
