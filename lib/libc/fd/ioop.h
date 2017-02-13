/* Copyright (C) 2008-2009  Thomas Zimmermann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef IOOP_H
#define IOOP_H

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

