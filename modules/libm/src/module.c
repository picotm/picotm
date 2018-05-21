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

#include "module.h"
#include "picotm/picotm-lib-state.h"
#include "picotm/picotm-lib-thread-state.h"
#include "picotm/picotm-module.h"
#include <assert.h>
#include <stdlib.h>
#include "fpu_tx.h"

/*
 * Module interface
 */

struct fpu_module {
    struct fpu_tx tx;
};

static void
fpu_module_undo(struct fpu_module* module, struct picotm_error* error)
{
    fpu_tx_undo(&module->tx, error);
}

static void
fpu_module_finish(struct fpu_module* module, struct picotm_error* error)
{
    fpu_tx_finish(&module->tx);
}

static void
fpu_module_uninit(struct fpu_module* module)
{
    fpu_tx_uninit(&module->tx);
}

/*
 * Thread-local state
 */

PICOTM_STATE(fpu_module, struct fpu_module);
PICOTM_STATE_STATIC_DECL(fpu_module, struct fpu_module)
PICOTM_THREAD_STATE_STATIC_DECL(fpu_module)

static void
undo_cb(void* data, struct picotm_error* error)
{
    fpu_module_undo(data, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    fpu_module_finish(data, error);
}

static void
release_cb(void* data)
{
    PICOTM_THREAD_STATE_RELEASE(fpu_module);
}

static void
init_fpu_module(struct fpu_module* module, struct picotm_error* error)
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

    fpu_tx_init(&module->tx, module_id, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
uninit_fpu_module(struct fpu_module* module)
{
    fpu_module_uninit(module);
}

PICOTM_STATE_STATIC_IMPL(fpu_module, struct fpu_module,
                         init_fpu_module,
                         uninit_fpu_module)
PICOTM_THREAD_STATE_STATIC_IMPL(fpu_module)

static struct fpu_tx*
get_fpu_tx(struct picotm_error* error)
{
    struct fpu_module* module = PICOTM_THREAD_STATE_ACQUIRE(fpu_module,
                                                            true, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return &module->tx;
}

static struct fpu_tx*
get_non_null_fpu_tx(void)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        struct fpu_tx* fpu_tx = get_fpu_tx(&error);

        if (!picotm_error_is_set(&error)) {
            /* assert() here as there's no legal way that fpu_tx
             * could be NULL */
            assert(fpu_tx);
            return fpu_tx;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

/*
 * Public interface
 */

void
fpu_module_save_fenv()
{
    struct fpu_tx* fpu_tx = get_non_null_fpu_tx();

    /* We have to save the floating-point enviroment
     * only once per transaction. */
    if (fpu_tx_fenv_saved(fpu_tx)) {
        return;
    }

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        fpu_tx_save_fenv(fpu_tx, &error);

        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

void
fpu_module_save_fexcept()
{
    struct fpu_tx* fpu_tx = get_non_null_fpu_tx();

    /* We have to save the floating-point status
     * flags only once per transaction. */
    if (fpu_tx_fexcept_saved(fpu_tx)) {
        return;
    }

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        fpu_tx_save_fexcept(fpu_tx, &error);
        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}
