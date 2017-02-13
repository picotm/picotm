/* Copyright (C) 2009  Thomas Zimmermann
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

