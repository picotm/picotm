/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <picotm/compiler.h>
#include <stdlib.h>

PICOTM_BEGIN_DECLS

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

PICOTM_EXPORT
/**
 * Variant of mkdtemp_tx() that operates on transactional memory.
 */
char*
mkdtemp_tm(char* template);

PICOTM_EXPORT
/**
 * Variant of mkstemp_tx() that operates on transactional memory.
 */
int
mkstemp_tm(char* template);

PICOTM_NOTHROW
/**
 * Variant of posix_memalign_tx() that operates on transactional memory.
 */
int
posix_memalign_tm(void** memptr, size_t alignment, size_t size);

PICOTM_NOTHROW
/**
 * Variant of qsort_tx() that operates on transactional memory.
 */
void
qsort_tm(void* base, size_t nel, size_t width,
         int (*compar)(const void*, const void*));

PICOTM_NOTHROW
/**
 * Variant of realloc_tx() that operates on transactional memory.
 */
void*
realloc_tm(void* ptr, size_t size);

PICOTM_NOTHROW
/**
 * Variant of rand_r_tx() that operates on transactional memory.
 */
int
rand_r_tm(unsigned int* seed);

PICOTM_END_DECLS
