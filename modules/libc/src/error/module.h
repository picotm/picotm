/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann
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

#include "picotm/picotm-libc.h"

struct picotm_error;

/**
 * \cond impl || libc_impl || libc_impl_error
 * \ingroup libc_impl
 * \ingroup libc_impl_error
 * \file
 * \endcond
 */

void
error_module_save_errno(struct picotm_error* error);

void
error_module_set_error_recovery(enum picotm_libc_error_recovery recovery,
                                struct picotm_error* error);

enum picotm_libc_error_recovery
error_module_get_error_recovery(struct picotm_error* error);

/**
 * \cond impl || libc_impl || libc_impl_error
 * \defgroup libc_impl_error libc Error Handling
 * \endcond
 */
