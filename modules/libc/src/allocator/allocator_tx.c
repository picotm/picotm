/* Permission is hereby granted, free of charge, to any person obtaining a
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
 */

#include "allocator_tx.h"
#include <picotm/picotm-lib-tab.h>
#include <picotm/picotm-module.h>
#include <stdlib.h>

enum allocator_tx_cmd {
    CMD_FREE = 0,
    CMD_POSIX_MEMALIGN,
    LAST_CMD
};

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
allocator_tx_init(struct allocator_tx* self, unsigned long module)
{
    self->module = module;

    self->ptrtab = NULL;
    self->ptrtablen = 0;
    self->ptrtabsiz = 0;
}

void
allocator_tx_uninit(struct allocator_tx* self)
{
    picotm_tabfree(self->ptrtab);
}

static int
append_cmd(struct allocator_tx* self, enum allocator_tx_cmd cmd, void* ptr,
           struct picotm_error* error)
{
    if (__builtin_expect(self->ptrtablen >= self->ptrtabsiz, 0)) {
        void* tmp = picotm_tabresize(self->ptrtab,
                                     self->ptrtabsiz,
                                     self->ptrtabsiz + 1,
                                     sizeof(self->ptrtab[0]),
                                     error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
        self->ptrtab = tmp;

        ++self->ptrtabsiz;
    }

    void** ptrtab = self->ptrtab + self->ptrtablen;

    *ptrtab = ptr;

    picotm_append_event(self->module, cmd, self->ptrtablen, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    return self->ptrtablen++;
}

/*
 * free()
 */

void
allocator_tx_exec_free(struct allocator_tx* self, void *mem,
                       struct picotm_error* error)
{
    append_cmd(self, CMD_FREE, mem, error);
}

static void
apply_free(struct allocator_tx* self, unsigned int cookie,
           struct picotm_error* error)
{
    free(self->ptrtab[cookie]);
}

static void
undo_free(struct allocator_tx* self, unsigned int cookie,
          struct picotm_error* error)
{ }

/*
 * posix_memalign()
 */

void
allocator_tx_exec_posix_memalign(struct allocator_tx* self, void** memptr,
                                 size_t alignment, size_t size,
                                 struct picotm_error* error)
{
    void* mem;

    int err = posix_memalign(&mem, alignment, rnd2wb(size));
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }

    append_cmd(self, CMD_POSIX_MEMALIGN, mem, error);
    if (picotm_error_is_set(error)) {
        goto err_append_cmd;
    }

    *memptr = mem;

    return;

err_append_cmd:
    free(mem);
}

static void
apply_posix_memalign(struct allocator_tx* self, unsigned int cookie,
                     struct picotm_error* error)
{ }

static void
undo_posix_memalign(struct allocator_tx* self, unsigned int cookie,
                    struct picotm_error* error)
{
    free(self->ptrtab[cookie]);
}

/*
 * Module interface
 */

void
allocator_tx_apply_event(struct allocator_tx* self,
                         const struct picotm_event* event,
                         struct picotm_error* error)
{
    static void (* const apply[LAST_CMD])(struct allocator_tx*,
                                          unsigned int,
                                          struct picotm_error*) = {
        apply_free,
        apply_posix_memalign
    };

    apply[event->call](self, event->cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
allocator_tx_undo_event(struct allocator_tx* self,
                        const struct picotm_event* event,
                        struct picotm_error* error)
{
    static void (* const undo[LAST_CMD])(struct allocator_tx*,
                                         unsigned int,
                                         struct picotm_error*) = {
        undo_free,
        undo_posix_memalign
    };

    undo[event->call](self, event->cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
allocator_tx_finish(struct allocator_tx* self)
{
    self->ptrtablen = 0;
}
