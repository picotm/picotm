/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

ceuta_hdrl(#ifndef TANGER_STM_MALLOC_H);
ceuta_hdrl(#define TANGER_STM_MALLOC_H);
ceuta_hdrl(#include <malloc.h>);

#include <malloc.h> /* Non-standard */

ceuta_pure(size_t, malloc_usable_size, malloc_usable_size, void *ptr);

ceuta_hdrl(#endif);

