/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <systx/systx.h>
#include "alloc/comalloctx.h"
#include "systx/stdlib.h"

/*
 * Memory management
 */

SYSTX_EXPORT
int
posix_memalign_tx(void** memptr, size_t alignment, size_t size)
{
    return com_alloc_tx_posix_memalign(memptr, alignment, size);
}

SYSTX_EXPORT
void
free_tx(void* ptr)
{
    return com_alloc_tx_free(ptr);
}

SYSTX_EXPORT
void*
calloc_tx(size_t nmemb, size_t size)
{
    return com_alloc_tx_calloc(nmemb, size);
}

SYSTX_EXPORT
void*
malloc_tx(size_t size)
{
    return com_alloc_tx_malloc(size);
}

SYSTX_EXPORT
void*
realloc_tx(void* ptr, size_t size)
{
    return com_alloc_tx_realloc(ptr, size);
}

/*
 * Programm termination
 */

SYSTX_EXPORT
void
abort_tx()
{
    systx_commit();
    abort();
}

SYSTX_EXPORT
void
exit_tx(int status)
{
    systx_commit();
    exit(status);
}

SYSTX_EXPORT
void
_Exit_tx(int status)
{
    systx_commit();
    _Exit(status);
}
