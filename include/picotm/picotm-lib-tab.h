/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

/**
 * \ingroup group_lib
 * \file
 */

#include <stddef.h>
#include "compiler.h"

PICOTM_BEGIN_DECLS

struct picotm_error;

/**
 * \ingroup group_lib
 * Invoked by table functions when walking over the elements of a table.
 * \param       data0   The table element.
 * \param[out]  error   Returns an error from the module.
 * \returns The number of element to advance.
 */
typedef size_t (*picotm_tabwalk_1_function)(void* data0,
                                            struct picotm_error* error);

/**
 * \ingroup group_lib
 * Invoked by table functions when walking over the elements of a table.
 * \param       data0   The table element.
 * \param       data1   An additional argument.
 * \param[out]  error   Returns an error from the module.
 * \returns The number of element to advance.
 */
typedef size_t (*picotm_tabwalk_2_function)(void* data0, void* data1,
                                            struct picotm_error* error);

/**
 * \ingroup group_lib
 * Invoked by table functions when walking over the elements of a table.
 * \param       data0   The table element.
 * \param       data1   An additional argument.
 * \param       data2   An additional argument.
 * \param[out]  error   Returns an error from the module.
 * \returns The number of element to advance.
 */
typedef size_t (*picotm_tabwalk_3_function)(void* data0, void* data1,
                                            void* data2,
                                            struct picotm_error* error);

/**
 * \ingroup group_lib
 * Invoked by table functions for comparing two elements.
 * \param   data0   A table element.
 * \param   data1   A table element.
 * \returns A value less, equal, or greater to zero if data0 is less, equal,
 *          or greatet than data1.
 */
typedef int (*picotm_tab_compare_function)(const void* data0,
                                           const void* data1);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Allocate or resize table.
 * \param       base        A pointer to the table's first element.
 * \param       nelems      The number of elements in the table.
 * \param       newnelems   The new number of elements in the table.
 * \param       siz         The number of bytes per element.
 * \param[out]  error       Returns an error from the module.
 * \returns A pointer to the resized table on success, or NULL otherwise.
 */
void*
picotm_tabresize(void* base, size_t nelems, size_t newnelems, size_t siz,
                 struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Free table memory.
 * \param   base    A pointer to the table's first element.
 */
void
picotm_tabfree(void* base);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Walk over table elements.
 * \param       base    A pointer to the table's first element.
 * \param       nelems  The number of elements in the table.
 * \param       siz     The number of bytes per element.
 * \param       walk    The call-back function.
 * \param[out]  error   Returns an error from the module.
 * \returns The number of walked elements.
 */
size_t
picotm_tabwalk_1(void* base, size_t nelems, size_t siz,
                 picotm_tabwalk_1_function walk,
                 struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Walk over table elements.
 * \param       base    A pointer to the table's first element.
 * \param       nelems  The number of elements in the table.
 * \param       siz     The number of bytes per element.
 * \param       walk    The call-back function.
 * \param       data    An additional second argument to the call-back function.
 * \param[out]  error   Returns an error from the module.
 * \returns The number of walked elements.
 */
size_t
picotm_tabwalk_2(void* base, size_t nelems, size_t siz,
                 picotm_tabwalk_2_function walk, void* data,
                 struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Walk over table elements.
 * \param       base    A pointer to the table's first element.
 * \param       nelems  The number of elements in the table.
 * \param       siz     The number of bytes per element.
 * \param       walk    The call-back function.
 * \param       data1   An additional second argument to the call-back function.
 * \param       data2   An additional third argument to the call-back function.
 * \param[out]  error   Returns an error from the module.
 * \returns The number of walked elements.
 */
size_t
picotm_tabwalk_3(void* base, size_t nelems, size_t siz,
                 picotm_tabwalk_3_function walk, void* data1, void* data2,
                 struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Walk over table in reversed order.
 * \param       base    A pointer to the table's first element.
 * \param       nelems  The number of elements in the table.
 * \param       siz     The number of bytes per element.
 * \param       walk    The call-back function.
 * \param[out]  error   Returns an error from the module.
 * \returns The number of walked elements.
 */
size_t
picotm_tabrwalk_1(void* base, size_t nelems, size_t siz,
                  picotm_tabwalk_1_function walk,
                  struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Walk over table in reversed order.
 * \param       base    A pointer to the table's first element.
 * \param       nelems  The number of elements in the table.
 * \param       siz     The number of bytes per element.
 * \param       walk    The call-back function.
 * \param       data    An additional second argument to the call-back function.
 * \param[out]  error   Returns an error from the module.
 * \returns The number of walked elements.
 */
size_t
picotm_tabrwalk_2(void* base, size_t nelems, size_t siz,
                  picotm_tabwalk_2_function walk, void* data,
                  struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Filters out duplicate elements
 * \param       base    A pointer to the table's first element.
 * \param       nelems  The number of elements in the table.
 * \param       siz     The number of bytes per element.
 * \param       compare The call-back function for comparing elements.
 * \returns The new number of elements in the table.
 */
size_t
picotm_tabuniq(void* base, size_t nelems, size_t siz,
               picotm_tab_compare_function compare);

PICOTM_END_DECLS
