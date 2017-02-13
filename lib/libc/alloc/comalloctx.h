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

#ifndef COMALLOCTX_H
#define COMALLOCTX_H

/**
 * \brief Acquire transaction's data for this component. This
 *        allows interacting with the components internals.
 */
struct com_alloc *
com_alloc_tx_aquire_data(void);

#endif

