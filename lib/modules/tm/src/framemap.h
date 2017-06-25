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

#pragma once

#include <stdatomic.h>
#include <limits.h>
#include <stdint.h>
#include "block.h"
#include "frame.h"

/**
 * \cond impl || tm_impl
 * \ingroup tm_impl
 * \file
 * \endcond
 */

struct picotm_error;

/*
 * Frame table
 */

#define TM_FRAME_TBL_SIZE_BITS  (10)
#define TM_FRAME_TBL_SIZE       (1ul << TM_FRAME_TBL_SIZE_BITS)
#define TM_FRAME_TBL_SIZE_MASK  (TM_FRAME_TBL_SIZE - 1)

struct tm_frame_tbl {
    struct tm_frame frame[TM_FRAME_TBL_SIZE];
};

/*
 * Frame directory
 */

#define TM_FRAME_DIR_SIZE_BITS  (10)
#define TM_FRAME_DIR_SIZE       (1ul << TM_FRAME_DIR_SIZE_BITS)
#define TM_FRAME_DIR_SIZE_MASK  (TM_FRAME_DIR_SIZE - 1)

struct tm_frame_dir {
    atomic_uintptr_t entry[TM_FRAME_DIR_SIZE];
};

/*
 * Frame top-level directory
 */

#define TM_FRAME_TLD_SIZE_BITS \
    (((CHAR_BIT * sizeof(void*)) - TM_BLOCK_SIZE_BITS - TM_FRAME_TBL_SIZE_BITS) % TM_FRAME_DIR_SIZE_BITS)
#define TM_FRAME_TLD_SIZE       (1ul << TM_FRAME_TLD_SIZE_BITS)
#define TM_FRAME_TLD_SIZE_MASK  (TM_FRAME_TLD_SIZE - 1)

struct tm_frame_tld {
    atomic_uintptr_t entry[TM_FRAME_TLD_SIZE];
};

/*
 * Frame map
 */

struct tm_frame_map {
    struct tm_frame_tld tld;
};

void
tm_frame_map_init(struct tm_frame_map* self);

void
tm_frame_map_uninit(struct tm_frame_map* self);

struct tm_frame*
tm_frame_map_lookup(struct tm_frame_map* self, uintptr_t addr,
                    struct picotm_error* error);
