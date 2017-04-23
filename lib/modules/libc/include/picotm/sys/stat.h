/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <picotm/compiler.h>
#include <sys/stat.h>

PICOTM_NOTHROW
int
chmod_tx(const char* path, mode_t mode);

PICOTM_NOTHROW
int
fchmod_tx(int fildes, mode_t mode);

PICOTM_NOTHROW
int
fstat_tx(int fildes, struct stat* buf);

PICOTM_NOTHROW
int
lstat_tx(const char* path, struct stat* buf);

PICOTM_NOTHROW
int
mkdir_tx(const char* path, mode_t mode);

PICOTM_NOTHROW
int
mkfifo_tx(const char* path, mode_t mode);

#if defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || _XOPEN_SOURCE >= 500
PICOTM_NOTHROW
int
mknod_tx(const char* path, mode_t mode, dev_t dev);
#endif

PICOTM_NOTHROW
int
stat_tx(const char* path, struct stat* buf);
