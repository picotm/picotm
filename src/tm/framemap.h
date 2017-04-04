/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <limits.h>
#include <stdint.h>
#include "block.h"
#include "frame.h"

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
    uintptr_t entry[TM_FRAME_DIR_SIZE];
};

/*
 * Frame top-level directory
 */

#define TM_FRAME_TLD_SIZE_BITS \
    (((CHAR_BIT * sizeof(void*)) - TM_BLOCK_SIZE_BITS - TM_FRAME_TBL_SIZE_BITS) % TM_FRAME_DIR_SIZE_BITS)
#define TM_FRAME_TLD_SIZE       (1ul << TM_FRAME_TLD_SIZE_BITS)
#define TM_FRAME_TLD_SIZE_MASK  (TM_FRAME_TLD_SIZE - 1)

struct tm_frame_tld {
    uintptr_t entry[TM_FRAME_TLD_SIZE];
};

/*
 * Frame map
 */

struct tm_frame_map {
    struct tm_frame_tld tld;
};

int
tm_frame_map_init(struct tm_frame_map* self);

void
tm_frame_map_uninit(struct tm_frame_map* self);

struct tm_frame*
tm_frame_map_lookup(struct tm_frame_map* self, uintptr_t addr);
