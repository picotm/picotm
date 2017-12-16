/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "framemap.h"
#include <assert.h>
#include <limits.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <stdlib.h>
#include "block.h"
#include "frame.h"

/*
 * frame table
 */

#define TM_FRAME_TBL_SIZE_BITS  (10)
#define TM_FRAME_TBL_SIZE       (1ul << TM_FRAME_TBL_SIZE_BITS)
#define TM_FRAME_TBL_SIZE_MASK  (TM_FRAME_TBL_SIZE - 1)

struct tm_frame_tbl {
    struct tm_frame frame[TM_FRAME_TBL_SIZE];
};

static void
tm_frame_tbl_init(struct tm_frame_tbl* self, size_t first_block_index)
{
    struct tm_frame* beg = picotm_arraybeg(self->frame);
    const struct tm_frame* end = picotm_arrayend(self->frame);

    while (beg < end) {
        tm_frame_init(beg++, first_block_index++);
    }
}

static void
tm_frame_tbl_uninit(struct tm_frame_tbl* self)
{
    struct tm_frame* beg = picotm_arraybeg(self->frame);
    const struct tm_frame* end = picotm_arrayend(self->frame);

    while (beg < end) {
        tm_frame_uninit(beg++);
    }
}

static uintptr_t
tm_frame_tbl_create(unsigned long long key,
                 struct picotm_shared_treemap* treemap,
                 struct picotm_error* error)
{
    struct tm_frame_tbl* tbl = malloc(sizeof(*tbl));
    if (!tbl) {
        picotm_error_set_error_code(error, PICOTM_OUT_OF_MEMORY);
        return 0;
    }

    tm_frame_tbl_init(tbl, key << TM_FRAME_TBL_SIZE_BITS);

    return (uintptr_t)tbl;
}

static void
tm_frame_tbl_destroy(uintptr_t value, struct picotm_shared_treemap* treemap)
{
    struct tm_frame_tbl* tbl = (struct tm_frame_tbl*)value;

    tm_frame_tbl_uninit(tbl);
    free(tbl);
}

/*
 * frame map
 */

void
tm_frame_map_init(struct tm_frame_map* self)
{
    /* The number of bits in an address that are handled by the
     * treemap. The lowest bits in each address index into the block
     * and frame. The higher bits index into the treemap directories.
     */
    unsigned long key_nbits =
        (sizeof(void*) * CHAR_BIT) -
            (TM_FRAME_TBL_SIZE_BITS + TM_BLOCK_SIZE_BITS);

    picotm_shared_treemap_init(&self->map, key_nbits, 10);
}

void
tm_frame_map_uninit(struct tm_frame_map* self)
{
    picotm_shared_treemap_uninit(&self->map, tm_frame_tbl_destroy);
}

static unsigned long long
frame_tbl_offset(uintptr_t addr)
{
    return addr >> (TM_FRAME_TBL_SIZE_BITS + TM_BLOCK_SIZE_BITS);
}

static size_t
frame_tbl_index(uintptr_t addr)
{
    return (addr >> TM_BLOCK_SIZE_BITS) & TM_FRAME_TBL_SIZE_MASK;
}

struct tm_frame*
tm_frame_map_lookup(struct tm_frame_map* self, uintptr_t addr,
                    struct picotm_error* error)
{
    assert(self);

    uintptr_t value = picotm_shared_treemap_find_value(
        &self->map, frame_tbl_offset(addr),
        tm_frame_tbl_create, tm_frame_tbl_destroy,
        error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    struct tm_frame_tbl* tbl = (struct tm_frame_tbl*)value;

    return tbl->frame + frame_tbl_index(addr);
}
