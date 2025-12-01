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

#pragma once

#include "picotm/picotm-lib-rwlock.h"
#include "picotm/picotm-lib-shared-treemap.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

enum {
    RWLOCKMAP_PAGE_NBITS    = 9,
    RWLOCKMAP_PAGE_NENTRIES = 1ul << RWLOCKMAP_PAGE_NBITS,
    RWLOCKMAP_PAGE_MASK     = RWLOCKMAP_PAGE_NENTRIES - 1
};

struct rwlockmap_page {
    struct picotm_rwlock lock[RWLOCKMAP_PAGE_NENTRIES];
};

struct rwlockmap {
    struct picotm_shared_treemap pagemap;
};

void
rwlockmap_init(struct rwlockmap* self);

void
rwlockmap_uninit(struct rwlockmap* self);

struct rwlockmap_page*
rwlockmap_find_page(struct rwlockmap* self, unsigned long long record_offset,
                    struct picotm_error* error);
