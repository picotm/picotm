/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 */

#include "fileid.h"
#include "picotm/picotm-error.h"
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
