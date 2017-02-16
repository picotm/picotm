/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMALLOCTX_H
#define COMALLOCTX_H

/**
 * \brief Acquire transaction's data for this component. This
 *        allows interacting with the components internals.
 */
struct com_alloc *
com_alloc_tx_aquire_data(void);

#endif

