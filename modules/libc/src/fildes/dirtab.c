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

#include "dirtab.h"

static void
dirtab_init_file(struct file file[static 1],
                 struct picotm_error error[static 1])
{
    dir_init(dir_of_base(file), error);
}

static void
dirtab_uninit_file(struct file file[static 1])
{
    dir_uninit(dir_of_base(file));
}

static const struct fildes_filetab_ops dirtab_ops = {
    dirtab_init_file,
    dirtab_uninit_file,
};

void
fildes_dirtab_init(struct fildes_dirtab self[static 1],
                   struct picotm_error error[static 1])
{
    fildes_filetab_init(&self->filetab, &dirtab_ops, sizeof(self->tab),
                        (void*)&self->tab, sizeof(self->tab[0]), error);
}

void
fildes_dirtab_uninit(struct fildes_dirtab self[static 1])
{
    fildes_filetab_uninit(&self->filetab);
}

struct dir*
fildes_dirtab_ref_fildes(struct fildes_dirtab self[static 1],
                         int fildes, bool new_file,
                         struct picotm_error error[static 1])
{
    struct file* file = fildes_filetab_ref_fildes(&self->filetab, fildes,
                                                  new_file, error);
    if (!file)
        return NULL;

    return dir_of_base(file);
}

size_t
fildes_dirtab_index(struct fildes_dirtab self[static 1],
                    struct dir dir[static 1])
{
    return fildes_filetab_index(&self->filetab, &dir->base);
}
