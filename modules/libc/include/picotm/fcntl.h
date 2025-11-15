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
#include <fcntl.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <fcntl.h>.
 */

#if defined(PICOTM_LIBC_HAVE_CREAT) && PICOTM_LIBC_HAVE_CREAT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [creat()][posix::creat].
 *
 * [posix::creat]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/creat.html
 */
int
creat_tx(const char* path, mode_t mode);
#endif

#if defined(PICOTM_LIBC_HAVE_FCNTL) && PICOTM_LIBC_HAVE_FCNTL || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [fcntl()][posix::fcntl].
 *
 * [posix::fcntl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fcntl.html
 */
int
fcntl_tx(int fildes, int cmd, ...);
#endif

#if defined(PICOTM_LIBC_HAVE_OPEN) && PICOTM_LIBC_HAVE_OPEN || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [open()][posix::open].
 *
 * [posix::open]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/open.html
 */
int
open_tx(const char* path, int oflag, ...);
#endif

PICOTM_END_DECLS
