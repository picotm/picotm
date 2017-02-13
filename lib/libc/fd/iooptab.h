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

#ifndef IOOPTAB_H_
#define IOOPTAB_H_

unsigned long
iooptab_append(struct ioop * restrict * restrict tab, size_t * restrict nelems,
                                                      size_t * restrict siz,
                                                      size_t nbyte,
                                                      off_t offset,
                                                      size_t bufoff);

void
iooptab_clear(struct ioop * restrict * restrict tab, size_t * restrict nelems);

ssize_t
iooptab_read(struct ioop * restrict tab, size_t nelems,
                                         void * restrict buf,
                                         size_t nbyte,
                                         off_t offset,
                                         void * restrict iobuf);

int
iooptab_sort(const struct ioop * restrict tab, size_t nelems,
                   struct ioop * restrict * restrict sorted);

void
iooptab_dump(struct ioop * restrict tab, size_t nelems);

#endif

