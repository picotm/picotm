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
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-global-state.h"
#include "picotm/picotm-lib-shared-state.h"
#include "picotm/picotm-lib-state.h"
#include "picotm/picotm-lib-thread-state.h"
#include "picotm/picotm-module.h"
#include "vmem.h"
#include "vmem_tx.h"

/*
 * Shared state
 */

static void
init_vmem_shared_state_fields(struct tm_vmem* vmem,
                              struct picotm_error* error)
{
    tm_vmem_init(vmem);
}

static void
uninit_vmem_shared_state_fields(struct tm_vmem* vmem)
{
    tm_vmem_uninit(vmem);
}

PICOTM_SHARED_STATE(vmem, struct tm_vmem);
PICOTM_SHARED_STATE_STATIC_IMPL(vmem, struct tm_vmem,
                                init_vmem_shared_state_fields,
                                uninit_vmem_shared_state_fields)

/*
 * Global state
 */

PICOTM_GLOBAL_STATE_STATIC_IMPL(vmem)

/*
 * Module interface
 */

struct tm_module {
    struct tm_vmem_tx tx;
};

static void
tm_module_init(struct tm_module* self, struct tm_vmem* vmem,
               unsigned long module_id)
{
    tm_vmem_tx_init(&self->tx, vmem, module_id);
}

static void
tm_module_uninit(struct tm_module* self)
{
    tm_vmem_tx_uninit(&self->tx);
}

static void
tm_module_apply(struct tm_module* self, struct picotm_error* error)
{
    tm_vmem_tx_apply(&self->tx, error);
}

static void
tm_module_undo(struct tm_module* self, struct picotm_error* error)
{
    tm_vmem_tx_undo(&self->tx, error);
}

static void
tm_module_finish(struct tm_module* self, struct picotm_error* error)
{
    tm_vmem_tx_finish(&self->tx, error);
}

/*
 * Thread-local state
 */

PICOTM_STATE(tm_module, struct tm_module);
PICOTM_STATE_STATIC_DECL(tm_module, struct tm_module)
PICOTM_THREAD_STATE_STATIC_DECL(tm_module)

static void
apply_cb(void* data, struct picotm_error* error)
{
    struct tm_module* module = data;
    tm_module_apply(data, error);
}

static void
undo_cb(void* data, struct picotm_error* error)
{
    struct tm_module* module = data;
    tm_module_undo(data, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    struct tm_module* module = data;
    tm_module_finish(data, error);
}

static void
release_cb(void* data)
{
    PICOTM_THREAD_STATE_RELEASE(tm_module);
}

static void
init_tm_module(struct tm_module* module, struct picotm_error* error)
{
    static const struct picotm_module_ops s_ops = {
        .apply = apply_cb,
        .undo = undo_cb,
        .finish = finish_cb,
        .release = release_cb
    };

    struct tm_vmem* vmem = PICOTM_GLOBAL_STATE_REF(vmem, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    unsigned long module_id = picotm_register_module(&s_ops, module, error);
    if (picotm_error_is_set(error)) {
        goto err_picotm_register_module;
    }

    tm_module_init(module, vmem, module_id);

    return;

err_picotm_register_module:
    PICOTM_GLOBAL_STATE_UNREF(vmem);
}

static void
uninit_tm_module(struct tm_module* module)
{
    tm_module_uninit(module);
    PICOTM_GLOBAL_STATE_UNREF(vmem);
}

PICOTM_STATE_STATIC_IMPL(tm_module, struct tm_module,
                         init_tm_module,
                         uninit_tm_module)
PICOTM_THREAD_STATE_STATIC_IMPL(tm_module)

static struct tm_vmem_tx*
get_vmem_tx(struct picotm_error* error)
{
    struct tm_module* module = PICOTM_THREAD_STATE_ACQUIRE(tm_module, true,
                                                           error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return &module->tx;
}

static struct tm_vmem_tx*
get_non_null_vmem_tx(void)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        struct tm_vmem_tx* vmem_tx = get_vmem_tx(&error);

        if (!picotm_error_is_set(&error)) {
            assert(vmem_tx);
            return vmem_tx;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

/*
 * Public interface
 */

void
tm_module_load(uintptr_t addr, void* buf, size_t siz)
{
    struct tm_vmem_tx* vmem_tx = get_non_null_vmem_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        tm_vmem_tx_ld(vmem_tx, addr, buf, siz, &error);

        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

void
tm_module_store(uintptr_t addr, const void* buf, size_t siz)
{
    struct tm_vmem_tx* vmem_tx = get_non_null_vmem_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        tm_vmem_tx_st(vmem_tx, addr, buf, siz, &error);

        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

void
tm_module_loadstore(uintptr_t laddr, uintptr_t saddr, size_t siz)
{
    struct tm_vmem_tx* vmem_tx = get_non_null_vmem_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        tm_vmem_tx_ldst(vmem_tx, laddr, saddr, siz, &error);

        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

void
tm_module_privatize(uintptr_t addr, size_t siz, unsigned long flags)
{
    struct tm_vmem_tx* vmem_tx = get_non_null_vmem_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        tm_vmem_tx_privatize(vmem_tx, addr, siz, flags, &error);

        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

void
tm_module_privatize_c(uintptr_t addr, int c, unsigned long flags)
{
    struct tm_vmem_tx* vmem_tx = get_non_null_vmem_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        tm_vmem_tx_privatize_c(vmem_tx, addr, c, flags, &error);

        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}
