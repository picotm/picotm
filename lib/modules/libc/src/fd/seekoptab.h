/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SEEKOPTAB_H
#define SEEKOPTAB_H

#include <sys/types.h>

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;
struct seekop;

unsigned long
seekoptab_append(struct seekop **tab, size_t *nelems,
                 off_t from, off_t offset, int whence,
                 struct picotm_error* error);

void
seekoptab_clear(struct seekop **tab, size_t *nelems);

void
seekoptab_dump(struct seekop *tab, size_t nelems);

#endif

