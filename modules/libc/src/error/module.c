/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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

#include "module.h"
#include "picotm/picotm-lib-state.h"
#include "picotm/picotm-lib-thread-state.h"
#include "picotm/picotm-module.h"
#include <assert.h>
#include <stdlib.h>
#include "error_tx.h"

/*
 * Module interface
 */

struct error_module {
    struct error_tx tx;
};

static void
error_module_init(struct error_module* self, unsigned long module_id)
{
    assert(self);

    error_tx_init(&self->tx, module_id);
}

static void
error_module_uninit(struct error_module* self)
{
    assert(self);

    error_tx_uninit(&self->tx);
}

static void
error_module_undo(struct error_module* self, struct picotm_error* error)
{
    assert(self);

    error_tx_undo(&self->tx, error);
}

static void
error_module_finish(struct error_module* self, struct picotm_error* error)
{
    assert(self);

    error_tx_finish(&self->tx, error);
}

/*
 * Thread-local state
 */

PICOTM_STATE(error_module, struct error_module);
PICOTM_STATE_STATIC_DECL(error_module, struct error_module)
PICOTM_THREAD_STATE_STATIC_DECL(error_module)

static void
undo_cb(void* data, struct picotm_error* error)
{
    struct error_module* module = data;
    error_module_undo(module, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    struct error_module* module = data;
    error_module_finish(module, error);
}

static void
release_cb(void* data)
{
    PICOTM_THREAD_STATE_RELEASE(error_module);
}

static void
init_error_module(struct error_module* module, struct picotm_error* error)
{
    static const struct picotm_module_ops s_ops = {
        .undo = undo_cb,
        .finish = finish_cb,
        .release = release_cb
    };

    unsigned long module_id = picotm_register_module(&s_ops, module, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    error_module_init(module, module_id);
}

static void
uninit_error_module(struct error_module* module)
{
    error_module_uninit(module);
}

PICOTM_STATE_STATIC_IMPL(error_module, struct error_module,
                         init_error_module,
                         uninit_error_module)
PICOTM_THREAD_STATE_STATIC_IMPL(error_module)

static struct error_tx*
get_error_tx(struct picotm_error* error)
{
    struct error_module* module = PICOTM_THREAD_STATE_ACQUIRE(error_module,
                                                              true, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return &module->tx;
}

static struct error_tx*
get_non_null_error_tx(void)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        struct error_tx* error_tx = get_error_tx(&error);

        if (!picotm_error_is_set(&error)) {
            /* assert here as there's no legal way that error_tx
             * could be NULL */
            assert(error_tx);
            return error_tx;
        }

        picotm_recover_from_error(&error);
    } while (true);
}

/*
 * Public interface
 */

void
error_module_save_errno()
{
    struct error_tx* error_tx = get_non_null_error_tx();

    /* We only have to save 'errno' once per transaction. */
    if (error_tx_errno_saved(error_tx)) {
        return;
    }

    error_tx_save_errno(error_tx);
}

void
error_module_set_error_recovery(enum picotm_libc_error_recovery recovery)
{
    return error_tx_set_error_recovery(get_non_null_error_tx(), recovery);
}

enum picotm_libc_error_recovery
error_module_get_error_recovery()
{
    return error_tx_get_error_recovery(get_non_null_error_tx());
}
