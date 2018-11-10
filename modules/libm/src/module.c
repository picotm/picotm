/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
fpu_module_init(struct fpu_module* self, unsigned long module_id,
                struct picotm_error* error)
{
    assert(self);

    fpu_tx_init(&self->tx, module_id, error);
}

static void
fpu_module_uninit(struct fpu_module* self)
{
    assert(self);

    fpu_tx_uninit(&self->tx);
}

static void
fpu_module_undo(struct fpu_module* self, struct picotm_error* error)
{
    assert(self);

    fpu_tx_undo(&self->tx, error);
}

static void
fpu_module_finish(struct fpu_module* self, struct picotm_error* error)
{
    assert(self);

    fpu_tx_finish(&self->tx);
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
    struct fpu_module* module = data;
    fpu_module_undo(module, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    struct fpu_module* module = data;
    fpu_module_finish(module, error);
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

    fpu_module_init(module, module_id, error);
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
fpu_module_save_fenv(struct picotm_error* error)
{
    struct fpu_tx* fpu_tx = get_non_null_fpu_tx();

    /* We have to save the floating-point enviroment
     * only once per transaction. */
    if (fpu_tx_fenv_saved(fpu_tx)) {
        return;
    }

    fpu_tx_save_fenv(fpu_tx, error);
}

void
fpu_module_save_fexcept(struct picotm_error* error)
{
    struct fpu_tx* fpu_tx = get_non_null_fpu_tx();

    /* We have to save the floating-point status
     * flags only once per transaction. */
    if (fpu_tx_fexcept_saved(fpu_tx)) {
        return;
    }

    fpu_tx_save_fexcept(fpu_tx, error);
}
