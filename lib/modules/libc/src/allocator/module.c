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

static int
apply_event_cb(const struct event* event, size_t nevents, void* data,
               struct picotm_error* error)
{
    struct allocator_module* module = data;

    int res = allocator_tx_apply_event(&module->tx, event, nevents);
    if (res < 0) {
        picotm_error_set_error_code(error, PICOTM_GENERAL_ERROR);
        return -1;
    }
    return 0;
}

static int
undo_event_cb(const struct event* event, size_t nevents, void* data,
              struct picotm_error* error)
{
    struct allocator_module* module = data;

    int res = allocator_tx_undo_event(&module->tx, event, nevents);
    if (res < 0) {
        picotm_error_set_error_code(error, PICOTM_GENERAL_ERROR);
        return -1;
    }
    return 0;
}

static int
finish_cb(void* data, struct picotm_error* error)
{
    struct allocator_module* module = data;

    allocator_tx_finish(&module->tx);

    return 0;
}

static void
uninit_cb(void* data)
{
    struct allocator_module* module = data;

    allocator_tx_uninit(&module->tx);
    module->is_initialized = false;
}

static struct allocator_tx*
get_allocator_tx(void)
{
    static __thread struct allocator_module t_module;

    if (t_module.is_initialized) {
        return &t_module.tx;
    }

    long res = picotm_register_module(NULL,
                                      NULL,
                                      NULL,
                                      apply_event_cb,
                                      undo_event_cb,
                                      NULL,
                                      NULL,
                                      finish_cb,
                                      uninit_cb,
                                      &t_module);
    if (res < 0) {
        return NULL;
    }
    unsigned long module = res;

    res = allocator_tx_init(&t_module.tx, module);
    if (res < 0) {
        return NULL;
    }

    t_module.is_initialized = true;

    return &t_module.tx;
}

int
allocator_module_free(void* mem, size_t usiz)
{
    struct allocator_tx* data = get_allocator_tx();
    assert(data);

    allocator_tx_exec_free(data, mem);

    return 0;
}

int
allocator_module_posix_memalign(void** memptr, size_t alignment, size_t size)
{
    struct allocator_tx* data = get_allocator_tx();
    assert(data);

    return allocator_tx_exec_posix_memalign(data, memptr, alignment, size);
}
