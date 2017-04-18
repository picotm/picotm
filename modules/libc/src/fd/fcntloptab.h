/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FCNTLOPTAB_H
#define FCNTLOPTAB_H

unsigned long
fcntloptab_append(struct fcntlop **tab, size_t *nelems, int command,
                                                        const union com_fd_fcntl_arg *value,
                                                        const union com_fd_fcntl_arg *oldvalue);

void
fcntloptab_clear(struct fcntlop **tab, size_t *nelems);

void
fcntloptab_dump(struct fcntlop *tab, size_t nelems);

#endif

