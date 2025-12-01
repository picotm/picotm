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

#include "fifo.h"
#include "filetab.h"

struct fildes_fifotab {
    struct fifo           tab[MAXNUMFD];
    struct fildes_filetab filetab;
};

void
fildes_fifotab_init(struct fildes_fifotab self[static 1],
                    struct picotm_error error[static 1]);

void
fildes_fifotab_uninit(struct fildes_fifotab self[static 1]);

struct fifo*
fildes_fifotab_ref_fildes(struct fildes_fifotab self[static 1],
                          int fildes, bool new_file,
                          struct picotm_error error[static 1]);

size_t
fildes_fifotab_index(struct fildes_fifotab self[static 1],
                     struct fifo fifo[static 1]);
