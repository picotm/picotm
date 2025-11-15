/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann
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

#include "picotm/picotm-lib-shared-treemap.h"

/**
 * \cond impl || tm_impl
 * \ingroup tm_impl
 * \file
 * \endcond
 */

struct picotm_error;
struct tm_frame;

struct tm_frame_map {
    struct picotm_shared_treemap map;
};

void
tm_frame_map_init(struct tm_frame_map* self);

void
tm_frame_map_uninit(struct tm_frame_map* self);

struct tm_frame*
tm_frame_map_lookup(struct tm_frame_map* self, uintptr_t addr,
                    struct picotm_error* error);
