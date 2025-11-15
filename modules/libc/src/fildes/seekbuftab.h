/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2019-2020  Thomas Zimmermann
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

#include "filebuftab.h"
#include "seekbuf.h"

struct fildes_seekbuftab {
    struct seekbuf tab[MAXNUMFD];
    struct fildes_filebuftab filebuftab;
};

void
fildes_seekbuftab_init(struct fildes_seekbuftab self[static 1],
                       struct picotm_error error[static 1]);

void
fildes_seekbuftab_uninit(struct fildes_seekbuftab self[static 1]);

struct seekbuf*
fildes_seekbuftab_ref_fildes(struct fildes_seekbuftab self[static 1],
                             int fildes,
                             struct picotm_error error[static 1]);

size_t
fildes_seekbuftab_index(struct fildes_seekbuftab self[static 1],
                        struct seekbuf seekbuf[static 1]);
