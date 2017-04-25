/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "module.h"
#include <assert.h>
#include <stdbool.h>
#include <picotm/picotm-module.h>
#include "comalloc.h"

struct allocator_module {
    bool             is_initialized;
    struct com_alloc instance;
};

static int
apply_event_cb(const struct event* event, size_t nevents, void* data)
{
    struct allocator_module* module = data;

    return com_alloc_apply_event(&module->instance, event, nevents);
}

static int
undo_event_cb(const struct event* event, size_t nevents, void* data)
{
    struct allocator_module* module = data;

    return com_alloc_undo_event(&module->instance, event, nevents);
}

static int
finish_cb(void* data)
{
    struct allocator_module* module = data;

    com_alloc_finish(&module->instance);

    return 0;
}

static int
uninit_cb(void* data)
{
    struct allocator_module* module = data;

    com_alloc_uninit(&module->instance);

    module->is_initialized = false;

    return 0;
}

static struct com_alloc*
get_com_alloc(void)
{
    static __thread struct allocator_module t_module;

    if (t_module.is_initialized) {
        return &t_module.instance;
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

    res = com_alloc_init(&t_module.instance, module);
    if (res < 0) {
        return NULL;
    }

    t_module.is_initialized = true;

    return &t_module.instance;
}

int
allocator_module_free(void* mem, size_t usiz)
{
    struct com_alloc* data = get_com_alloc();
    assert(data);

    com_alloc_exec_free(data, mem);

    return 0;
}

int
allocator_module_posix_memalign(void** memptr, size_t alignment, size_t size)
{
    struct com_alloc* data = get_com_alloc();
    assert(data);

    return com_alloc_exec_posix_memalign(data, memptr, alignment, size);
}
