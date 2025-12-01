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
 * Variant of chmod_tx() that operates on transactional memory.
 */
int
chmod_tm(const char* path, mode_t mode);
#endif

#if defined(PICOTM_LIBC_HAVE_FSTAT) && PICOTM_LIBC_HAVE_FSTAT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of fstat_tx() that operates on transactional memory.
 */
int
fstat_tm(int fildes, struct stat* buf);
#endif

#if defined(PICOTM_LIBC_HAVE_LSTAT) && PICOTM_LIBC_HAVE_LSTAT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of lstat_tx() that operates on transactional memory.
 */
int
lstat_tm(const char* path, struct stat* buf);
#endif

#if defined(PICOTM_LIBC_HAVE_MKDIR) && PICOTM_LIBC_HAVE_MKDIR || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of mkdir_tx() that operates on transactional memory.
 */
int
mkdir_tm(const char* path, mode_t mode);
#endif

#if defined(PICOTM_LIBC_HAVE_MKFIFO) && PICOTM_LIBC_HAVE_MKFIFO || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of mkfifo_tx() that operates on transactional memory.
 */
int
mkfifo_tm(const char* path, mode_t mode);
#endif

#if defined(PICOTM_LIBC_HAVE_MKNOD) && PICOTM_LIBC_HAVE_MKNOD || \
    defined(__PICOTM_DOXYGEN)
#if defined(_BSD_SOURCE) || \
    defined(_SVID_SOURCE) || \
    defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 500 || \
    defined(__PICOTM_DOXYGEN)
/**
 * \ingroup group_libc
 * Variant of mknod_tx() that operates on transactional memory.
 */
PICOTM_NOTHROW
int
mknod_tm(const char* path, mode_t mode, dev_t dev);
#endif
#endif

#if defined(PICOTM_LIBC_HAVE_STAT) && PICOTM_LIBC_HAVE_STAT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of stat_tx() that operates on transactional memory.
 */
int
stat_tm(const char* path, struct stat* buf);
#endif

PICOTM_END_DECLS
