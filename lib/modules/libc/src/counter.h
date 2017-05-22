/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COUNTER_H
#define COUNTER_H

#include <pthread.h>

/**
 * \cond impl || libc_impl
 * \ingroup libc_impl
 * \file
 * \endcond
 */

/** \brief Counter type */
typedef long long count_type;

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

