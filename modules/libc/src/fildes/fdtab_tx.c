/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "fdtab_tx.h"
#include "picotm/picotm-error.h"
#include <assert.h>
#include "fdtab.h"

void
fdtab_tx_init(struct fdtab_tx* self)
{
    assert(self);

    picotm_rwstate_init(&self->rwstate);
}

void
fdtab_tx_uninit(struct fdtab_tx* self)
{
    assert(self);

    picotm_rwstate_uninit(&self->rwstate);
}

void
fdtab_tx_try_rdlock(struct fdtab_tx* self, struct picotm_error* error)
{
    assert(self);

    fdtab_try_rdlock(&self->rwstate, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fdtab_tx_try_wrlock(struct fdtab_tx* self, struct picotm_error* error)
{
    assert(self);

    fdtab_try_wrlock(&self->rwstate, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

struct fd*
fdtab_tx_ref_fildes(struct fdtab_tx* self, int fildes,
                    struct picotm_error* error)
{
    assert(self);

    fdtab_try_rdlock(&self->rwstate, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct fd* fd = fdtab_ref_fildes(fildes, &self->rwstate, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return fd;
}

/*
 * Module Interface
 */

void
fdtab_tx_finish(struct fdtab_tx* self)
{
    assert(self);

    fdtab_unlock(&self->rwstate);
}
