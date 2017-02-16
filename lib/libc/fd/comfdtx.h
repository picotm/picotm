/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMFDTX_H
#define COMFDTX_H

struct com_fd;

/** \brief Aquire component data */
struct com_fd *
com_fd_tx_aquire_data(void);

#endif

