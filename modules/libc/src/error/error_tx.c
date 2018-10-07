/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
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
 *
 * SPDX-License-Identifier: MIT
 */

#include "error_tx.h"
#include <errno.h>

enum {
    ERROR_TX_FLAG_ERRNO_SAVED = 1 << 0
};

void
error_tx_init(struct error_tx* self, unsigned long module)
{
    self->module = module;
    self->recovery = PICOTM_LIBC_ERROR_RECOVERY_AUTO;
    self->flags = 0;
    self->saved_errno = 0;
}

void
error_tx_uninit(struct error_tx* self)
{ }

void
error_tx_set_error_recovery(struct error_tx* self,
                            enum picotm_libc_error_recovery recovery)
{
    self->recovery = recovery;
}

enum picotm_libc_error_recovery
error_tx_get_error_recovery(const struct error_tx* self)
{
    return self->recovery;
}

bool
error_tx_errno_saved(const struct error_tx* self)
{
    return !!(self->flags & ERROR_TX_FLAG_ERRNO_SAVED);
}

void
error_tx_save_errno(struct error_tx* self)
{
    if (self->flags & ERROR_TX_FLAG_ERRNO_SAVED) {
        return;
    }

    self->saved_errno = errno;
    self->flags |= ERROR_TX_FLAG_ERRNO_SAVED;
}

void
error_tx_undo(struct error_tx* self, struct picotm_error* error)
{
    if (self->flags & ERROR_TX_FLAG_ERRNO_SAVED) {
        errno = self->saved_errno;
    }
}

void
error_tx_finish(struct error_tx* self, struct picotm_error* error)
{
    self->flags = 0; /* marks errno as 'not saved' */
}
