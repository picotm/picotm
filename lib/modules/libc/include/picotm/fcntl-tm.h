/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <fcntl.h>
#include <picotm/compiler.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <fcntl.h>.
 */

PICOTM_NOTHROW
/**
 * Variant of creat_tx() that operates on transactional memory.
 */
int
creat_tm(const char* path, mode_t mode);

PICOTM_NOTHROW
/**
 * Variant of fcntl_tx() that operates on transactional memory.
 */
int
fcntl_tm(int fildes, int cmd, ...);

PICOTM_NOTHROW
/**
 * Variant of open_tx() that operates on transactional memory.
 */
int
open_tm(const char* path, int oflag, ...);

PICOTM_END_DECLS
