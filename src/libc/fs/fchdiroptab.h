/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FCHDIROPTAB_H
#define FCHDIROPTAB_H

unsigned long
fchdiroptab_append(struct fchdirop **tab, size_t *nelems, int oldcwd,
                                                          int newcwd);

void
fchdiroptab_clear(struct fchdirop **tab, size_t *nelems);

void
fchdiroptab_dump(struct fchdirop *tab, size_t nelems);

#endif

