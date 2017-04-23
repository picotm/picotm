/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <picotm/compiler.h>
#include <picotm/picotm-tm.h>
#include <stdlib.h>

PICOTM_TM_LOAD_TX(div_t, div_t);
PICOTM_TM_LOAD_TX(ldiv_t, ldiv_t);
PICOTM_TM_LOAD_TX(lldiv_t, lldiv_t);
/*PICOTM_TM_LOAD_TX(size_t, size_t);*/ /* defined in stddef.h */
/*PICOTM_TM_LOAD_TX(_t, wchar_t);*/ /* defined in stddef.h */

PICOTM_TM_STORE_TX(div_t, div_t);
PICOTM_TM_STORE_TX(ldiv_t, ldiv_t);
PICOTM_TM_STORE_TX(lldiv_t, lldiv_t);
/*PICOTM_TM_STORE_TX(size_t, size_t);*/ /* defined in stddef.h */
/*PICOTM_TM_LOAD_TX(_t, wchar_t);*/ /* defined in stddef.h */

PICOTM_NOTHROW PICOTM_NORETURN
void _Exit_tx(int status);

PICOTM_NOTHROW PICOTM_NORETURN
void abort_tx(void);

PICOTM_NOTHROW
/**
 * Executes calloc() within a transaction.
 */
void* calloc_tx(size_t nmemb, size_t size);

PICOTM_NOTHROW PICOTM_NORETURN
void exit_tx(int status);

PICOTM_NOTHROW
/**
 * Executes free() within a transaction.
 */
void free_tx(void* ptr);

PICOTM_NOTHROW
/**
 * Executes malloc() within a transaction.
 */
void* malloc_tx(size_t size);

PICOTM_EXPORT
/**
 * Executes mkdtemp() within a transaction.
 */
char*
mkdtemp_tx(char* template);

PICOTM_EXPORT
/**
 * Executes mkstemp() within a transaction.
 */
int
mkstemp_tx(char* template);

PICOTM_NOTHROW
/**
 * Executes posix_memalign() within a transaction.
 */
int posix_memalign_tx(void** memptr, size_t alignment, size_t size);

PICOTM_NOTHROW
/**
 * Executes qsort() within a transaction.
 */
void
qsort_tx(void* base, size_t nel, size_t width,
         int (*compar)(const void*, const void*));

PICOTM_NOTHROW
/**
 * Executes realloc() within a transaction.
 */
void* realloc_tx(void* ptr, size_t size);

PICOTM_NOTHROW
/**
 * Executes rand_r() within a transaction.
 */
int
rand_r_tx(unsigned int* seed);
