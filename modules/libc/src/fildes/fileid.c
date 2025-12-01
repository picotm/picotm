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

#include "fileid.h"
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>

void
file_id_init(struct file_id* self, dev_t dev, ino_t ino)
{
    assert(self);

    self->dev = dev;
    self->ino = ino;
}

void
file_id_init_from_fildes(struct file_id* self, int fildes,
                         struct picotm_error* error)
{
    assert(self);

    struct stat buf;

    int res = fstat(fildes, &buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }

    self->dev = buf.st_dev;
    self->ino = buf.st_ino;
    self->fildes = fildes;
}

void
file_id_clear(struct file_id* self)
{
    self->dev = (dev_t)-1;
    self->ino = (ino_t)-1;
    self->fildes = -1;
}

bool
file_id_is_empty(const struct file_id* self)
{
    assert(self);

    return (self->dev == (dev_t)-1) &&
           (self->ino == (ino_t)-1) &&
           (self->fildes == -1);
}

static int
cmp_dev_t(dev_t lhs, dev_t rhs)
{
    return (lhs > rhs) - (lhs < rhs);
}

static int
cmp_ino_t(ino_t lhs, ino_t rhs)
{
    return (lhs > rhs) - (lhs < rhs);
}

static int
cmp_fildes(int lhs, int rhs)
{
    return (lhs > rhs) - (lhs < rhs);
}

int
file_id_cmp(const struct file_id* lhs, const struct file_id* rhs)
{
    int cmp = cmp_dev_t(lhs->dev, rhs->dev);
    if (cmp)
        return cmp;

    cmp = cmp_ino_t(lhs->ino, rhs->ino);
    if (cmp)
        return cmp;

    return cmp_fildes(lhs->fildes, rhs->fildes);
}

int
file_id_cmp_eq_fildes(const struct file_id lhs[static 1],
                      const struct file_id rhs[static 1],
                      struct picotm_error error[static 1])
{
    int cmp = cmp_dev_t(lhs->dev, rhs->dev);
    if (cmp)
        return cmp;

    cmp = cmp_ino_t(lhs->ino, rhs->ino);
    if (cmp)
        return cmp;

    cmp = cmp_fildes(lhs->fildes, rhs->fildes);
    if (cmp) {
        /* file descriptors are different; return error */
        picotm_error_set_errno(error, EBADF);
        return cmp;
    }

    return 0;
}
