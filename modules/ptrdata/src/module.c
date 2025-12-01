/*
 * picotm - A system-level transaction manager
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
#include <assert.h>
#include "ptrdata.h"
#include "ptrdata_tx.h"

/*
 * Shared state
 */

static void
init_ptrdata_shared_state_fields(struct ptrdata* ptrdata,
                                 struct picotm_error* error)
{
    ptrdata_init(ptrdata);
}

static void
uninit_ptrdata_shared_state_fields(struct ptrdata* ptrdata)
{
    ptrdata_uninit(ptrdata);
}

PICOTM_SHARED_STATE(ptrdata, struct ptrdata);
PICOTM_SHARED_STATE_STATIC_IMPL(ptrdata, struct ptrdata,
                                init_ptrdata_shared_state_fields,
                                uninit_ptrdata_shared_state_fields)

/*
 * Global state
 */

PICOTM_GLOBAL_STATE_STATIC_IMPL(ptrdata)

/*
 * Module interface
 */

struct ptrdata_module {
    struct ptrdata_tx tx;
    struct ptrdata* global;
};

static void
ptrdata_module_init(struct ptrdata_module* self, struct ptrdata* global)
{
    assert(self);

    ptrdata_tx_init(&self->tx);
    self->global = global;
}

static void
ptrdata_module_uninit(struct ptrdata_module* self)
{
    assert(self);

    ptrdata_tx_uninit(&self->tx);
}

/*
 * Thread-local state
 */

PICOTM_STATE(ptrdata_module, struct ptrdata_module);
PICOTM_STATE_STATIC_DECL(ptrdata_module, struct ptrdata_module)
PICOTM_THREAD_STATE_STATIC_DECL(ptrdata_module)

static void
release_cb(void* data)
{
    PICOTM_THREAD_STATE_RELEASE(ptrdata_module);
}

static void
init_ptrdata_module(struct ptrdata_module* module,
                    struct picotm_error* error)
{
    static const struct picotm_module_ops s_ops = {
        .release = release_cb
    };

    struct ptrdata* ptrdata = PICOTM_GLOBAL_STATE_REF(ptrdata, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    picotm_register_module(&s_ops, module, error);
    if (picotm_error_is_set(error)) {
        goto err_picotm_register_module;
    }

    ptrdata_module_init(module, ptrdata);

    return;

err_picotm_register_module:
    PICOTM_GLOBAL_STATE_UNREF(ptrdata);
}

static void
uninit_ptrdata_module(struct ptrdata_module* module)
{
    ptrdata_module_uninit(module);
    PICOTM_GLOBAL_STATE_UNREF(ptrdata);
}

PICOTM_STATE_STATIC_IMPL(ptrdata_module, struct ptrdata_module,
                         init_ptrdata_module,
                         uninit_ptrdata_module)
PICOTM_THREAD_STATE_STATIC_IMPL(ptrdata_module)

/*
 * Public interface
 */

void
ptrdata_module_set_shared_data(const void* ptr, const void* data,
                               struct picotm_error* error)
{
    struct ptrdata_module* module =
        PICOTM_THREAD_STATE_ACQUIRE(ptrdata_module, true, error);
    if (picotm_error_is_set(error)) {
        return;
    }
    ptrdata_set_shared_data(module->global, ptr, data, error);
}

bool
ptrdata_module_test_and_set_shared_data(const void* ptr, const void* current,
                                        const void* data,
                                        struct picotm_error* error)
{
    struct ptrdata_module* module =
        PICOTM_THREAD_STATE_ACQUIRE(ptrdata_module, true, error);
    if (picotm_error_is_set(error)) {
        return false;
    }
    return ptrdata_test_and_set_shared_data(module->global, ptr, current,
                                             data, error);
}

void
ptrdata_module_clear_shared_data(const void* ptr, struct picotm_error* error)
{
    struct ptrdata_module* module =
        PICOTM_THREAD_STATE_ACQUIRE(ptrdata_module, false, error);
    if (picotm_error_is_set(error)) {
        return;
    } else if (!module) {
        return;
    }
    ptrdata_clear_shared_data(module->global, ptr, error);
}

void*
ptrdata_module_get_shared_data(const void* ptr, struct picotm_error* error)
{
    struct ptrdata_module* module =
        PICOTM_THREAD_STATE_ACQUIRE(ptrdata_module, false, error);
    if (picotm_error_is_set(error)) {
        return nullptr;
    } else if (!module) {
        return nullptr;
    }
    return ptrdata_get_shared_data(module->global, ptr, error);
}

void
ptrdata_module_set_data(const void* ptr, const void* data,
                        struct picotm_error* error)
{
    struct ptrdata_module* module =
        PICOTM_THREAD_STATE_ACQUIRE(ptrdata_module, true, error);
    if (picotm_error_is_set(error)) {
        return;
    }
    ptrdata_tx_set_data(&module->tx, ptr, data, error);
}

void
ptrdata_module_clear_data(const void* ptr, struct picotm_error* error)
{
    struct ptrdata_module* module =
        PICOTM_THREAD_STATE_ACQUIRE(ptrdata_module, false, error);
    if (picotm_error_is_set(error)) {
        return;
    } else if (!module) {
        return;
    }
    ptrdata_tx_clear_data(&module->tx, ptr, error);
}

void*
ptrdata_module_get_data(const void* ptr, struct picotm_error* error)
{
    struct ptrdata_module* module =
        PICOTM_THREAD_STATE_ACQUIRE(ptrdata_module, false, error);
    if (picotm_error_is_set(error)) {
        return nullptr;
    } else if (!module) {
        return nullptr;
    }
    return ptrdata_tx_get_data(&module->tx, ptr, error);
}
