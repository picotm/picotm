/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "module.h"
#include <assert.h>
#include <stdbool.h>
#include <picotm/picotm-module.h>
#include "allocator_tx.h"

struct allocator_module {
    bool                is_initialized;
    struct allocator_tx tx;
};

static void
apply_event_cb(const struct event* event, size_t nevents, void* data,
               struct picotm_error* error)
{
    struct allocator_module* module = data;

    allocator_tx_apply_event(&module->tx, event, nevents, error);
}

static void
undo_event_cb(const struct event* event, size_t nevents, void* data,
              struct picotm_error* error)
{
    struct allocator_module* module = data;

    allocator_tx_undo_event(&module->tx, event, nevents, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    struct allocator_module* module = data;

    allocator_tx_finish(&module->tx);
}

static void
uninit_cb(void* data)
{
    struct allocator_module* module = data;

    allocator_tx_uninit(&module->tx);
    module->is_initialized = false;
}

static struct allocator_tx*
get_allocator_tx(bool initialize, struct picotm_error* error)
{
    static __thread struct allocator_module t_module;

    if (t_module.is_initialized) {
        return &t_module.tx;
    } else if (!initialize) {
        return NULL;
    }

    unsigned long module = picotm_register_module(NULL, NULL, NULL,
                                                  NULL, NULL,
                                                  apply_event_cb,
                                                  undo_event_cb,
                                                  NULL, NULL,
                                                  finish_cb,
                                                  uninit_cb,
                                                  &t_module,
                                                  error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    allocator_tx_init(&t_module.tx, module);

    t_module.is_initialized = true;

    return &t_module.tx;
}

static struct allocator_tx*
get_non_null_allocator_tx(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    struct allocator_tx* allocator_tx = get_allocator_tx(true, &error);
    if (picotm_error_is_set(&error)) {
        picotm_recover_from_error(&error);
    }
    /* assert() here as there's no legal way that allocator_tx could be NULL */
    assert(allocator_tx);
    return allocator_tx;
}

/*
 * Public interface
 */

void
allocator_module_free(void* mem, size_t usiz)
{
    struct allocator_tx* data = get_non_null_allocator_tx();

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    allocator_tx_exec_free(data, mem, &error);
    if (picotm_error_is_set(&error)) {
        picotm_recover_from_error(&error);
    }
}

void
allocator_module_posix_memalign(void** memptr, size_t alignment, size_t size)
{
    struct allocator_tx* data = get_non_null_allocator_tx();
    assert(data);

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    allocator_tx_exec_posix_memalign(data, memptr, alignment, size, &error);
    if (picotm_error_is_set(&error)) {
        picotm_recover_from_error(&error);
    }
}
