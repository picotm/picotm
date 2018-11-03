/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "picotm/stdlib-tm.h"
#include "picotm/picotm-module.h"
#include "picotm/picotm.h"
#include <errno.h>
/* We test and include <malloc_np.h> first, because FreeBSD provides
 * <malloc_np.h>, but fails with an error if <malloc.h> is included.
 */
#if defined(HAVE_MALLOC_NP_H) && HAVE_MALLOC_NP_H
#include <malloc_np.h>
#elif defined (HAVE_MALLOC_H) && HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <string.h>
#include "allocator/module.h"
#include "compat/malloc_usable_size.h"
#include "error/module.h"
#include "fildes/module.h"

#if defined(PICOTM_LIBC_HAVE_FREE) && PICOTM_LIBC_HAVE_FREE
PICOTM_EXPORT
void
free_tm(void* ptr)
{
    allocator_module_free(ptr, malloc_usable_size(ptr));
}
#endif

#if defined(PICOTM_LIBC_HAVE_MKDTEMP) && PICOTM_LIBC_HAVE_MKDTEMP && \
    !defined(__MACH__)
PICOTM_EXPORT
char*
mkdtemp_tm(char* template)
{
    error_module_save_errno();

    char* str;

    do {
        str = mkdtemp(template);
        if (!str) {
            picotm_recover_from_errno(errno);
        }
    } while (!str);

    return str;
}
#endif

#if defined(PICOTM_LIBC_HAVE_MKSTEMP) && PICOTM_LIBC_HAVE_MKSTEMP
PICOTM_EXPORT
int
mkstemp_tm(char* template)
{
    error_module_save_errno();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        int res = fildes_module_mkstemp(template, &error);
        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);
    } while (true);
}
#endif

#if defined(PICOTM_LIBC_HAVE_POSIX_MEMALIGN) && \
        PICOTM_LIBC_HAVE_POSIX_MEMALIGN
PICOTM_EXPORT
int
posix_memalign_tm(void** memptr, size_t alignment, size_t size)
{
    allocator_module_posix_memalign(memptr, alignment, size);
    return 0;
}
#endif

#if defined(PICOTM_LIBC_HAVE_QSORT) && PICOTM_LIBC_HAVE_QSORT
PICOTM_EXPORT
void
qsort_tm(void* base, size_t nel, size_t width,
         int (*compar)(const void*, const void*))
{
    qsort(base, nel, width, compar);
}
#endif

#if defined(PICOTM_LIBC_HAVE_RAND_R) && PICOTM_LIBC_HAVE_RAND_R
PICOTM_EXPORT
int
rand_r_tm(unsigned int* seed)
{
    return rand_r(seed);
}
#endif

#if defined(PICOTM_LIBC_HAVE_REALLOC) && PICOTM_LIBC_HAVE_REALLOC
PICOTM_EXPORT
void*
realloc_tm(void* ptr, size_t size)
{
    error_module_save_errno();

    size_t usiz = malloc_usable_size(ptr);

    void* mem = NULL;

    if (size) {
        mem = allocator_module_malloc(size);
    }

    if (ptr && mem) {
        /* Valgrind might report invalid reads and out-of-bounds access
         * within this function. This is a false positive. The result of
         * malloc_usable_size() is the maximum available buffer space,
         * not the amount of allocated or valid memory. Any memcpy() could
         * therefore operate on uninitialized data.
         */
        memcpy(mem, ptr, size < usiz ? size : usiz);
    }

    if (ptr) {
        allocator_module_free(ptr, usiz);
    }

    return mem;
}
#endif
