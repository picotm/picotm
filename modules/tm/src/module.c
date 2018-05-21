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
apply(struct tm_module* module, struct picotm_error* error)
{
    tm_vmem_tx_apply(&module->tx, error);
}

static void
undo(struct tm_module* module, struct picotm_error* error)
{
    tm_vmem_tx_undo(&module->tx, error);
}

static void
finish(struct tm_module* module, struct picotm_error* error)
{
    tm_vmem_tx_finish(&module->tx, error);
}

static void
uninit(struct tm_module* module)
{
    tm_vmem_tx_release(&module->tx);
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
    apply(data, error);
}

static void
undo_cb(void* data, struct picotm_error* error)
{
    undo(data, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    finish(data, error);
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

    tm_vmem_tx_init(&module->tx, vmem, module_id);

    return;

err_picotm_register_module:
    PICOTM_GLOBAL_STATE_UNREF(vmem);
}

static void
uninit_tm_module(struct tm_module* module)
{
    uninit(module);
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
