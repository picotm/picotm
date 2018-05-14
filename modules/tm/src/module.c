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
#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-shared-ref-obj.h"
#include "picotm/picotm-lib-spinlock.h"
#include "picotm/picotm-module.h"
#include "vmem.h"
#include "vmem_tx.h"

/*
 * Shared state
 */

struct tm_shared_state {
    struct picotm_shared_ref16_obj ref_obj;
    struct tm_vmem vmem;
};

#define TM_SHARED_STATE_INITIALIZER                 \
{                                                   \
    .ref_obj = PICOTM_SHARED_REF16_OBJ_INITIALIZER  \
}

static void
init_tm_shared_state_fields(struct tm_shared_state* shared)
{
    tm_vmem_init(&shared->vmem);
}

static void
uninit_tm_shared_state_fields(struct tm_shared_state* shared)
{
    tm_vmem_uninit(&shared->vmem);
}

static void
first_ref_tm_shared_state_cb(struct picotm_shared_ref16_obj* ref_obj,
                             void* data, struct picotm_error* error)
{
    struct tm_shared_state* shared =
        picotm_containerof(ref_obj, struct tm_shared_state, ref_obj);
    init_tm_shared_state_fields(shared);
}

static void
tm_shared_state_ref(struct tm_shared_state* self, struct picotm_error* error)
{
    picotm_shared_ref16_obj_up(&self->ref_obj, NULL, NULL,
                               first_ref_tm_shared_state_cb,
                               error);
    if (picotm_error_is_set(error)) {
        return;
    }
};

static void
final_ref_tm_shared_state_cb(struct picotm_shared_ref16_obj* ref_obj,
                             void* data, struct picotm_error* error)
{
    struct tm_shared_state* shared =
        picotm_containerof(ref_obj, struct tm_shared_state, ref_obj);
    uninit_tm_shared_state_fields(shared);
}

static void
tm_shared_state_unref(struct tm_shared_state* self)
{
    picotm_shared_ref16_obj_down(&self->ref_obj, NULL, NULL,
                                 final_ref_tm_shared_state_cb);
}

/*
 * Global state
 */

/* Returns the statically allocated global state. Callers *must* already
 * hold a reference. */
static struct tm_shared_state*
get_tm_global_state(void)
{
    static struct tm_shared_state s_global = TM_SHARED_STATE_INITIALIZER;
    return &s_global;
}

static struct tm_shared_state*
ref_tm_global_state(struct picotm_error* error)
{
    struct tm_shared_state* global = get_tm_global_state();

    tm_shared_state_ref(global, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return global;
};

static void
unref_tm_global_state(void)
{
    struct tm_shared_state* global = get_tm_global_state();
    tm_shared_state_unref(global);
}

/*
 * Module interface
 */

struct tm_module {
    struct tm_vmem_tx tx;

    /* True if module structure has been initialized, false otherwise */
    bool is_initialized;
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
    module->is_initialized = false;

    unref_tm_global_state();
}

/*
 * Thread-local data
 */

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
uninit_cb(void* data)
{
    uninit(data);
}

static struct tm_vmem_tx*
get_vmem_tx(bool initialize, struct picotm_error* error)
{
    static const struct picotm_module_ops g_ops = {
        .apply = apply_cb,
        .undo = undo_cb,
        .finish = finish_cb,
        .uninit = uninit_cb
    };
    static __thread struct tm_module t_module;

    if (t_module.is_initialized) {
        return &t_module.tx;
    } else if (!initialize) {
        return NULL;
    }

    unsigned long module = picotm_register_module(&g_ops, &t_module, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct tm_shared_state* global = ref_tm_global_state(error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    tm_vmem_tx_init(&t_module.tx, &global->vmem, module);

    t_module.is_initialized = true;

    return &t_module.tx;
}

static struct tm_vmem_tx*
get_non_null_vmem_tx(void)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        struct tm_vmem_tx* vmem_tx = get_vmem_tx(true, &error);

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
