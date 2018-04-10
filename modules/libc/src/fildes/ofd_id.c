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
