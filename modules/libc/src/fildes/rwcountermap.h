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

#include "picotm/picotm-lib-treemap.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct rwlockmap;

struct rwcountermap {
    struct picotm_treemap map;
};

void
rwcountermap_init(struct rwcountermap* self);

void
rwcountermap_uninit(struct rwcountermap* self);

void
rwcountermap_rdlock(struct rwcountermap* self,
                    unsigned long long record_length,
                    unsigned long long record_offset,
                    struct rwlockmap* rwlockmap,
                    struct picotm_error* error);

void
rwcountermap_wrlock(struct rwcountermap* self,
                    unsigned long long record_length,
                    unsigned long long record_offset,
                    struct rwlockmap* rwlockmap,
                    struct picotm_error* error);

void
rwcountermap_unlock(struct rwcountermap* self,
                    unsigned long long record_length,
                    unsigned long long record_offset,
                    struct rwlockmap *rwlockmap);
