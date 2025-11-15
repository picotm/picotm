/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2020  Thomas Zimmermann
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

#include "filebuf_id.h"
#include "picotm/picotm-error.h"
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>

void
filebuf_id_init(struct filebuf_id* self, dev_t dev, ino_t ino)
{
    assert(self);

    self->dev = dev;
    self->ino = ino;
}

void
filebuf_id_init_from_fildes(struct filebuf_id* self, int fildes,
                            struct picotm_error* error)
{
    assert(self);

    struct stat buf;

    int res = fstat(fildes, &buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }

    filebuf_id_init(self, buf.st_dev, buf.st_ino);
}

void
filebuf_id_clear(struct filebuf_id* self)
{
    self->dev = (dev_t)-1;
    self->ino = (ino_t)-1;
}

bool
filebuf_id_is_empty(const struct filebuf_id* self)
{
    assert(self);

    return (self->dev  == (dev_t)-1) && (self->ino  == (ino_t)-1);
}

int
filebuf_id_cmp(const struct filebuf_id* lhs, const struct filebuf_id* rhs)
{
    const long long diff_dev = lhs->dev - rhs->dev;

    if (diff_dev) {
        return (diff_dev > 0) - (diff_dev < 0);
    }

    const long long diff_ino = lhs->ino - rhs->ino;

    if (diff_ino) {
        return (diff_ino > 0) - (diff_ino < 0);
    }

    return 0;
}
