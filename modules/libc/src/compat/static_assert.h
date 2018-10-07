/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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

#include <assert.h>

/**
 * \brief Provides static compile-time assertion
 *
 * This interface was taken from C11.
 */
#define __PICOTM_LIBC_STATIC_ASSERT(expr, stmt)     			 \
    (__extension__({                                			 \
	int assertion_holds[1-(2*!(expr))] __attribute__((__unused__));  \
     }))

#ifndef static_assert
#define static_assert	__PICOTM_LIBC_STATIC_ASSERT
#endif

