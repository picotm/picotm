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

/*
 * Public interface
 */

void
error_module_save_errno(struct picotm_error* error)
{
    struct error_tx* error_tx = get_error_tx(error);
    if (picotm_error_is_set(error)) {
        return;
    }
    /* We only have to save 'errno' once per transaction. */
    if (error_tx_errno_saved(error_tx)) {
        return;
    }
    error_tx_save_errno(error_tx);
}

void
error_module_set_error_recovery(enum picotm_libc_error_recovery recovery,
                                struct picotm_error* error)
{
    struct error_tx* error_tx = get_error_tx(error);
    if (picotm_error_is_set(error)) {
        return;
    }
    return error_tx_set_error_recovery(error_tx, recovery);
}

enum picotm_libc_error_recovery
error_module_get_error_recovery(struct picotm_error* error)
{
    struct error_tx* error_tx = get_error_tx(error);
    if (picotm_error_is_set(error)) {
        return PICOTM_LIBC_ERROR_RECOVERY_AUTO;
    }
    return error_tx_get_error_recovery(error_tx);
}
