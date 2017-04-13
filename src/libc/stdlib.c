/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systx/stdlib.h"
#include <systx/systx.h>
#include "alloc/comalloctx.h"
#include "systx/stdlib-tm.h"

SYSTX_EXPORT
void
_Exit_tx(int status)
{
    __systx_commit();
    _Exit(status);
}

SYSTX_EXPORT
void
abort_tx()
{
    __systx_commit();
    abort();
}

SYSTX_EXPORT
void*
calloc_tx(size_t nmemb, size_t size)
{
    return com_alloc_tx_calloc(nmemb, size);
}

SYSTX_EXPORT
void
exit_tx(int status)
{
    __systx_commit();
    exit(status);
}

SYSTX_EXPORT
void
free_tx(void* ptr)
{
    return com_alloc_tx_free(ptr);
}

SYSTX_EXPORT
void*
malloc_tx(size_t size)
{
    return com_alloc_tx_malloc(size);
}

SYSTX_EXPORT
char*
mkdtemp_tx(char* template)
{
    privatize_c_tx(template, '\0', SYSTX_TM_PRIVATIZE_LOADSTORE);
    return mkdtemp_tm(template);
}

SYSTX_EXPORT
int
mkstemp_tx(char* template)
{
    privatize_c_tx(template, '\0', SYSTX_TM_PRIVATIZE_LOADSTORE);
    return mkstemp_tm(template);
}

SYSTX_EXPORT
int
posix_memalign_tx(void** memptr, size_t alignment, size_t size)
{
    return com_alloc_tx_posix_memalign(memptr, alignment, size);
}

SYSTX_EXPORT
void*
realloc_tx(void* ptr, size_t size)
{
    return com_alloc_tx_realloc(ptr, size);
}

SYSTX_EXPORT
int
rand_r_tx(unsigned int* seed)
{
    privatize_tx(seed, sizeof(*seed), SYSTX_TM_PRIVATIZE_LOADSTORE);
    return rand_r_tm(seed);
}
