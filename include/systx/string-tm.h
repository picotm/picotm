/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <string.h>
#include "compiler.h"

/*
 * Memory functions
 */

SYSTX_NOTHROW
/**
 * Executes memccpy() within a transaction.
 */
void*
memccpy_tm(void* restrict dest, const void* restrict src, int c, size_t n);

SYSTX_NOTHROW
/**
 * Executes memchr() within a transaction.
 */
void*
memchr_tm(const void* s, int c, size_t n);

SYSTX_NOTHROW
/**
 * Executes memcmp() within a transaction.
 */
int
memcmp_tm(const void* s1, const void* s2, size_t n);

SYSTX_NOTHROW
/**
 * Executes memcpy() within a transaction.
 */
void*
memcpy_tm(void* restrict dest, const void* restrict src, size_t n);

SYSTX_NOTHROW
/**
 * Executes memmove() within a transaction.
 */
void*
memmove_tm(void* dest, const void* src, size_t n);

SYSTX_NOTHROW
/**
 * Executes memset() within a transaction.
 */
void*
memset_tm(void* s, int c, size_t n);

SYSTX_NOTHROW
/**
 * Executes memrchr() within a transaction.
 */
void*
memrchr_tm(const void* s, int c, size_t n);

SYSTX_NOTHROW
/**
 * Executes rawmemchr() within a transaction.
 */
void*
rawmemchr_tm(const void* s, int c);

/*
 * String functions
 */

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of stpcpy().
 */
char*
stpcpy_tm(char* restrict dest, const char* restrict src);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of stpncpy().
 */
char*
stpncpy_tm(char* restrict dest, const char* restrict src, size_t n);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strcat().
 */
char*
strcat_tm(char* restrict dest, const char* restrict src);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strchr().
 */
char*
strchr_tm(const char* s, int c);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strcmp().
 */
int
strcmp_tm(const char* s1, const char* s2);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strcoll_l().
 */
int
strcoll_l_tm(const char* s1, const char* s2, locale_t locale);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strcpy().
 */
char*
strcpy_tm(char* restrict dest, const char* restrict src);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strcspn().
 */
size_t
strcspn_tm(const char* s, const char* reject);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strdup().
 */
char*
strdup_tm(const char* s);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strerror_r().
 */
int
strerror_r_tm(int errnum, char* buf, size_t buflen);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strlen().
 */
size_t
strlen_tm(const char* s);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strncat().
 */
char*
strncat_tm(char* restrict dest, const char* restrict src, size_t n);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strncmp().
 */
int
strncmp_tm(const char* s1, const char* s2, size_t n);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strncpy().
 */
char*
strncpy_tm(char* restrict dest, const char* restrict src, size_t n);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strndup().
 */
char*
strndup_tm(const char* s, size_t n);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strnlen().
 */
size_t
strnlen_tm(const char* s, size_t maxlen);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strpbrk().
 */
char*
strpbrk_tm(const char* s, const char* accept);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strrchr().
 */
char*
strrchr_tm(const char* s, int c);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strspn().
 */
size_t
strspn_tm(const char* s, const char* accept);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strstr().
 */
char*
strstr_tm(const char* haystack, const char* needle);

SYSTX_NOTHROW
/**
 * Provides a transaction-safe variant of strtok_r().
 */
char*
strtok_r_tm(char* restrict str, const char* restrict delim,
            char** restrict saveptr);
