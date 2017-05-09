/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "allocator_tx.h"
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

int
allocator_tx_init(struct allocator_tx* self, unsigned long module)
{
    self->module = module;

    self->ptrtab = NULL;
    self->ptrtablen = 0;
    self->ptrtabsiz = 0;

    return 0;
}

void
allocator_tx_uninit(struct allocator_tx* self)
{
    picotm_tabfree(self->ptrtab);
}

static int
append_cmd(struct allocator_tx* self, enum allocator_tx_cmd cmd, void* ptr)
{
    if (__builtin_expect(self->ptrtablen >= self->ptrtabsiz, 0)) {
        void* tmp = picotm_tabresize(self->ptrtab,
                                     self->ptrtabsiz,
                                     self->ptrtabsiz + 1,
                                     sizeof(self->ptrtab[0]));
        if (!tmp) {
            return -1;
        }
        self->ptrtab = tmp;

        ++self->ptrtabsiz;
    }

    void** ptrtab = self->ptrtab + self->ptrtablen;

    *ptrtab = ptr;

    int res = picotm_inject_event(self->module, cmd, self->ptrtablen);
    if (res < 0) {
        return -1;
    }

    return self->ptrtablen++;
}

/*
 * free()
 */

int
allocator_tx_exec_free(struct allocator_tx* self, void *mem)
{
    int res = append_cmd(self, CMD_FREE, mem);
    if (res < 0) {
        return res;
    }
    return 0;
}

static int
apply_free(struct allocator_tx* self, unsigned int cookie,
           struct picotm_error* error)
{
    free(self->ptrtab[cookie]);

    return 0;
}

static int
undo_free(struct allocator_tx* self, unsigned int cookie,
          struct picotm_error* error)
{
    return 0;
}

/*
 * posix_memalign()
 */

int
allocator_tx_exec_posix_memalign(struct allocator_tx* self, void** memptr,
                                 size_t alignment, size_t size)
{
    void* mem;

    int err = posix_memalign(&mem, alignment, rnd2wb(size));
    if (err) {
        return -err;
    }

    int res = append_cmd(self, CMD_POSIX_MEMALIGN, mem);
    if (res < 0) {
        goto err_append_cmd;
    }

    *memptr = mem;

    return 0;

err_append_cmd:
    free(mem);
    return res;
}

static int
apply_posix_memalign(struct allocator_tx* self, unsigned int cookie,
                     struct picotm_error* error)
{
    return 0;
}

static int
undo_posix_memalign(struct allocator_tx* self, unsigned int cookie,
                    struct picotm_error* error)
{
    free(self->ptrtab[cookie]);

    return 0;
}

/*
 * Module interface
 */

int
allocator_tx_apply_event(struct allocator_tx* self, const struct event* event,
                         size_t nevents, struct picotm_error* error)
{
    static int (* const apply[LAST_CMD])(struct allocator_tx*,
                                         unsigned int,
                                         struct picotm_error*) = {
        apply_free,
        apply_posix_memalign
    };

    while (nevents) {
        int res = apply[event->call](self, event->cookie, error);
        if (res < 0) {
            return -1;
        }
        --nevents;
        ++event;
    }

    return 0;
}

int
allocator_tx_undo_event(struct allocator_tx* self, const struct event* event,
                        size_t nevents, struct picotm_error* error)
{
    static int (* const undo[LAST_CMD])(struct allocator_tx*,
                                        unsigned int,
                                        struct picotm_error*) = {
        undo_free,
        undo_posix_memalign
    };

    event += nevents;

    while (nevents) {
        --event;
        int res = undo[event->call](self, event->cookie, error);
        if (res < 0) {
            return -1;
        }
        --nevents;
    }

    return 0;
}

int
allocator_tx_finish(struct allocator_tx* self, struct picotm_error* error)
{
    self->ptrtablen = 0;

    return 0;
}
