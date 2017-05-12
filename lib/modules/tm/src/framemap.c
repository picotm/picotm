/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "framemap.h"
#include <picotm/picotm-error.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "ptr.h"

/*
 * Frame table
 */

static void
tm_frame_tbl_init(struct tm_frame_tbl* self, size_t first_block_index)
{
    struct tm_frame* beg = self->frame;
    const struct tm_frame* end = self->frame + arraylen(self->frame);

    while (beg < end) {
        tm_frame_init(beg++, first_block_index++);
    }
}

/*
 * Frame directory
 */

void
tm_frame_dir_init(struct tm_frame_dir* self)
{
    memset(self->entry, 0, sizeof(self->entry));
}

/*
 * Frame top-level directory
 */

void
tm_frame_tld_init(struct tm_frame_tld* self)
{
    memset(self->entry, 0, sizeof(self->entry));
}

/*
 * Frame map
 */

void
tm_frame_map_init(struct tm_frame_map* self)
{
    tm_frame_tld_init(&self->tld);
}

static bool
reached_tbl(size_t shiftbits)
{
    return shiftbits <= (TM_FRAME_TBL_SIZE_BITS + TM_BLOCK_SIZE_BITS);
}

static void
cleanup_tbl(struct tm_frame_tbl* tbl)
{
    free(tbl);
}

static void
cleanup_dir(struct tm_frame_dir* dir, size_t shiftbits)
{
    shiftbits -= TM_FRAME_DIR_SIZE_BITS;

    uintptr_t* beg = dir->entry;
    const uintptr_t* end = dir->entry + arraylen(dir->entry);

    if (reached_tbl(shiftbits)) {
        while (beg < end) {
            if (*beg) {
                cleanup_tbl((struct tm_frame_tbl*)(*beg));
            }
            ++beg;
        }
    } else {
        while (beg < end) {
            if (*beg) {
                cleanup_dir((struct tm_frame_dir*)(*beg), shiftbits);
            }
            ++beg;
        }
    }
    free(dir);
}

static void
cleanup_tld(struct tm_frame_tld* tld, size_t shiftbits)
{
    shiftbits -= TM_FRAME_TLD_SIZE_BITS;

    uintptr_t* beg = tld->entry;
    const uintptr_t* end = tld->entry + arraylen(tld->entry);

    if (reached_tbl(shiftbits)) {
        while (beg < end) {
            if (*beg) {
                cleanup_tbl((struct tm_frame_tbl*)(*beg));
            }
            ++beg;
        }
    } else {
        while (beg < end) {
            if (*beg) {
                cleanup_dir((struct tm_frame_dir*)(*beg), shiftbits);
            }
            ++beg;
        }
    }
}

static size_t
initial_shiftbits(void)
{
    return (CHAR_BIT * sizeof(void*));
}

void
tm_frame_map_uninit(struct tm_frame_map* self)
{
    cleanup_tld(&self->tld, initial_shiftbits());
}

static size_t
first_block_index(uintptr_t addr)
{
    return (addr >> (TM_FRAME_TBL_SIZE_BITS + TM_BLOCK_SIZE_BITS)) << TM_FRAME_TBL_SIZE_BITS;
}

static size_t
tbl_index(uintptr_t addr, size_t shiftbits)
{
    shiftbits -= TM_FRAME_TBL_SIZE_BITS;
    return (addr >> shiftbits) & TM_FRAME_TBL_SIZE_MASK;
}

static size_t
next_dir_index(uintptr_t addr, size_t* shiftbits)
{
    *shiftbits -= TM_FRAME_DIR_SIZE_BITS;
    return (addr >> (*shiftbits)) & TM_FRAME_DIR_SIZE_MASK;
}

static size_t
next_tld_index(uintptr_t addr, size_t* shiftbits)
{
    *shiftbits -= TM_FRAME_TLD_SIZE_BITS;
    return (addr >> (*shiftbits)) & TM_FRAME_TLD_SIZE_MASK;
}

static struct tm_frame_dir*
load_dir(uintptr_t* entry, struct picotm_error* error)
{
    struct tm_frame_dir* dir =
        (struct tm_frame_dir*)__atomic_load_n(entry, __ATOMIC_ACQUIRE);
    if (dir) {
        return dir;
    }

    dir = malloc(sizeof(*dir));
    if (!dir) {
        picotm_error_set_error_code(error, PICOTM_OUT_OF_MEMORY);
        return NULL;
    }
    tm_frame_dir_init(dir);

    uintptr_t existing = 0;
    bool success = __atomic_compare_exchange_n(entry, &existing,
                                               (uintptr_t)dir, false,
                                               __ATOMIC_ACQ_REL,
                                               __ATOMIC_ACQUIRE);
    if (!success) {
        /* A concurrent transaction already installed the
         * directory. We free our instance and use the existing
         * one. */;
        free(dir);
        dir = (struct tm_frame_dir*)existing;
    }

    return dir;
}

static struct tm_frame_tbl*
load_tbl(uintptr_t* entry, uintptr_t addr, struct picotm_error* error)
{
    struct tm_frame_tbl* tbl =
        (struct tm_frame_tbl*)__atomic_load_n(entry, __ATOMIC_ACQUIRE);
    if (tbl) {
        return tbl;
    }

    tbl = malloc(sizeof(*tbl));
    if (!tbl) {
        picotm_error_set_error_code(error, PICOTM_OUT_OF_MEMORY);
        return NULL;
    }
    tm_frame_tbl_init(tbl, first_block_index(addr));

    uintptr_t existing = 0;
    bool success = __atomic_compare_exchange_n(entry, &existing,
                                               (uintptr_t)tbl, false,
                                               __ATOMIC_ACQ_REL,
                                               __ATOMIC_ACQUIRE);
    if (!success) {
        /* A concurrent transaction already installed the
         * table. We free our instance and use the existing
         * one. */;
        free(tbl);
        tbl = (struct tm_frame_tbl*)existing;
    }

    return tbl;
}

struct tm_frame*
tm_frame_map_lookup(struct tm_frame_map* self, uintptr_t addr,
                    struct picotm_error* error)
{
    size_t shiftbits = initial_shiftbits();

    /* Look up frame directory from top-level directory */

    uintptr_t* entry = self->tld.entry + next_tld_index(addr, &shiftbits);

    /* Look up frame table from directory hierarchy */

    while (!reached_tbl(shiftbits)) {
        struct tm_frame_dir* dir = load_dir(entry, error);
        if (picotm_error_is_set(error)) {
            return NULL;
        }
        entry = dir->entry + next_dir_index(addr, &shiftbits);
    }

    /* Look up frame from frame table */

    struct tm_frame_tbl* tbl = load_tbl(entry, addr, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return tbl->frame + tbl_index(addr, shiftbits);
}
