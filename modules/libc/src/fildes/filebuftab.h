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

#include <pthread.h>
#include <stddef.h>
#include "filebuf.h"

struct fildes_filebuftab_ops {
    void (*init_filebuf)(struct filebuf[static 1],
                         struct picotm_error[static 1]);
    void (*uninit_filebuf)(struct filebuf[static 1]);
};

struct fildes_filebuftab {
    const struct fildes_filebuftab_ops* ops;
    size_t           len;
    size_t           siz;
    unsigned char*   tab;
    size_t           stride;
    pthread_rwlock_t rwlock;
};

void
fildes_filebuftab_init(struct fildes_filebuftab self[static 1],
                       const struct fildes_filebuftab_ops ops[static 1],
                       size_t siz, unsigned char tab[siz], size_t stride,
                       struct picotm_error error[static 1]);

void
fildes_filebuftab_uninit(struct fildes_filebuftab self[static 1]);

struct filebuf*
fildes_filebuftab_ref_fildes(struct fildes_filebuftab self[static 1],
                             int fildes,
                             struct picotm_error error[static 1]);

size_t
fildes_filebuftab_index(struct fildes_filebuftab self[static 1],
                        struct filebuf filebuf[static 1]);
