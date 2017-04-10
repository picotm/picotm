/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdlib.h>
#include <systx/systx-tm.h>
#include "compiler.h"

SYSTX_TM_LOAD_TX(div_t, div_t);
SYSTX_TM_LOAD_TX(ldiv_t, ldiv_t);
SYSTX_TM_LOAD_TX(lldiv_t, lldiv_t);
/*SYSTX_TM_LOAD_TX(size_t, size_t);*/ /* defined in stddef.h */
/*SYSTX_TM_LOAD_TX(_t, wchar_t);*/ /* defined in stddef.h */

SYSTX_TM_STORE_TX(div_t, div_t);
SYSTX_TM_STORE_TX(ldiv_t, ldiv_t);
SYSTX_TM_STORE_TX(lldiv_t, lldiv_t);
/*SYSTX_TM_STORE_TX(size_t, size_t);*/ /* defined in stddef.h */
/*SYSTX_TM_LOAD_TX(_t, wchar_t);*/ /* defined in stddef.h */

SYSTX_NOTHROW SYSTX_NORETURN
void _Exit_tx(int status);

SYSTX_NOTHROW SYSTX_NORETURN
void abort_tx(void);

SYSTX_NOTHROW
/**
 * Executes calloc() within a transaction.
 */
void* calloc_tx(size_t nmemb, size_t size);

SYSTX_NOTHROW SYSTX_NORETURN
void exit_tx(int status);

SYSTX_NOTHROW
/**
 * Executes free() within a transaction.
 */
void free_tx(void* ptr);

SYSTX_NOTHROW
/**
 * Executes malloc() within a transaction.
 */
void* malloc_tx(size_t size);

SYSTX_EXPORT
/**
 * Executes mkdtemp() within a transaction.
 */
char*
mkdtemp_tx(char* template);

SYSTX_EXPORT
/**
 * Executes mkstemp() within a transaction.
 */
int
mkstemp_tx(char* template);

SYSTX_NOTHROW
/**
 * Executes posix_memalign() within a transaction.
 */
int posix_memalign_tx(void** memptr, size_t alignment, size_t size);

SYSTX_NOTHROW
/**
 * Executes realloc() within a transaction.
 */
void* realloc_tx(void* ptr, size_t size);

SYSTX_NOTHROW
/**
 * Executes rand_r() within a transaction.
 */
int
rand_r_tx(unsigned int* seed);
