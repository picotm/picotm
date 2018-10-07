/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "picotm/picotm-lib-tab.h"
#include "table.h"

PICOTM_EXPORT
void*
picotm_tabresize(void* base, size_t nelems, size_t newnelems, size_t siz,
                 struct picotm_error* error)
{
    return tabresize(base, nelems, newnelems, siz, error);
}

PICOTM_EXPORT
void
picotm_tabfree(void* base)
{
    tabfree(base);
}

PICOTM_EXPORT
size_t
picotm_tabwalk_1(void* base, size_t nelems, size_t siz,
                 picotm_tabwalk_1_function walk, struct picotm_error* error)
{
    return tabwalk_1(base, nelems, siz, walk, error);
}

PICOTM_EXPORT
size_t
picotm_tabwalk_2(void* base, size_t nelems, size_t siz,
                 picotm_tabwalk_2_function walk, void* data,
                 struct picotm_error* error)
{
    return tabwalk_2(base, nelems, siz, walk, data, error);
}

PICOTM_EXPORT
size_t
picotm_tabwalk_3(void* base, size_t nelems, size_t siz,
                 picotm_tabwalk_3_function walk, void* data1, void* data2,
                 struct picotm_error* error)
{
    return tabwalk_3(base, nelems, siz, walk, data1, data2, error);
}

PICOTM_EXPORT
size_t
picotm_tabrwalk_1(void* base, size_t nelems, size_t siz,
                  picotm_tabwalk_1_function walk, struct picotm_error* error)
{
    return tabrwalk_1(base, nelems, siz, walk, error);
}

PICOTM_EXPORT
size_t
picotm_tabrwalk_2(void* base, size_t nelems, size_t siz,
                  picotm_tabwalk_2_function walk, void* data,
                  struct picotm_error* error)
{
    return tabrwalk_2(base, nelems, siz, walk, data, error);
}

PICOTM_EXPORT
size_t
picotm_tabuniq(void* base, size_t nelems, size_t siz,
              int (*compare)(const void*, const void*))
{
    return tabuniq(base, nelems, siz, compare);
}
