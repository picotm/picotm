/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/stdlib.h"
#include <errno.h>
#include <malloc.h>
#include <picotm/picotm.h>
#include <picotm/picotm-module.h>
#include "alloc/comalloctx.h"
#include "picotm/picotm-libc.h"
#include "picotm/stdlib-tm.h"

PICOTM_EXPORT
void
_Exit_tx(int status)
{
    __picotm_commit();
    _Exit(status);
}

PICOTM_EXPORT
void
abort_tx()
{
    __picotm_commit();
    abort();
}

PICOTM_EXPORT
void*
calloc_tx(size_t nmemb, size_t size)
{
    picotm_libc_save_errno();

    void* mem;

    size_t alloc_size = nmemb * size;

    do {
        mem = com_alloc_tx_calloc(nmemb, size);
        if (alloc_size && !mem) {
            picotm_recover_from_errno(errno);
        }
    } while (alloc_size && ! mem);

    return mem;
}

PICOTM_EXPORT
void
exit_tx(int status)
{
    __picotm_commit();
    exit(status);
}

PICOTM_EXPORT
void
free_tx(void* ptr)
{
    size_t usiz = malloc_usable_size(ptr);
    if (usiz) {
        privatize_tx(ptr, usiz, PICOTM_TM_PRIVATIZE_LOADSTORE);
    }
    com_alloc_tx_free(ptr, usiz);
}

PICOTM_EXPORT
void*
malloc_tx(size_t size)
{
    picotm_libc_save_errno();

    void* mem;

    do {
        mem = com_alloc_tx_malloc(size);
        if (size && !mem) {
            picotm_recover_from_errno(errno);
        }
    } while (size && !mem);

    return mem;
}

PICOTM_EXPORT
char*
mkdtemp_tx(char* template)
{
    privatize_c_tx(template, '\0', PICOTM_TM_PRIVATIZE_LOADSTORE);
    return mkdtemp_tm(template);
}

PICOTM_EXPORT
int
mkstemp_tx(char* template)
{
    privatize_c_tx(template, '\0', PICOTM_TM_PRIVATIZE_LOADSTORE);
    return mkstemp_tm(template);
}

PICOTM_EXPORT
int
posix_memalign_tx(void** memptr, size_t alignment, size_t size)
{
    privatize_tx(*memptr, sizeof(*memptr), PICOTM_TM_PRIVATIZE_STORE);
    return posix_memalign_tm(memptr, alignment, size);
}

PICOTM_EXPORT
void
qsort_tx(void* base, size_t nel, size_t width,
         int (*compar)(const void*, const void*))
{
    privatize_tx(base, nel * width, PICOTM_TM_PRIVATIZE_LOADSTORE);
    qsort_tm(base, nel, width, compar);
}

PICOTM_EXPORT
void*
realloc_tx(void* ptr, size_t size)
{
    size_t usiz = malloc_usable_size(ptr);
    if (usiz) {
        privatize_tx(ptr, usiz, PICOTM_TM_PRIVATIZE_LOADSTORE);
    }

    picotm_libc_save_errno();

    void* mem;

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
rand_r_tx(unsigned int* seed)
{
    privatize_tx(seed, sizeof(*seed), PICOTM_TM_PRIVATIZE_LOADSTORE);
    return rand_r_tm(seed);
}
