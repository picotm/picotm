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

#include "framemap.h"
#include <picotm/picotm-error.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "picotm/picotm-lib-array.h"

/*
 * Frame table
 */

static void
tm_frame_tbl_init(struct tm_frame_tbl* self, size_t first_block_index)
{
    struct tm_frame* beg = picotm_arraybeg(self->frame);
    const struct tm_frame* end = picotm_arrayend(self->frame);

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

    atomic_uintptr_t* beg = picotm_arraybeg(dir->entry);
    const atomic_uintptr_t* end = picotm_arrayend(dir->entry);

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

    atomic_uintptr_t* beg = picotm_arraybeg(tld->entry);
    const atomic_uintptr_t* end = picotm_arrayend(tld->entry);

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
load_dir(atomic_uintptr_t* entry, struct picotm_error* error)
{
    struct tm_frame_dir* dir =
        (struct tm_frame_dir*)atomic_load_explicit(entry, memory_order_acquire);
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
    bool success =
        atomic_compare_exchange_strong_explicit(entry,
                                                &existing,
                                                (uintptr_t)dir,
                                                memory_order_acq_rel,
                                                memory_order_acquire);
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
load_tbl(atomic_uintptr_t* entry, uintptr_t addr, struct picotm_error* error)
{
    struct tm_frame_tbl* tbl =
        (struct tm_frame_tbl*)atomic_load_explicit(entry,
                                                   memory_order_acquire);
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
    bool success =
        atomic_compare_exchange_strong_explicit(entry,
                                                &existing,
                                                (uintptr_t)tbl,
                                                memory_order_acq_rel,
                                                memory_order_acquire);
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

    atomic_uintptr_t* entry = self->tld.entry + next_tld_index(addr, &shiftbits);

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
