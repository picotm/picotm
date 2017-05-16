/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOOP_H
#define IOOP_H

#include <sys/types.h>

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct ioop
{
    off_t  off;
    size_t nbyte;
    size_t bufoff;
};

int
ioop_init(struct ioop *ioop, off_t offset, size_t nbyte, size_t bufoff);

void
ioop_uninit(struct ioop *ioop);

/* \brief Read an ioop into a buffer.
 * \param iobuf The ioop data buffer, can be NULL if no data has been written.
 * \return The number of bytes read.
 */
ssize_t
ioop_read(const struct ioop *ioop, void *buf, size_t nbyte, off_t offset, const void *iobuf);

void
ioop_dump(const struct ioop *ioop);

int
ioop_uninit_walk(void *ioop);

#endif

