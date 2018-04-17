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
#include <stdlib.h>

PICOTM_BEGIN_DECLS

#if defined(PICOTM_LIBC_HAVE_FREE) && PICOTM_LIBC_HAVE_FREE || \
    defined(__PICOTM_DOXYGEN)
/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <stdlib.h>.
 */

PICOTM_NOTHROW
/**
 * Variant of free_tx() that operates on transactional memory.
 */
void
free_tm(void* ptr);
#endif

#if defined(PICOTM_LIBC_HAVE_MKDTEMP) && PICOTM_LIBC_HAVE_MKDTEMP && \
    !defined(__MACH__) || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of mkdtemp_tx() that operates on transactional memory.
 */
char*
mkdtemp_tm(char* template);
#endif

#if defined(PICOTM_LIBC_HAVE_MKSTEMP) && PICOTM_LIBC_HAVE_MKSTEMP || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of mkstemp_tx() that operates on transactional memory.
 */
int
mkstemp_tm(char* template);
#endif

#if defined(PICOTM_LIBC_HAVE_POSIX_MEMALIGN) && \
        PICOTM_LIBC_HAVE_POSIX_MEMALIGN || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of posix_memalign_tx() that operates on transactional memory.
 */
int
posix_memalign_tm(void** memptr, size_t alignment, size_t size);
#endif

#if defined(PICOTM_LIBC_HAVE_QSORT) && PICOTM_LIBC_HAVE_QSORT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of qsort_tx() that operates on transactional memory.
 */
void
qsort_tm(void* base, size_t nel, size_t width,
         int (*compar)(const void*, const void*));
#endif

#if defined(PICOTM_LIBC_HAVE_RAND_R) && PICOTM_LIBC_HAVE_RAND_R || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of rand_r_tx() that operates on transactional memory.
 */
int
rand_r_tm(unsigned int* seed);
#endif

#if defined(PICOTM_LIBC_HAVE_REALLOC) && PICOTM_LIBC_HAVE_REALLOC || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of realloc_tx() that operates on transactional memory.
 */
void*
realloc_tm(void* ptr, size_t size);
#endif

PICOTM_END_DECLS
