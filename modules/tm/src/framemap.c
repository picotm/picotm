/*
 * picotm - A system-level transaction manager
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

#include "framemap.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include <assert.h>
#include <limits.h>
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
        return nullptr;
    }
    struct tm_frame_tbl* tbl = (struct tm_frame_tbl*)value;

    return tbl->frame + frame_tbl_index(addr);
}
