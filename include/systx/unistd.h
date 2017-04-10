/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <unistd.h>
#include "compiler.h"

SYSTX_NOTHROW SYSTX_NORETURN
void _exit_tx(int status);

SYSTX_NOTHROW
int
close_tx(int fildes);

SYSTX_NOTHROW
int
dup_tx(int fildes);

SYSTX_NOTHROW
int
fsync_tx(int fildes);

SYSTX_NOTHROW
off_t
lseek_tx(int fildes, off_t offset, int whence);

SYSTX_NOTHROW
int
pipe_tx(int fildes[2]);

SYSTX_NOTHROW
ssize_t
pread_tx(int fildes, void* buf, size_t nbyte, off_t offset);

SYSTX_NOTHROW
ssize_t
pwrite_tx(int fildes, const void* buf, size_t nbyte, off_t offset);

SYSTX_NOTHROW
ssize_t
read_tx(int fildes, void* buf, size_t nbyte);

SYSTX_NOTHROW
void
sync_tx(void);

SYSTX_NOTHROW
ssize_t
write_tx(int fildes, const void* buf, size_t nbyte);
