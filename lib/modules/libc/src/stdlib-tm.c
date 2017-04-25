/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/stdlib-tm.h"
#include <errno.h>
#include <malloc.h>
#include <picotm/picotm-module.h>
#include <picotm/picotm.h>
#include <string.h>
#include "allocator/module.h"
#include "fs/comfstx.h"
#include "picotm/picotm-libc.h"

PICOTM_EXPORT
void
free_tm(void* ptr)
{
    allocator_module_free(ptr, malloc_usable_size(ptr));
}

PICOTM_EXPORT
char*
mkdtemp_tm(char* template)
{
    picotm_libc_save_errno();

    char* str;

    do {
        str = mkdtemp(template);
        if (!str) {
            picotm_recover_from_errno(errno);
        }
    } while (!str);

    return str;
}

PICOTM_EXPORT
int
mkstemp_tm(char* template)
{
    picotm_libc_save_errno();

    int res;

    do {
        res = com_fs_tx_mkstemp(template);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
posix_memalign_tm(void** memptr, size_t alignment, size_t size)
{
    int err;

    do {
        err = allocator_module_posix_memalign(memptr, alignment, size);
        if (err) {
            picotm_recover_from_errno(err);
        }
    } while (err);

    return err;
}

PICOTM_EXPORT
void
qsort_tm(void* base, size_t nel, size_t width,
         int (*compar)(const void*, const void*))
{
    qsort(base, nel, width, compar);
}

PICOTM_EXPORT
void*
realloc_tm(void* ptr, size_t size)
{
    picotm_libc_save_errno();

    size_t usiz = malloc_usable_size(ptr);

    void* mem = NULL;

    if (size) {
        int err;
        do {
            err = allocator_module_posix_memalign(&mem, 2 * sizeof(void*),
                                                  size);
            if (err) {
                picotm_recover_from_errno(err);
            }
        } while (err);
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

    if (ptr && !size) {
        allocator_module_free(ptr, usiz);
    }

    return mem;
}

PICOTM_EXPORT
int
rand_r_tm(unsigned int* seed)
{
    return rand_r(seed);
}
