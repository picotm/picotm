/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TABLE_H
#define TABLE_H

/**
 * \brief Allocate or resize table
 */
void *
tabresize(void * restrict base, size_t nelems, size_t newnelems, size_t siz);

/* Walk over table elements
 */

int
tabwalk_1(void * restrict base, size_t nelems, size_t siz, int (*walk)(void*));

int
tabwalk_2(void * restrict base, size_t nelems,
                                size_t siz, int (*walk)(void*, void*), void *data);

int
tabwalk_3(void * restrict base, size_t nelems,
                                size_t siz,
                                int (*walk)(void*, void*, void*), void *data1,
                                                                  void *data2);

/* Walk over table in reversed order
 */

int
tabrwalk_1(void * restrict base, size_t nelems,
                                 size_t siz, int (*walk)(void*));

int
tabrwalk_2(void * restrict base, size_t nelems,
                                 size_t siz,
                                 int (*walk)(void*, void*), void *data);

/**
 * \brief Filter out duplicate elements
 * \return New length
 */
size_t
tabuniq(void * restrict base, size_t nelems,
                              size_t siz,
                              int (*compare)(const void*, const void*));

#endif

