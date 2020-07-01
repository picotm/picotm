/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2019-2020  Thomas Zimmermann <contact@tzimmermann.org>
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
#include "pipebuf.h"

struct fildes_pipebuftab {
    struct pipebuf tab[MAXNUMFD];
    struct fildes_filebuftab filebuftab;
};

void
fildes_pipebuftab_init(struct fildes_pipebuftab self[static 1],
                       struct picotm_error error[static 1]);

void
fildes_pipebuftab_uninit(struct fildes_pipebuftab self[static 1]);

struct pipebuf*
fildes_pipebuftab_ref_fildes(struct fildes_pipebuftab self[static 1],
                             int fildes,
                             struct picotm_error error[static 1]);

size_t
fildes_pipebuftab_index(struct fildes_pipebuftab self[static 1],
                        struct pipebuf pipebuf[static 1]);
