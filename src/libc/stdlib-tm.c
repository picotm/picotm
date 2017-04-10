/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systx/stdlib-tm.h"
#include <systx/systx.h>
#include "alloc/comalloctx.h"

SYSTX_EXPORT
void
free_tm(void* ptr)
{
    return com_alloc_tx_free(ptr);
}

SYSTX_EXPORT
int
posix_memalign_tm(void** memptr, size_t alignment, size_t size)
{
    return com_alloc_tx_posix_memalign(memptr, alignment, size);
}

SYSTX_EXPORT
void*
realloc_tm(void* ptr, size_t size)
{
    return com_alloc_tx_realloc(ptr, size);
}
