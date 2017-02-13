/* Copyright (C) 2009  Thomas Zimmermann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

ceuta_hdrl(#ifndef TANGER_STM_MALLOC_H);
ceuta_hdrl(#define TANGER_STM_MALLOC_H);
ceuta_hdrl(#include <malloc.h>);

#include <malloc.h> /* Non-standard */

ceuta_pure(size_t, malloc_usable_size, malloc_usable_size, void *ptr);

ceuta_hdrl(#endif);

