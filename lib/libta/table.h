/* Copyright (C) 2008  Thomas Zimmermann
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

