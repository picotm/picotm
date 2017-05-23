/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

/**
 * \ingroup group_modules
 * \file
 */

#include <stddef.h>
#include "compiler.h"

PICOTM_BEGIN_DECLS

/**
 * Invoked by table functions when walking over the elements of a table.
 * \param   data0   The table element.
 * \returns The number of element to advance on success, or a negative value
 *          otherwise.
 */
typedef int (*picotm_tabwalk_1_function)(void* data0);

/**
 * Invoked by table functions when walking over the elements of a table.
 * \param   data0   The table element.
 * \param   data1   An additional argument.
 * \returns The number of element to advance on success, or a negative value
 *          otherwise.
 */
typedef int (*picotm_tabwalk_2_function)(void* data0, void* data1);

/**
 * Invoked by table functions when walking over the elements of a table.
 * \param   data0   The table element.
 * \param   data1   An additional argument.
 * \param   data2   An additional argument.
 * \returns The number of element to advance on success, or a negative value
 *          otherwise.
 */
typedef int (*picotm_tabwalk_3_function)(void* data0, void* data1,
                                         void* data2);

/**
 * Invoked by table functions for comparing two elements.
 * \param   data0   A table element.
 * \param   data1   A table element.
 * \returns -1, 0, or 1 if data0 is less, equal, or greatet than data1.
 */
typedef int (*picotm_tab_compare_function)(const void* data0,
                                           const void* data1);

PICOTM_NOTHROW
/**
 * Allocate or resize table.
 * \param   base        A pointer to the table's first element.
 * \param   nelems      The number of elements in the table.
 * \param   newnelems   The new number of elements in the table.
 * \param   siz         The number of bytes per element.
 * \returns A pointer to the resized table on success, or NULL otherwise.
 */
void*
picotm_tabresize(void* base, size_t nelems, size_t newnelems, size_t siz);

PICOTM_NOTHROW
/**
 * Free table memory.
 * \param   base    A pointer to the table's first element.
 */
void
picotm_tabfree(void* base);

PICOTM_NOTHROW
/**
 * Walk over table elements.
 * \param   base    A pointer to the table's first element.
 * \param   nelems  The number of elements in the table.
 * \param   siz     The number of bytes per element.
 * \param   walk    The call-back function.
 * \returns 0 on success, or a negative value otherwise.
 */
int
picotm_tabwalk_1(void* base, size_t nelems, size_t siz,
                 picotm_tabwalk_1_function walk);

PICOTM_NOTHROW
/**
 * Walk over table elements.
 * \param   base    A pointer to the table's first element.
 * \param   nelems  The number of elements in the table.
 * \param   siz     The number of bytes per element.
 * \param   walk    The call-back function.
 * \param   data    An additional second argument to the call-back function.
 * \returns 0 on success, or a negative value otherwise.
 */
int
picotm_tabwalk_2(void* base, size_t nelems, size_t siz,
                 picotm_tabwalk_2_function walk, void* data);

PICOTM_NOTHROW
/**
 * Walk over table elements.
 * \param   base    A pointer to the table's first element.
 * \param   nelems  The number of elements in the table.
 * \param   siz     The number of bytes per element.
 * \param   walk    The call-back function.
 * \param   data1   An additional second argument to the call-back function.
 * \param   data2   An additional third argument to the call-back function.
 * \returns 0 on success, or a negative value otherwise.
 */
int
picotm_tabwalk_3(void* base, size_t nelems, size_t siz,
                 picotm_tabwalk_3_function walk, void* data1, void* data2);

PICOTM_NOTHROW
/**
 * Walk over table in reversed order.
 * \param   base    A pointer to the table's first element.
 * \param   nelems  The number of elements in the table.
 * \param   siz     The number of bytes per element.
 * \param   walk    The call-back function.
 * \returns 0 on success, or a negative value otherwise.
 */
int
picotm_tabrwalk_1(void* base, size_t nelems, size_t siz,
                  picotm_tabwalk_1_function walk);

PICOTM_NOTHROW
/**
 * Walk over table in reversed order.
 * \param   base    A pointer to the table's first element.
 * \param   nelems  The number of elements in the table.
 * \param   siz     The number of bytes per element.
 * \param   walk    The call-back function.
 * \param   data    An additional second argument to the call-back function.
 * \returns 0 on success, or a negative value otherwise.
 */
int
picotm_tabrwalk_2(void* base, size_t nelems, size_t siz,
                  picotm_tabwalk_2_function walk, void* data);

PICOTM_NOTHROW
/**
 * Filters out duplicate elements
 * \param   base    A pointer to the table's first element.
 * \param   nelems  The number of elements in the table.
 * \param   siz     The number of bytes per element.
 * \param   compare The call-back function for comparing elements.
 * \returns The new number of elements in the table.
 */
size_t
picotm_tabuniq(void* base, size_t nelems, size_t siz,
               picotm_tab_compare_function compare);

PICOTM_END_DECLS
