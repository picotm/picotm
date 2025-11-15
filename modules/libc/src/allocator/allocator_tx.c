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

#include "allocator_tx.h"
#include "picotm/picotm-error.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include "allocator_log.h"

/**
 * Round size up to next multiple of word size.
 */
static size_t
rnd2wb(size_t size)
{
    const unsigned int mask = sizeof(void*) - 1;

    return (size + mask) & ~mask;
}

void
allocator_tx_init(struct allocator_tx* self, struct allocator_log* log)
{
    assert(self);

    self->log = log;
}

void
allocator_tx_uninit(struct allocator_tx* self)
{ }

/*
 * free()
 */

void
allocator_tx_exec_free(struct allocator_tx* self, void* mem,
                       struct picotm_error* error)
{
    assert(self);

    allocator_log_append(self->log, ALLOCATOR_OP_FREE, mem, error);
}

static void
apply_free(struct allocator_tx* self, void* ptr,
           struct picotm_error* error)
{
    free(ptr);
}

static void
undo_free(struct allocator_tx* self, void* ptr,
          struct picotm_error* error)
{ }

/*
 * malloc()
 */

void*
allocator_tx_exec_malloc(struct allocator_tx* self, size_t size,
                         struct picotm_error* error)
{
    assert(self);

    void* mem = malloc(rnd2wb(size));
    if (!mem) {
        /* ISO C specifies allocator functions to return NULL in the
         * case of an error. We report the errno code on all systems
         * that support it, otherwise we set a picotm error code. */
#if defined(ENOMEM)
        picotm_error_set_errno(error, errno);
#else
        picotm_error_set_error_code(error, PICOTM_OUT_OF_MEMORY);
#endif
        return NULL;
    }

    allocator_log_append(self->log, ALLOCATOR_OP_MALLOC, mem, error);
    if (picotm_error_is_set(error)) {
        goto err_allocator_log_append;
    }

    return mem;

err_allocator_log_append:
    free(mem);
    return NULL;
}

static void
apply_malloc(struct allocator_tx* self, void* mem,
             struct picotm_error* error)
{ }

static void
undo_malloc(struct allocator_tx* self, void* mem,
            struct picotm_error* error)
{
    free(mem);
}

/*
 * posix_memalign()
 */

#if defined(HAVE_POSIX_MEMALIGN) && HAVE_POSIX_MEMALIGN
int
allocator_tx_exec_posix_memalign(struct allocator_tx* self, void** memptr,
                                 size_t alignment, size_t size,
                                 struct picotm_error* error)
{
    assert(self);

    void* mem;

    int err = posix_memalign(&mem, alignment, rnd2wb(size));
    if (err) {
        picotm_error_set_errno(error, err);
        return err;
    }

    allocator_log_append(self->log, ALLOCATOR_OP_POSIX_MEMALIGN, mem, error);
    if (picotm_error_is_set(error)) {
        goto err_allocator_log_append;
    }

    *memptr = mem;

    return 0;

err_allocator_log_append:
    free(mem);
    return err;
}
#endif

#if defined(HAVE_POSIX_MEMALIGN) && HAVE_POSIX_MEMALIGN
static void
apply_posix_memalign(struct allocator_tx* self, void* ptr,
                     struct picotm_error* error)
{ }
#else
#define apply_posix_memalign    NULL
#endif

#if defined(HAVE_POSIX_MEMALIGN) && HAVE_POSIX_MEMALIGN
static void
undo_posix_memalign(struct allocator_tx* self, void* ptr,
                    struct picotm_error* error)
{
    free(ptr);
}
#else
#define undo_posix_memalign    NULL
#endif

/*
 * Module interface
 */

void
allocator_tx_apply_event(struct allocator_tx* self,
                         enum allocator_op op, void* ptr,
                         struct picotm_error* error)
{
    static void (* const apply[LAST_ALLOCATOR_OP])(struct allocator_tx*,
                                                   void*,
                                                   struct picotm_error*) = {
        apply_free,
        apply_malloc,
        apply_posix_memalign
    };

    apply[op](self, ptr, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
allocator_tx_undo_event(struct allocator_tx* self,
                        enum allocator_op op, void* ptr,
                        struct picotm_error* error)
{
    static void (* const undo[LAST_ALLOCATOR_OP])(struct allocator_tx*,
                                                  void*,
                                                  struct picotm_error*) = {
        undo_free,
        undo_malloc,
        undo_posix_memalign
    };

    undo[op](self, ptr, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
allocator_tx_finish(struct allocator_tx* self)
{ }
