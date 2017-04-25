/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOOPTAB_H_
#define IOOPTAB_H_

#include <sys/types.h>

struct ioop;

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

