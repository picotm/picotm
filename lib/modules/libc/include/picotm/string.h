/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <picotm/compiler.h>
#include <string.h>

/*
 * Memory functions
 */

PICOTM_NOTHROW
/**
 * Executes memccpy() within a transaction.
 */
void*
memccpy_tx(void* restrict dest, const void* restrict src, int c, size_t n);

PICOTM_NOTHROW
/**
 * Executes memchr() within a transaction.
 */
void*
memchr_tx(const void* s, int c, size_t n);

PICOTM_NOTHROW
/**
 * Executes memcmp() within a transaction.
 */
int
memcmp_tx(const void* s1, const void* s2, size_t n);

PICOTM_NOTHROW
/**
 * Executes memcpy() within a transaction.
 */
void*
memcpy_tx(void* restrict dest, const void* restrict src, size_t n);

PICOTM_NOTHROW
/**
 * Executes memmove() within a transaction.
 */
void*
memmove_tx(void* dest, const void* src, size_t n);

PICOTM_NOTHROW
/**
 * Executes memset() within a transaction.
 */
void*
memset_tx(void* s, int c, size_t n);

PICOTM_NOTHROW
/**
 * Executes memrchr() within a transaction.
 */
void*
memrchr_tx(const void* s, int c, size_t n);

PICOTM_NOTHROW
/**
 * Executes rawmemchr() within a transaction.
 */
void*
rawmemchr_tx(const void* s, int c);

/*
 * String functions
 */

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of stpcpy().
 */
char*
stpcpy_tx(char* restrict dest, const char* restrict src);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of stpncpy().
 */
char*
stpncpy_tx(char* restrict dest, const char* restrict src, size_t n);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strcat().
 */
char*
strcat_tx(char* restrict dest, const char* restrict src);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strchr().
 */
char*
strchr_tx(const char* s, int c);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strcmp().
 */
int
strcmp_tx(const char* s1, const char* s2);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strcoll_l().
 */
int
strcoll_l_tx(const char* s1, const char* s2, locale_t locale);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strcpy().
 */
char*
strcpy_tx(char* restrict dest, const char* restrict src);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strcspn().
 */
size_t
strcspn_tx(const char* s, const char* reject);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strdup().
 */
char*
strdup_tx(const char* s);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strerror_r().
 */
int
strerror_r_tx(int errnum, char* buf, size_t buflen);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strlen().
 */
size_t
strlen_tx(const char* s);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strncat().
 */
char*
strncat_tx(char* restrict dest, const char* restrict src, size_t n);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strncmp().
 */
int
strncmp_tx(const char* s1, const char* s2, size_t n);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strncpy().
 */
char*
strncpy_tx(char* restrict dest, const char* restrict src, size_t n);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strndup().
 */
char*
strndup_tx(const char* s, size_t n);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strnlen().
 */
size_t
strnlen_tx(const char* s, size_t maxlen);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strpbrk().
 */
char*
strpbrk_tx(const char* s, const char* accept);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strrchr().
 */
char*
strrchr_tx(const char* s, int c);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strspn().
 */
size_t
strspn_tx(const char* s, const char* accept);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strstr().
 */
char*
strstr_tx(const char* haystack, const char* needle);

PICOTM_NOTHROW
/**
 * Provides a transaction-safe variant of strtok_r().
 */
char*
strtok_r_tx(char* restrict str, const char* restrict delim,
            char** restrict saveptr);
