/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2020  Thomas Zimmermann
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
#include <stdbool.h>
#include <stddef.h>
#include "file.h"

struct fildes_filetab_ops {
    void (*init_file)(struct file[static 1],
                      struct picotm_error[static 1]);
    void (*uninit_file)(struct file[static 1]);
};

struct fildes_filetab {
    const struct fildes_filetab_ops* ops;
    size_t           len;
    size_t           siz;
    unsigned char*   tab;
    size_t           stride;
    pthread_rwlock_t rwlock;
};

void
fildes_filetab_init(struct fildes_filetab self[static 1],
                    const struct fildes_filetab_ops ops[static 1],
                    size_t siz, unsigned char tab[siz], size_t stride,
                    struct picotm_error error[static 1]);

void
fildes_filetab_uninit(struct fildes_filetab self[static 1]);

struct file*
fildes_filetab_ref_fildes(struct fildes_filetab self[static 1], int fildes,
                          bool new_file, struct picotm_error error[static 1]);

size_t
fildes_filetab_index(struct fildes_filetab self[static 1],
                     struct file file[static 1]);
