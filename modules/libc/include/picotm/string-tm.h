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
#include <string.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <string.h>.
 */

#if defined(PICOTM_LIBC_HAVE_MEMCCPY) && PICOTM_LIBC_HAVE_MEMCCPY || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of memccpy_tx() that operates on transactional memory.
 */
void*
memccpy_tm(void* restrict dest, const void* restrict src, int c, size_t n);
#endif

#if defined(PICOTM_LIBC_HAVE_MEMCHR) && PICOTM_LIBC_HAVE_MEMCHR || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of memcchr_tx() that operates on transactional memory.
 */
void*
memchr_tm(const void* s, int c, size_t n);
#endif

#if defined(PICOTM_LIBC_HAVE_MEMCMP) && PICOTM_LIBC_HAVE_MEMCMP || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of memcmp_tx() that operates on transactional memory.
 */
int
memcmp_tm(const void* s1, const void* s2, size_t n);
#endif

#if defined(PICOTM_LIBC_HAVE_MEMCPY) && PICOTM_LIBC_HAVE_MEMCPY || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of memcpy_tx() that operates on transactional memory.
 */
void*
memcpy_tm(void* restrict dest, const void* restrict src, size_t n);
#endif

#if defined(PICOTM_LIBC_HAVE_MEMMOVE) && PICOTM_LIBC_HAVE_MEMMOVE || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of memmove_tx() that operates on transactional memory.
 */
void*
memmove_tm(void* dest, const void* src, size_t n);
#endif

#if defined(PICOTM_LIBC_HAVE_MEMSET) && PICOTM_LIBC_HAVE_MEMSET || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of memset_tx() that operates on transactional memory.
 */
void*
memset_tm(void* s, int c, size_t n);
#endif

#if defined(PICOTM_LIBC_HAVE_MEMRCHR) && PICOTM_LIBC_HAVE_MEMRCHR || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of memrchr_tx() that operates on transactional memory.
 */
void*
memrchr_tm(const void* s, int c, size_t n);
#endif

#if defined(PICOTM_LIBC_HAVE_RAWMEMCHR) && PICOTM_LIBC_HAVE_RAWMEMCHR || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of rawmemchr_tx() that operates on transactional memory.
 */
void*
rawmemchr_tm(const void* s, int c);
#endif

#if defined(PICOTM_LIBC_HAVE_STPCPY) && PICOTM_LIBC_HAVE_STPCPY || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of stpcpy_tx() that operates on transactional memory.
 */
char*
stpcpy_tm(char* restrict dest, const char* restrict src);
#endif

#if defined(PICOTM_LIBC_HAVE_STPNCPY) && PICOTM_LIBC_HAVE_STPNCPY || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of stpncpy_tx() that operates on transactional memory.
 */
char*
stpncpy_tm(char* restrict dest, const char* restrict src, size_t n);
#endif

#if defined(PICOTM_LIBC_HAVE_STRCAT) && PICOTM_LIBC_HAVE_STRCAT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of strcat_tx() that operates on transactional memory.
 */
char*
strcat_tm(char* restrict dest, const char* restrict src);
#endif

#if defined(PICOTM_LIBC_HAVE_STRCHR) && PICOTM_LIBC_HAVE_STRCHR || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of strchr_tx() that operates on transactional memory.
 */
char*
strchr_tm(const char* s, int c);
#endif

#if defined(PICOTM_LIBC_HAVE_STRCMP) && PICOTM_LIBC_HAVE_STRCMP || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of strcmp_tx() that operates on transactional memory.
 */
int
strcmp_tm(const char* s1, const char* s2);
#endif

#if defined(PICOTM_LIBC_HAVE_STRCOLL_L) && PICOTM_LIBC_HAVE_STRCOLL_L || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of strcoll_l_tx() that operates on transactional memory.
 */
int
strcoll_l_tm(const char* s1, const char* s2, locale_t locale);
#endif

#if defined(PICOTM_LIBC_HAVE_STRCPY) && PICOTM_LIBC_HAVE_STRCPY || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of strcpy_tx() that operates on transactional memory.
 */
char*
strcpy_tm(char* restrict dest, const char* restrict src);
#endif

#if defined(PICOTM_LIBC_HAVE_STRCSPN) && PICOTM_LIBC_HAVE_STRCSPN || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of strcspn_tx() that operates on transactional memory.
 */
size_t
strcspn_tm(const char* s, const char* reject);
#endif

#if defined(PICOTM_LIBC_HAVE_STRDUP) && PICOTM_LIBC_HAVE_STRDUP || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of strdup_tx() that operates on transactional memory.
 */
char*
strdup_tm(const char* s);
#endif

#if defined(PICOTM_LIBC_HAVE_STRERROR_R) && PICOTM_LIBC_HAVE_STRERROR_R || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * \internal
 * Variant of __strerror_r_gnu_tx() that operates on transactional memory.
 *
 * \warning This is an internal interface. Call strerror_r_tm() instead.
 */
char*
__strerror_r_gnu_tm(int errnum, char* buf, size_t buflen);

PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * \internal
 * Variant of __strerror_r_posix_tx() that operates on transactional memory.
 *
 * \warning This is an internal interface. Call strerror_r_tm() instead.
 */
int
__strerror_r_posix_tm(int errnum, char* buf, size_t buflen);

#if defined(__GNU_LIBRARY__) && \
        (defined(_GNU_SOURCE) || (_POSIX_C_SOURCE < 200112L)) || \
    defined(__CYGWIN__) && (__GNU_VISIBLE)
#define strerror_r_tx   __strerror_r_gnu_tx
#else
#define strerror_r_tx   __strerror_r_posix_tx
#endif
#endif

#if defined(PICOTM_LIBC_HAVE_STRLEN) && PICOTM_LIBC_HAVE_STRLEN || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of strlen_tx() that operates on transactional memory.
 */
size_t
strlen_tm(const char* s);
#endif

#if defined(PICOTM_LIBC_HAVE_STRCAT) && PICOTM_LIBC_HAVE_STRCAT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of strncat_tx() that operates on transactional memory.
 */
char*
strncat_tm(char* restrict dest, const char* restrict src, size_t n);
#endif

#if defined(PICOTM_LIBC_HAVE_STRNCMP) && PICOTM_LIBC_HAVE_STRNCMP || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of strncmp_tx() that operates on transactional memory.
 */
int
strncmp_tm(const char* s1, const char* s2, size_t n);
#endif

#if defined(PICOTM_LIBC_HAVE_STRNCPY) && PICOTM_LIBC_HAVE_STRNCPY || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of strncpy_tx() that operates on transactional memory.
 */
char*
strncpy_tm(char* restrict dest, const char* restrict src, size_t n);
#endif

#if defined(PICOTM_LIBC_HAVE_STRNDUP) && PICOTM_LIBC_HAVE_STRNDUP || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of strndup_tx() that operates on transactional memory.
 */
char*
strndup_tm(const char* s, size_t n);
#endif

#if defined(PICOTM_LIBC_HAVE_STRNLEN) && PICOTM_LIBC_HAVE_STRNLEN || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of strnlen_tx() that operates on transactional memory.
 */
size_t
strnlen_tm(const char* s, size_t maxlen);
#endif

#if defined(PICOTM_LIBC_HAVE_STRPBRK) && PICOTM_LIBC_HAVE_STRPBRK || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of strpbrk_tx() that operates on transactional memory.
 */
char*
strpbrk_tm(const char* s, const char* accept);
#endif

#if defined(PICOTM_LIBC_HAVE_STRRCHR) && PICOTM_LIBC_HAVE_STRRCHR || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of strrchr_tx() that operates on transactional memory.
 */
char*
strrchr_tm(const char* s, int c);
#endif

#if defined(PICOTM_LIBC_HAVE_STRSPN) && PICOTM_LIBC_HAVE_STRSPN || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of strspn_tx() that operates on transactional memory.
 */
size_t
strspn_tm(const char* s, const char* accept);
#endif

#if defined(PICOTM_LIBC_HAVE_STRSTR) && PICOTM_LIBC_HAVE_STRSTR || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of strstr_tx() that operates on transactional memory.
 */
char*
strstr_tm(const char* haystack, const char* needle);
#endif

#if defined(PICOTM_LIBC_HAVE_STRTOK_R) && PICOTM_LIBC_HAVE_STRTOK_R || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of strtok_r_tx() that operates on transactional memory.
 */
char*
strtok_r_tm(char* restrict str, const char* restrict delim,
            char** restrict saveptr);
#endif

PICOTM_END_DECLS
