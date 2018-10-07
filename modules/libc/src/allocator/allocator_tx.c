/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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
void
allocator_tx_exec_posix_memalign(struct allocator_tx* self, void** memptr,
                                 size_t alignment, size_t size,
                                 struct picotm_error* error)
{
    assert(self);

    void* mem;

    int err = posix_memalign(&mem, alignment, rnd2wb(size));
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }

    allocator_log_append(self->log, ALLOCATOR_OP_POSIX_MEMALIGN, mem, error);
    if (picotm_error_is_set(error)) {
        goto err_allocator_log_append;
    }

    *memptr = mem;

    return;

err_allocator_log_append:
    free(mem);
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
