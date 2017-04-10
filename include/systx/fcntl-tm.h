/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <fcntl.h>
#include "compiler.h"

SYSTX_NOTHROW
int
creat_tm(const char* path, mode_t mode);

SYSTX_NOTHROW
int
fcntl_tm(int fildes, int cmd, ...);

SYSTX_NOTHROW
int
open_tm(const char* path, int oflag, ...);
