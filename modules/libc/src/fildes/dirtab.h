/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann
 * Copyright (c) 2020       Thomas Zimmermann
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

#include "dir.h"
#include "filetab.h"

struct fildes_dirtab {
    struct dir            tab[MAXNUMFD];
    struct fildes_filetab filetab;
};

void
fildes_dirtab_init(struct fildes_dirtab self[static 1],
                   struct picotm_error error[static 1]);

void
fildes_dirtab_uninit(struct fildes_dirtab self[static 1]);

struct dir*
fildes_dirtab_ref_fildes(struct fildes_dirtab self[static 1],
                         int fildes, bool new_file,
                         struct picotm_error error[static 1]);

size_t
fildes_dirtab_index(struct fildes_dirtab self[static 1],
                    struct dir dir[static 1]);
