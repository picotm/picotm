/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <sys/stat.h>
#include <systx/compiler.h>

SYSTX_NOTHROW
int
chmod_tx(const char* path, mode_t mode);

SYSTX_NOTHROW
int
fchmod_tx(int fildes, mode_t mode);

SYSTX_NOTHROW
int
fstat_tx(int fildes, struct stat* buf);

SYSTX_NOTHROW
int
lstat_tx(const char* path, struct stat* buf);

SYSTX_NOTHROW
int
mkdir_tx(const char* path, mode_t mode);

SYSTX_NOTHROW
int
mkfifo_tx(const char* path, mode_t mode);

#if defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || _XOPEN_SOURCE >= 500
SYSTX_NOTHROW
int
mknod_tx(const char* path, mode_t mode, dev_t dev);
#endif

SYSTX_NOTHROW
int
stat_tx(const char* path, struct stat* buf);
