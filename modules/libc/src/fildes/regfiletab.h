/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 * Copyright (c) 2020       Thomas Zimmermann <contact@tzimmermann.org>
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

#include "filetab.h"
#include "regfile.h"

struct fildes_regfiletab {
    struct regfile        tab[MAXNUMFD];
    struct fildes_filetab filetab;
};

void
fildes_regfiletab_init(struct fildes_regfiletab self[static 1],
                       struct picotm_error error[static 1]);

void
fildes_regfiletab_uninit(struct fildes_regfiletab self[static 1]);

struct regfile*
fildes_regfiletab_ref_fildes(struct fildes_regfiletab self[static 1],
                             int fildes, bool new_file,
                             struct picotm_error error[static 1]);

size_t
fildes_regfiletab_index(struct fildes_regfiletab self[static 1],
                        struct regfile regfile[static 1]);
