/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
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

#include <stdbool.h>
#include "picotm/picotm-lib-shared-treemap.h"

/**
 * \cond impl || ptrdata_impl
 * \ingroup ptrdata_impl
 * \file
 * \endcond
 */

struct picotm_error;

struct ptrdata {
    struct picotm_shared_treemap map;
};

void
ptrdata_init(struct ptrdata* self);

void
ptrdata_uninit(struct ptrdata* self);

void
ptrdata_set_shared_data(struct ptrdata* self, const void* ptr,
                         const void* data, struct picotm_error* error);

bool
ptrdata_test_and_set_shared_data(struct ptrdata* self, const void* ptr,
                                  const void* current, const void* data,
                                  struct picotm_error* error);

void
ptrdata_clear_shared_data(struct ptrdata* self, const void* ptr,
                           struct picotm_error* error);

void*
ptrdata_get_shared_data(struct ptrdata* self, const void* ptr,
                         struct picotm_error* error);
