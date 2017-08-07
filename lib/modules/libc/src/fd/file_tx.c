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

#include "file_tx.h"
#include <assert.h>
#include <string.h>

void
file_tx_init(struct file_tx* self,  const struct file_tx_ops* ops)
{
    assert(self);
    assert(ops);

    memset(&self->active_list, 0, sizeof(self->active_list));

    self->ops = ops;
}

void
file_tx_uninit(struct file_tx* self)
{ }

enum picotm_libc_file_type
file_tx_file_type(const struct file_tx* self)
{
    assert(self);

    return self->ops->type;
}

void
file_tx_ref(struct file_tx* self)
{
    assert(self);

    self->ops->ref(self);
}

void
file_tx_unref(struct file_tx* self)
{
    assert(self);

    self->ops->unref(self);
}

/*
 * Module interfaces
 */

void
file_tx_lock(struct file_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->ops);
    assert(self->ops->lock);

    self->ops->lock(self, error);
}

void
file_tx_unlock(struct file_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->ops);
    assert(self->ops->unlock);

    self->ops->unlock(self, error);
}

void
file_tx_validate(struct file_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->ops);
    assert(self->ops->validate);

    self->ops->validate(self, error);
}

void
file_tx_update_cc(struct file_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->ops);
    assert(self->ops->update_cc);

    self->ops->update_cc(self, error);
}

void
file_tx_clear_cc(struct file_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->ops);
    assert(self->ops->clear_cc);

    self->ops->clear_cc(self, error);
}
