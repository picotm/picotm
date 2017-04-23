/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdlib.h>

int
com_alloc_tx_posix_memalign(void **memptr, size_t alignment, size_t size);

void
com_alloc_tx_free(void* mem, size_t usiz);

void *
com_alloc_tx_calloc(size_t nelem, size_t elsize);

void *
com_alloc_tx_malloc(size_t siz);

void*
com_alloc_tx_realloc(void* ptr, size_t siz, size_t usiz);
