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
#include <sys/stat.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <sys/stat.h>.
 */

#if defined(PICOTM_LIBC_HAVE_CHMOD) && PICOTM_LIBC_HAVE_CHMOD || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [chmod()][posix::chmod].
 *
 * [posix::chmod]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/chmod.html
 */
int
chmod_tx(const char* path, mode_t mode);
#endif

#if defined(PICOTM_LIBC_HAVE_FCHMOD) && PICOTM_LIBC_HAVE_FCHMOD || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [fchmod()][posix::fchmod].
 *
 * [posix::fchmod]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fchmod.html
 */
int
fchmod_tx(int fildes, mode_t mode);
#endif

#if defined(PICOTM_LIBC_HAVE_FSTAT) && PICOTM_LIBC_HAVE_FSTAT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [fstat()][posix::fstat].
 *
 * [posix::fstat]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fstat.html
 */
int
fstat_tx(int fildes, struct stat* buf);
#endif

#if defined(PICOTM_LIBC_HAVE_LSTAT) && PICOTM_LIBC_HAVE_LSTAT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [lstat()][posix::lstat].
 *
 * [posix::lstat]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/lstat.html
 */
int
lstat_tx(const char* path, struct stat* buf);
#endif

#if defined(PICOTM_LIBC_HAVE_MKDIR) && PICOTM_LIBC_HAVE_MKDIR || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [mkdir()][posix::mkdir].
 *
 * [posix::mkdir]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/mkdir.html
 */
int
mkdir_tx(const char* path, mode_t mode);
#endif

#if defined(PICOTM_LIBC_HAVE_MKFIFO) && PICOTM_LIBC_HAVE_MKFIFO || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [mkfifo()][posix::mkfifo].
 *
 * [posix::mkfifo]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/mkfifo.html
 */
int
mkfifo_tx(const char* path, mode_t mode);
#endif

#if defined(PICOTM_LIBC_HAVE_MKNOD) && PICOTM_LIBC_HAVE_MKNOD || \
    defined(__PICOTM_DOXYGEN)
#if defined(_BSD_SOURCE) || \
    defined(_SVID_SOURCE) || \
    defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 500 || \
    defined(__PICOTM_DOXYGEN)
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [mknod()][posix::mknod].
 *
 * [posix::mknod]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/mknod.html
 */
PICOTM_NOTHROW
int
mknod_tx(const char* path, mode_t mode, dev_t dev);
#endif
#endif

#if defined(PICOTM_LIBC_HAVE_STAT) && PICOTM_LIBC_HAVE_STAT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [stat()][posix::stat].
 *
 * [posix::stat]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/stat.html
 */
int
stat_tx(const char* path, struct stat* buf);
#endif

PICOTM_END_DECLS
