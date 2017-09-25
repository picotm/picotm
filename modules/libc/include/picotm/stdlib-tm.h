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