/* Permission is hereby granted, free of charge, to any person obtaining a
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
 */

#pragma once

#include <picotm/compiler.h>
#include <string.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <string.h>.
 */

PICOTM_NOTHROW
/**
 * Variant of memccpy_tx() that operates on transactional memory.
 */
void*
memccpy_tm(void* restrict dest, const void* restrict src, int c, size_t n);

PICOTM_NOTHROW
/**
 * Variant of memcchr_tx() that operates on transactional memory.
 */
void*
memchr_tm(const void* s, int c, size_t n);

PICOTM_NOTHROW
/**
 * Variant of memcmp_tx() that operates on transactional memory.
 */
int
memcmp_tm(const void* s1, const void* s2, size_t n);

PICOTM_NOTHROW
/**
 * Variant of memcpy_tx() that operates on transactional memory.
 */
void*
memcpy_tm(void* restrict dest, const void* restrict src, size_t n);

PICOTM_NOTHROW
/**
 * Variant of memmove_tx() that operates on transactional memory.
 */
void*
memmove_tm(void* dest, const void* src, size_t n);

PICOTM_NOTHROW
/**
 * Variant of memset_tx() that operates on transactional memory.
 */
void*
memset_tm(void* s, int c, size_t n);

PICOTM_NOTHROW
/**
 * Variant of memrchr_tx() that operates on transactional memory.
 */
void*
memrchr_tm(const void* s, int c, size_t n);

PICOTM_NOTHROW
/**
 * Variant of rawmemchr_tx() that operates on transactional memory.
 */
void*
rawmemchr_tm(const void* s, int c);

PICOTM_NOTHROW
/**
 * Variant of stpcpy_tx() that operates on transactional memory.
 */
char*
stpcpy_tm(char* restrict dest, const char* restrict src);

PICOTM_NOTHROW
/**
 * Variant of stpncpy_tx() that operates on transactional memory.
 */
char*
stpncpy_tm(char* restrict dest, const char* restrict src, size_t n);

PICOTM_NOTHROW
/**
 * Variant of strcat_tx() that operates on transactional memory.
 */
char*
strcat_tm(char* restrict dest, const char* restrict src);

PICOTM_NOTHROW
/**
 * Variant of strchr_tx() that operates on transactional memory.
 */
char*
strchr_tm(const char* s, int c);

PICOTM_NOTHROW
/**
 * Variant of strcmp_tx() that operates on transactional memory.
 */
int
strcmp_tm(const char* s1, const char* s2);

PICOTM_NOTHROW
/**
 * Variant of strcoll_l_tx() that operates on transactional memory.
 */
int
strcoll_l_tm(const char* s1, const char* s2, locale_t locale);

PICOTM_NOTHROW
/**
 * Variant of strcpy_tx() that operates on transactional memory.
 */
char*
strcpy_tm(char* restrict dest, const char* restrict src);

PICOTM_NOTHROW
/**
 * Variant of strcspn_tx() that operates on transactional memory.
 */
size_t
strcspn_tm(const char* s, const char* reject);

PICOTM_NOTHROW
/**
 * Variant of strdup_tx() that operates on transactional memory.
 */
char*
strdup_tm(const char* s);

PICOTM_NOTHROW
/**
 * Variant of __strerror_r_gnu_tx() that operates on transactional memory.
 *
 * \warning This is an internal interface. Call strerror_r_tm() instead.
 */
char*
__strerror_r_gnu_tm(int errnum, char* buf, size_t buflen);

PICOTM_NOTHROW
/**
 * Variant of __strerror_r_posix_tx() that operates on transactional memory.
 *
 * \warning This is an internal interface. Call strerror_r_tm() instead.
 */
int
__strerror_r_posix_tm(int errnum, char* buf, size_t buflen);

#if (_POSIX_C_SOURCE >= 200112L) && !_GNU_SOURCE
#define strerror_r_tm   __strerror_r_posix_tm
#else
#define strerror_r_tm   __strerror_r_gnu_tm
#endif

PICOTM_NOTHROW
/**
 * Variant of strlen_tx() that operates on transactional memory.
 */
size_t
strlen_tm(const char* s);

PICOTM_NOTHROW
/**
 * Variant of strncat_tx() that operates on transactional memory.
 */
char*
strncat_tm(char* restrict dest, const char* restrict src, size_t n);

PICOTM_NOTHROW
/**
 * Variant of strncmp_tx() that operates on transactional memory.
 */
int
strncmp_tm(const char* s1, const char* s2, size_t n);

PICOTM_NOTHROW
/**
 * Variant of strncpy_tx() that operates on transactional memory.
 */
char*
strncpy_tm(char* restrict dest, const char* restrict src, size_t n);

PICOTM_NOTHROW
/**
 * Variant of strndup_tx() that operates on transactional memory.
 */
char*
strndup_tm(const char* s, size_t n);

PICOTM_NOTHROW
/**
 * Variant of strnlen_tx() that operates on transactional memory.
 */
size_t
strnlen_tm(const char* s, size_t maxlen);

PICOTM_NOTHROW
/**
 * Variant of strpbrk_tx() that operates on transactional memory.
 */
char*
strpbrk_tm(const char* s, const char* accept);

PICOTM_NOTHROW
/**
 * Variant of strrchr_tx() that operates on transactional memory.
 */
char*
strrchr_tm(const char* s, int c);

PICOTM_NOTHROW
/**
 * Variant of strspn_tx() that operates on transactional memory.
 */
size_t
strspn_tm(const char* s, const char* accept);

PICOTM_NOTHROW
/**
 * Variant of strstr_tx() that operates on transactional memory.
 */
char*
strstr_tm(const char* haystack, const char* needle);

PICOTM_NOTHROW
/**
 * Variant of strtok_r_tx() that operates on transactional memory.
 */
char*
strtok_r_tm(char* restrict str, const char* restrict delim,
            char** restrict saveptr);

PICOTM_END_DECLS
