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

#include "picotm/config/picotm-libc-config.h"
#include "picotm/compiler.h"
#include <errno.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <errno.h>.
 */

#if defined(PICOTM_LIBC_HAVE_ERRNO) && PICOTM_LIBC_HAVE_ERRNO || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * \internal
 * Returns the address of 'errno'.
 * \warning This is an internal interface. Don't use it in application code.
 */
int*
__errno_location_tx(void);

/**
 * \ingroup group_libc
 * A transaction-safe implementation of [errno][posix::errno].
 *
 * [posix::errno]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/errno.html
 */
#define errno_tx    (*(__errno_location_tx()))
#endif

PICOTM_END_DECLS
