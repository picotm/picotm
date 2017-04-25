/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdlib.h>

void
com_alloc_tx_free(void* mem, size_t usiz);

int
com_alloc_tx_posix_memalign(void **memptr, size_t alignment, size_t size);
