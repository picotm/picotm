/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann
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
#include <stdbool.h>
#include "allocator_log.h"
#include "allocator_tx.h"

/*
 * Module interface
 */

struct allocator_module {
    struct allocator_log log;
    struct allocator_tx tx;
};

static void
allocator_module_init(struct allocator_module* self, unsigned long module_id)
{
    assert(self);

    allocator_log_init(&self->log, module_id);
    allocator_tx_init(&self->tx, &self->log);
}

static void
allocator_module_uninit(struct allocator_module* self)
{
    assert(self);

    allocator_tx_uninit(&self->tx);
    allocator_log_uninit(&self->log);
}

static void
allocator_module_apply_event(struct allocator_module* self,
                             uint16_t head, uintptr_t tail,
                             struct picotm_error* error)
{
    assert(self);

    const struct allocator_event* event =
        allocator_log_at(&self->log, tail);
    assert(event);

    allocator_tx_apply_event(&self->tx, head, event->ptr, error);
}

static void
allocator_module_undo_event(struct allocator_module* self,
                            uint16_t head, uintptr_t tail,
                            struct picotm_error* error)
{
    assert(self);

    const struct allocator_event* event =
        allocator_log_at(&self->log, tail);
    assert(event);

    allocator_tx_undo_event(&self->tx, head, event->ptr, error);
}

static void
allocator_module_finish(struct allocator_module* self)
{
    assert(self);

    allocator_tx_finish(&self->tx);
    allocator_log_clear(&self->log);
}


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
    allocator_module_apply_event(module, head, tail, error);
}

static void
undo_event_cb(uint16_t head, uintptr_t tail, void* data,
              struct picotm_error* error)
{
    struct allocator_module* module = data;
    allocator_module_undo_event(module, head, tail, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    struct allocator_module* module = data;
    allocator_module_finish(module);
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

    allocator_module_init(module, module_id);
}

static void
uninit_allocator_module(struct allocator_module* module)
{
    allocator_module_uninit(module);
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

/*
 * Public interface
 */

void
allocator_module_free(void* mem, size_t usiz, struct picotm_error* error)
{
    struct allocator_tx* data = get_allocator_tx(error);
    if (picotm_error_is_set(error)) {
        return;
    }
    allocator_tx_exec_free(data, mem, error);
}

void*
allocator_module_malloc(size_t size, struct picotm_error* error)
{
    struct allocator_tx* data = get_allocator_tx(error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return allocator_tx_exec_malloc(data, size, error);
}

#if defined(HAVE_POSIX_MEMALIGN) && HAVE_POSIX_MEMALIGN
int
allocator_module_posix_memalign(void** memptr, size_t alignment, size_t size,
                                struct picotm_error* error)
{
    struct allocator_tx* data = get_allocator_tx(error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return allocator_tx_exec_posix_memalign(data, memptr, alignment, size,
                                            error);
}
#endif
