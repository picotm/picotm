/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <picotm/compiler.h>
#include <sys/stat.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <sys/stat.h>.
 */

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [chmod()][posix::chmod].
 *
 * [posix::chmod]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/chmod.html
 */
int
chmod_tx(const char* path, mode_t mode);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fchmod()][posix::fchmod].
 *
 * [posix::fchmod]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fchmod.html
 */
int
fchmod_tx(int fildes, mode_t mode);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fstat()][posix::fstat].
 *
 * [posix::fstat]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fstat.html
 */
int
fstat_tx(int fildes, struct stat* buf);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [lstat()][posix::lstat].
 *
 * [posix::lstat]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/lstat.html
 */
int
lstat_tx(const char* path, struct stat* buf);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [mkdir()][posix::mkdir].
 *
 * [posix::mkdir]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/mkdir.html
 */
int
mkdir_tx(const char* path, mode_t mode);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [mkfifo()][posix::mkfifo].
 *
 * [posix::mkfifo]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/mkfifo.html
 */
int
mkfifo_tx(const char* path, mode_t mode);

#if defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || _XOPEN_SOURCE >= 500
/**
 * A transaction-safe implementation of [mknod()][posix::mknod].
 *
 * [posix::mknod]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/mknod.html
 */
PICOTM_NOTHROW
int
mknod_tx(const char* path, mode_t mode, dev_t dev);
#endif

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [stat()][posix::stat].
 *
 * [posix::stat]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/stat.html
 */
int
stat_tx(const char* path, struct stat* buf);

PICOTM_END_DECLS
