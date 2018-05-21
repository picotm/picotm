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
#include <stdbool.h>
#include "allocator_log.h"
#include "allocator_tx.h"

struct allocator_module {
    struct allocator_log log;
    struct allocator_tx tx;
};

/*
 * Thread-local state
 */

PICOTM_STATE(allocator_module, struct allocator_module);
PICOTM_STATE_STATIC_DECL(allocator_module, struct allocator_module)
PICOTM_THREAD_STATE_STATIC_DECL(allocator_module)

static void
apply_event_cb(uint16_t head, uintptr_t tail, void* data,
               struct picotm_error* error)
{
    struct allocator_module* module = data;

    const struct allocator_event* event =
        allocator_log_at(&module->log, tail);

    allocator_tx_apply_event(&module->tx, head, event->ptr, error);
}

static void
undo_event_cb(uint16_t head, uintptr_t tail, void* data,
              struct picotm_error* error)
{
    struct allocator_module* module = data;

    const struct allocator_event* event =
        allocator_log_at(&module->log, tail);

    allocator_tx_undo_event(&module->tx, head, event->ptr, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    struct allocator_module* module = data;

    allocator_tx_finish(&module->tx);
    allocator_log_clear(&module->log);
}

static void
release_cb(void* data)
{
    PICOTM_THREAD_STATE_RELEASE(allocator_module);
}

void
init_allocator_module(struct allocator_module* module,
                      struct picotm_error* error)
{
    static const struct picotm_module_ops s_ops = {
        .apply_event = apply_event_cb,
        .undo_event = undo_event_cb,
        .finish = finish_cb,
        .release = release_cb
    };

    unsigned long module_id = picotm_register_module(&s_ops, module, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    allocator_log_init(&module->log, module_id);
    allocator_tx_init(&module->tx, &module->log);
}

static void
uninit_allocator_module(struct allocator_module* module)
{
    allocator_tx_uninit(&module->tx);
    allocator_log_uninit(&module->log);
}

PICOTM_STATE_STATIC_IMPL(allocator_module, struct allocator_module,
                         init_allocator_module,
                         uninit_allocator_module)
PICOTM_THREAD_STATE_STATIC_IMPL(allocator_module)

static struct allocator_tx*
get_allocator_tx(struct picotm_error* error)
{
    struct allocator_module* module =
        PICOTM_THREAD_STATE_ACQUIRE(allocator_module, true, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return &module->tx;
}

static struct allocator_tx*
get_non_null_allocator_tx(void)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        struct allocator_tx* allocator_tx = get_allocator_tx(&error);
        if (!picotm_error_is_set(&error)) {
            /* assert() here as there's no legal way that allocator_tx
             * could be NULL */
            assert(allocator_tx);
            return allocator_tx;
        }

        picotm_recover_from_error(&error);
    } while (true);
}

/*
 * Public interface
 */

void
allocator_module_free(void* mem, size_t usiz)
{
    struct allocator_tx* data = get_non_null_allocator_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        allocator_tx_exec_free(data, mem, &error);

        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

void*
allocator_module_malloc(size_t size)
{
    struct allocator_tx* data = get_non_null_allocator_tx();
    assert(data);

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        void* mem = allocator_tx_exec_malloc(data, size, &error);

        if (!picotm_error_is_set(&error)) {
            return mem;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

#if defined(HAVE_POSIX_MEMALIGN) && HAVE_POSIX_MEMALIGN
void
allocator_module_posix_memalign(void** memptr, size_t alignment, size_t size)
{
    struct allocator_tx* data = get_non_null_allocator_tx();
    assert(data);

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        allocator_tx_exec_posix_memalign(data, memptr, alignment, size, &error);

        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}
#endif
