/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

