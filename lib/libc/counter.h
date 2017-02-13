/* Copyright (C) 2008-2009  Thomas Zimmermann
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

#ifndef COUNTER_H
#define COUNTER_H

/**
 * \brief Counter with locking
 */
struct counter
{
    pthread_spinlock_t lock;
    count_type         val;
};

int
counter_init(struct counter *c);

void
counter_uninit(struct counter *c);

void
counter_reset(struct counter *c);

count_type
counter_inc(struct counter *c);

count_type
counter_dec(struct counter *c);

count_type
counter_set(struct counter *c, count_type val);

count_type
counter_get(struct counter *c);

count_type
counter_add(struct counter *c, count_type val);

int
counter_equal_val(struct counter *c, count_type val);

int
counter_init_walk(void *c);

int
counter_uninit_walk(void *c);

int
counter_set_walk(void *c, void *val);

int
counter_equal_val_walk(void *c, void *val);

#endif

