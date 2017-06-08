/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
