/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "picotm/config/picotm-libc-config.h"
#include "picotm/compiler.h"
#include <sys/stat.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <sys/stat.h>.
 */

#if defined(PICOTM_LIBC_HAVE_CHMOD) && PICOTM_LIBC_HAVE_CHMOD || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of chmod_tx() that operates on transactional memory.
 */
int
chmod_tm(const char* path, mode_t mode);
#endif

#if defined(PICOTM_LIBC_HAVE_FSTAT) && PICOTM_LIBC_HAVE_FSTAT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of fstat_tx() that operates on transactional memory.
 */
int
fstat_tm(int fildes, struct stat* buf);
#endif

#if defined(PICOTM_LIBC_HAVE_LSTAT) && PICOTM_LIBC_HAVE_LSTAT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of lstat_tx() that operates on transactional memory.
 */
int
lstat_tm(const char* path, struct stat* buf);
#endif

#if defined(PICOTM_LIBC_HAVE_MKDIR) && PICOTM_LIBC_HAVE_MKDIR || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of mkdir_tx() that operates on transactional memory.
 */
int
mkdir_tm(const char* path, mode_t mode);
#endif

#if defined(PICOTM_LIBC_HAVE_MKFIFO) && PICOTM_LIBC_HAVE_MKFIFO || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of mkfifo_tx() that operates on transactional memory.
 */
int
mkfifo_tm(const char* path, mode_t mode);
#endif

#if defined(PICOTM_LIBC_HAVE_MKNOD) && PICOTM_LIBC_HAVE_MKNOD || \
    defined(__PICOTM_DOXYGEN)
#if defined(_BSD_SOURCE) || \
    defined(_SVID_SOURCE) || \
    defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 500 || \
    defined(__PICOTM_DOXYGEN)
/**
 * Variant of mknod_tx() that operates on transactional memory.
 */
PICOTM_NOTHROW
int
mknod_tm(const char* path, mode_t mode, dev_t dev);
#endif
#endif

#if defined(PICOTM_LIBC_HAVE_STAT) && PICOTM_LIBC_HAVE_STAT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of stat_tx() that operates on transactional memory.
 */
int
stat_tm(const char* path, struct stat* buf);
#endif

PICOTM_END_DECLS
