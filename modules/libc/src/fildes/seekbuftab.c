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

#include "seekbuftab.h"

static void
seekbuftab_init_file(struct filebuf filebuf[static 1],
                     struct picotm_error error[static 1])
{
    seekbuf_init(seekbuf_of_base(filebuf), error);
}

static void
seekbuftab_uninit_file(struct filebuf filebuf[static 1])
{
    seekbuf_uninit(seekbuf_of_base(filebuf));
}

static const struct fildes_filebuftab_ops seekbuftab_ops = {
    seekbuftab_init_file,
    seekbuftab_uninit_file,
};

void
fildes_seekbuftab_init(struct fildes_seekbuftab self[static 1],
                       struct picotm_error error[static 1])
{
    fildes_filebuftab_init(&self->filebuftab, &seekbuftab_ops,
                           sizeof(self->tab), (void*)&self->tab,
                           sizeof(self->tab[0]), error);
}

void
fildes_seekbuftab_uninit(struct fildes_seekbuftab self[static 1])
{
    fildes_filebuftab_uninit(&self->filebuftab);
}

struct seekbuf*
fildes_seekbuftab_ref_fildes(struct fildes_seekbuftab self[static 1],
                             int fildes,
                             struct picotm_error error[static 1])
{
    struct filebuf* filebuf = fildes_filebuftab_ref_fildes(&self->filebuftab,
                                                           fildes, error);
    if (!filebuf)
        return NULL;

    return seekbuf_of_base(filebuf);
}

size_t
fildes_seekbuftab_index(struct fildes_seekbuftab self[static 1],
                        struct seekbuf seekbuf[static 1])
{
    return fildes_filebuftab_index(&self->filebuftab, &seekbuf->base);
}
