/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stddef.h>

struct picotm_error;

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

/**
 * Allocate or resize table.
 */
void*
tabresize(void* restrict base, size_t nelems, size_t newnelems, size_t siz,
          struct picotm_error* error);

/**
 * Free table memory.
 */
void
tabfree(void* restrict base);

/* Walk over table elements
 */

size_t
tabwalk_1(void* restrict base, size_t nelems, size_t siz,
          size_t (*walk)(void*, struct picotm_error*),
          struct picotm_error* error);

size_t
tabwalk_2(void* restrict base, size_t nelems, size_t siz,
          size_t (*walk)(void*, void*, struct picotm_error*), void* data,
          struct picotm_error* error);

size_t
tabwalk_3(void* restrict base, size_t nelems, size_t siz,
          size_t (*walk)(void*, void*, void*, struct picotm_error*),
          void* data1, void* data2,  struct picotm_error* error);

/* Walk over table in reversed order
 */

size_t
tabrwalk_1(void* restrict base, size_t nelems, size_t siz,
           size_t (*walk)(void*, struct picotm_error*),
           struct picotm_error* error);

size_t
tabrwalk_2(void* restrict base, size_t nelems, size_t siz,
           size_t (*walk)(void*, void*, struct picotm_error*), void* data,
           struct picotm_error* error);

/**
 * Filter out duplicate elements.
 * \return New length
 */
size_t
tabuniq(void* restrict base, size_t nelems, size_t siz,
        int (*compare)(const void*, const void*));
