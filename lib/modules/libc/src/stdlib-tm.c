/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/stdlib-tm.h"
#include <errno.h>
#include <malloc.h>
#include <picotm/picotm-module.h>
#include <picotm/picotm.h>
#include "alloc/comalloctx.h"
#include "fs/comfstx.h"
#include "picotm/picotm-libc.h"

PICOTM_EXPORT
void
free_tm(void* ptr)
{
    com_alloc_tx_free(ptr, malloc_usable_size(ptr));
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
        err = com_alloc_tx_posix_memalign(memptr, alignment, size);
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

    void* mem;

    size_t usiz = malloc_usable_size(ptr);

    do {
        mem = com_alloc_tx_realloc(ptr, size, usiz);
        if (size && !mem) {
            picotm_recover_from_errno(errno);
        }
    } while (size && !mem);

    return mem;
}

PICOTM_EXPORT
int
rand_r_tm(unsigned int* seed)
{
    return rand_r(seed);
}
