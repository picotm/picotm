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
 * Variant of chmod_tx() that operates on transactional memory.
 */
int
chmod_tm(const char* path, mode_t mode);

PICOTM_NOTHROW
/**
 * Variant of fstat_tx() that operates on transactional memory.
 */
int
fstat_tm(int fildes, struct stat* buf);

PICOTM_NOTHROW
/**
 * Variant of lstat_tx() that operates on transactional memory.
 */
int
lstat_tm(const char* path, struct stat* buf);

PICOTM_NOTHROW
/**
 * Variant of mkdir_tx() that operates on transactional memory.
 */
int
mkdir_tm(const char* path, mode_t mode);

PICOTM_NOTHROW
/**
 * Variant of mkfifo_tx() that operates on transactional memory.
 */
int
mkfifo_tm(const char* path, mode_t mode);

#if defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || _XOPEN_SOURCE >= 500
/**
 * Variant of mknod_tx() that operates on transactional memory.
 */
PICOTM_NOTHROW
int
mknod_tm(const char* path, mode_t mode, dev_t dev);
#endif

PICOTM_NOTHROW
/**
 * Variant of stat_tx() that operates on transactional memory.
 */
int
stat_tm(const char* path, struct stat* buf);

PICOTM_END_DECLS
