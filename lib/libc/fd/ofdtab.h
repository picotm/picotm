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

#ifndef OFDTAB_H
#define OFDTAB_H

struct ofdid;
struct ofd;

extern struct ofd ofdtab[MAXNUMFD];

void
ofdtab_lock(void);

void
ofdtab_unlock(void);

long
ofdtab_ref_ofd(int fildes, int flags);

size_t
ofdtab_index(struct ofd *ofd);

#endif

