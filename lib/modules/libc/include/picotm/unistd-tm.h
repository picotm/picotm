/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <picotm/compiler.h>
#include <unistd.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <unistd.h>.
 */

PICOTM_NOTHROW
/**
 * Variant of chdir_tx() that operates on transactional memory.
 */
int
chdir_tm(const char* path);

PICOTM_NOTHROW
/**
 * Variant of getcwd_tx() that operates on transactional memory.
 */
char*
getcwd_tm(char* buf, size_t size);

PICOTM_NOTHROW
/**
 * Variant of link_tx() that operates on transactional memory.
 */
int
link_tm(const char* path1, const char* path2);

PICOTM_NOTHROW
/**
 * Variant of pipe_tx() that operates on transactional memory.
 */
int
pipe_tm(int fildes[2]);

PICOTM_NOTHROW
/**
 * Variant of pread_tx() that operates on transactional memory.
 */
ssize_t
pread_tm(int fildes, void* buf, size_t nbyte, off_t offset);

PICOTM_NOTHROW
/**
 * Variant of pwrite_tx() that operates on transactional memory.
 */
ssize_t
pwrite_tm(int fildes, const void* buf, size_t nbyte, off_t offset);

PICOTM_NOTHROW
/**
 * Variant of read_tx() that operates on transactional memory.
 */
ssize_t
read_tm(int fildes, void* buf, size_t nbyte);

PICOTM_NOTHROW
/**
 * Variant of unlink_tx() that operates on transactional memory.
 */
int
unlink_tm(const char* path);

PICOTM_NOTHROW
/**
 * Variant of write_tx() that operates on transactional memory.
 */
ssize_t
write_tm(int fildes, const void* buf, size_t nbyte);

PICOTM_END_DECLS
