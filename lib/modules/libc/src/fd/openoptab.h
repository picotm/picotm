/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OPENOPTAB_H
#define OPENOPTAB_H

#include <sys/types.h>

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct openop;

unsigned long
openoptab_append(struct openop **tab, size_t *nelems, int unlink);

void
openoptab_clear(struct openop **tab, size_t *nelems);

void
openoptab_dump(struct openop *tab, size_t nelems);

#endif

