/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SEEKOPTAB_H
#define SEEKOPTAB_H

unsigned long
seekoptab_append(struct seekop **tab, size_t *nelems, off_t from,
                                                      off_t offset, int whence);

void
seekoptab_clear(struct seekop **tab, size_t *nelems);

void
seekoptab_dump(struct seekop *tab, size_t nelems);

#endif

