/*
 * picotm - A system-level transaction manager
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include "picotm/picotm-lib-treemap.h"

/**
 * \cond impl || ptrdata_impl
 * \ingroup ptrdata_impl
 * \file
 * \endcond
 */

struct picotm_error;

struct ptrdata_tx {
    struct picotm_treemap map;
};

void
ptrdata_tx_init(struct ptrdata_tx* self);

void
ptrdata_tx_uninit(struct ptrdata_tx* self);

void
ptrdata_tx_set_data(struct ptrdata_tx* self, const void* ptr,
                    const void* data, struct picotm_error* error);

void
ptrdata_tx_clear_data(struct ptrdata_tx* self, const void* ptr,
                      struct picotm_error* error);

void*
ptrdata_tx_get_data(struct ptrdata_tx* self, const void* ptr,
                    struct picotm_error* error);
