/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/picotm-lib-tab.h"
#include "table.h"

PICOTM_EXPORT
void*
picotm_tabresize(void* base, size_t nelems, size_t newnelems, size_t siz)
{
    return tabresize(base, nelems, newnelems, siz);
}

PICOTM_EXPORT
void
picotm_tabfree(void* base)
{
    tabfree(base);
}

PICOTM_EXPORT
int
picotm_tabwalk_1(void* base, size_t nelems, size_t siz, int (*walk)(void*))
{
    return tabwalk_1(base, nelems, siz, walk);
}

PICOTM_EXPORT
int
picotm_tabwalk_2(void* base, size_t nelems, size_t siz,
                int (*walk)(void*, void*), void* data)
{
    return tabwalk_2(base, nelems, siz, walk, data);
}

PICOTM_EXPORT
int
picotm_tabwalk_3(void* base, size_t nelems, size_t siz,
                int (*walk)(void*, void*, void*), void* data1, void* data2)
{
    return tabwalk_3(base, nelems, siz, walk, data1, data2);
}

PICOTM_EXPORT
int
picotm_tabrwalk_1(void* base, size_t nelems, size_t siz, int (*walk)(void*))
{
    return tabrwalk_1(base, nelems, siz, walk);
}

PICOTM_EXPORT
int
picotm_tabrwalk_2(void* base, size_t nelems, size_t siz,
                 int (*walk)(void*, void*), void* data)
{
    return tabrwalk_2(base, nelems, siz, walk, data);
}

PICOTM_EXPORT
size_t
picotm_tabuniq(void* base, size_t nelems, size_t siz,
              int (*compare)(const void*, const void*))
{
    return tabuniq(base, nelems, siz, compare);
}

