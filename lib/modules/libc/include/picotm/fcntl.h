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
 * A transaction-safe implementation of [creat()][posix::creat].
 *
 * [posix::creat]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/creat.html
 */
int
creat_tx(const char* path, mode_t mode);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fcntl()][posix::fcntl].
 *
 * [posix::fcntl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fcntl.html
 */
int
fcntl_tx(int fildes, int cmd, ...);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [open()][posix::open].
 *
 * [posix::open]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/open.html
 */
int
open_tx(const char* path, int oflag, ...);

PICOTM_END_DECLS
