/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <unistd.h>
#include "compiler.h"

SYSTX_NOTHROW
int
chdir_tm(const char* path);

SYSTX_NOTHROW
char*
getcwd_tm(char* buf, size_t size);

SYSTX_NOTHROW
int
link_tm(const char* path1, const char* path2);

SYSTX_NOTHROW
int
pipe_tm(int fildes[2]);

SYSTX_NOTHROW
ssize_t
pread_tm(int fildes, void* buf, size_t nbyte, off_t offset);

SYSTX_NOTHROW
ssize_t
pwrite_tm(int fildes, const void* buf, size_t nbyte, off_t offset);

SYSTX_NOTHROW
ssize_t
read_tm(int fildes, void* buf, size_t nbyte);

SYSTX_NOTHROW
int
unlink_tm(const char* path);

SYSTX_NOTHROW
ssize_t
write_tm(int fildes, const void* buf, size_t nbyte);
