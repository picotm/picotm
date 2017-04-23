/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/stdlib-tm.h"
#include <malloc.h>
#include <picotm/picotm.h>
#include "alloc/comalloctx.h"
#include "fs/comfstx.h"

PICOTM_EXPORT
void
free_tm(void* ptr)
{
    return com_alloc_tx_free(ptr, malloc_usable_size(ptr));
}

PICOTM_EXPORT
char*
mkdtemp_tm(char* template)
{
    char* res = mkdtemp(template);
    if (!res) {
        return NULL;
    }
    return res;
}

PICOTM_EXPORT
int
mkstemp_tm(char* template)
{
    return com_fs_tx_mkstemp(template);
}

PICOTM_EXPORT
int
posix_memalign_tm(void** memptr, size_t alignment, size_t size)
{
    return com_alloc_tx_posix_memalign(memptr, alignment, size);
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
    return com_alloc_tx_realloc(ptr, size, malloc_usable_size(ptr));
}

PICOTM_EXPORT
int
rand_r_tm(unsigned int* seed)
{
    return rand_r(seed);
}
