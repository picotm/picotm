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

#include "chrdevtab.h"

static void
chrdevtab_init_file(struct file file[static 1],
                    struct picotm_error error[static 1])
{
    chrdev_init(chrdev_of_base(file), error);
}

static void
chrdevtab_uninit_file(struct file file[static 1])
{
    chrdev_uninit(chrdev_of_base(file));
}

static const struct fildes_filetab_ops chrdevtab_ops = {
    chrdevtab_init_file,
    chrdevtab_uninit_file,
};

void
fildes_chrdevtab_init(struct fildes_chrdevtab self[static 1],
                      struct picotm_error error[static 1])
{
    fildes_filetab_init(&self->filetab, &chrdevtab_ops, sizeof(self->tab),
                        (void*)&self->tab, sizeof(self->tab[0]), error);
}

void
fildes_chrdevtab_uninit(struct fildes_chrdevtab self[static 1])
{
    fildes_filetab_uninit(&self->filetab);
}

struct chrdev*
fildes_chrdevtab_ref_fildes(struct fildes_chrdevtab self[static 1], int fildes,
                            bool new_file, struct picotm_error error[static 1])
{
    struct file* file = fildes_filetab_ref_fildes(&self->filetab, fildes,
                                                  new_file, error);
    if (!file)
        return NULL;

    return chrdev_of_base(file);
}

size_t
fildes_chrdevtab_index(struct fildes_chrdevtab self[static 1],
                       struct chrdev chrdev[static 1])
{
    return fildes_filetab_index(&self->filetab, &chrdev->base);
}
