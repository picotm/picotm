/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "fileid.h"
#include <assert.h>
#include <errno.h>
#include <picotm/picotm-error.h>
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

    file_id_init(self, buf.st_dev, buf.st_ino);
}

void
file_id_clear(struct file_id* self)
{
    self->dev = (dev_t)-1;
    self->ino = (ino_t)-1;
}

bool
file_id_is_empty(const struct file_id* self)
{
    assert(self);

    return (self->dev  == (dev_t)-1) && (self->ino  == (ino_t)-1);
}

int
file_id_cmp(const struct file_id* lhs, const struct file_id* rhs)
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
