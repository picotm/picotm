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

#include "pipebuftab.h"

static void
pipebuftab_init_file(struct filebuf filebuf[static 1],
                     struct picotm_error error[static 1])
{
    pipebuf_init(pipebuf_of_base(filebuf), error);
}

static void
pipebuftab_uninit_file(struct filebuf filebuf[static 1])
{
    pipebuf_uninit(pipebuf_of_base(filebuf));
}

static const struct fildes_filebuftab_ops pipebuftab_ops = {
    pipebuftab_init_file,
    pipebuftab_uninit_file,
};

void
fildes_pipebuftab_init(struct fildes_pipebuftab self[static 1],
                       struct picotm_error error[static 1])
{
    fildes_filebuftab_init(&self->filebuftab, &pipebuftab_ops,
                           sizeof(self->tab), (void*)&self->tab,
                           sizeof(self->tab[0]), error);
}

void
fildes_pipebuftab_uninit(struct fildes_pipebuftab self[static 1])
{
    fildes_filebuftab_uninit(&self->filebuftab);
}

struct pipebuf*
fildes_pipebuftab_ref_fildes(struct fildes_pipebuftab self[static 1],
                             int fildes,
                             struct picotm_error error[static 1])
{
    struct filebuf* filebuf = fildes_filebuftab_ref_fildes(&self->filebuftab,
                                                           fildes, error);
    if (!filebuf)
        return nullptr;

    return pipebuf_of_base(filebuf);
}

size_t
fildes_pipebuftab_index(struct fildes_pipebuftab self[static 1],
                        struct pipebuf pipebuf[static 1])
{
    return fildes_filebuftab_index(&self->filebuftab, &pipebuf->base);
}
