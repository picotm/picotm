/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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

#include "ofd_id.h"
#include "picotm/picotm-error.h"
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>

static void
set_ofd_id(struct ofd_id* self, int fildes, dev_t dev, ino_t ino)
{
    assert(self);

    self->fildes = fildes;
    file_id_init(&self->file_id, dev, ino);
}

void
ofd_id_init(struct ofd_id* self)
{
    set_ofd_id(self, -1, 0, 0);
}

void
ofd_id_init_from_fildes(struct ofd_id* self, int fildes,
                        struct picotm_error* error)
{
    struct stat buf;
    int res = fstat(fildes, &buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }

    set_ofd_id(self, fildes, buf.st_dev, buf.st_ino);
}

void
ofd_id_uninit(struct ofd_id* self)
{ }

void
ofd_id_set_from_fildes(struct ofd_id* self, int fildes,
                       struct picotm_error* error)
{
    struct stat buf;
    int res = fstat(fildes, &buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }

    set_ofd_id(self, fildes, buf.st_dev, buf.st_ino);
}

void
ofd_id_clear(struct ofd_id* self)
{
    set_ofd_id(self, -1, 0, 0);
}

static int
fildes_cmp(int lhs, int rhs)
{
    return (lhs < rhs) - (lhs > rhs);
}

int
ofd_id_cmp(const struct ofd_id* lhs, const struct ofd_id* rhs)
{
    assert(lhs);
    assert(rhs);

    int cmp_file_id = file_id_cmp(&lhs->file_id, &rhs->file_id);

    if (cmp_file_id) {
        return cmp_file_id; /* file ids are different; early out */
    }

    return fildes_cmp(lhs->fildes, rhs->fildes);
}

int
ofd_id_cmp_ne_fildes(const struct ofd_id* lhs, const struct ofd_id* rhs,
                     struct picotm_error* error)
{
    assert(lhs);
    assert(rhs);

    int cmp_file_id = file_id_cmp(&lhs->file_id, &rhs->file_id);

    if (cmp_file_id) {
        return cmp_file_id; /* file ids are different; early out */
    }

    int cmp_fildes = fildes_cmp(lhs->fildes, rhs->fildes);

    if (cmp_fildes) {
        /* file descriptors are different; return error */
        picotm_error_set_errno(error, EBADF);
        return cmp_fildes;
    }

    return 0;
}
