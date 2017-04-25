/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "counter.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "spinlock.h"

static void
counter_lock(struct counter *c)
{
    assert(c);

    spinlock_lock(&c->lock);
}

static void
counter_unlock(struct counter *c)
{
    assert(c);

    spinlock_unlock(&c->lock);
}

int
counter_init(struct counter *c)
{
    int err;

    assert(c);

    if ((err = spinlock_init(&c->lock, PTHREAD_PROCESS_PRIVATE)) < 0) {
        return err;
    }

    c->val = 0;

    return 0;
}

void
counter_uninit(struct counter *c)
{
    assert(c);

    spinlock_uninit(&c->lock);
}

void
counter_reset(struct counter *c)
{
    counter_set(c, 0);
}

count_type
counter_inc(struct counter *c)
{
    return counter_add(c, 1);
}

count_type
counter_dec(struct counter *c)
{
    return counter_add(c, -1);
}

count_type
counter_set(struct counter *c, count_type val)
{
    assert(c);

    counter_lock(c);
    c->val = val;
    counter_unlock(c);

    return val;
}

count_type
counter_get(struct counter *c)
{
    assert(c);

    counter_lock(c);
    count_type val = c->val;
    counter_unlock(c);

    return val;
}

count_type
counter_add(struct counter *c, count_type val)
{
    assert(c);

    counter_lock(c);
    count_type nval = c->val += val;
    counter_unlock(c);

    return nval;
}

int
counter_equal_val(struct counter *c, count_type val)
{
    assert(c);

    counter_lock(c);

    int eq = c->val == val;

    counter_unlock(c);

    return eq;
}

int
counter_init_walk(void *c)
{
    return !counter_init(c) ? 1 : -1;
}

int
counter_uninit_walk(void *c)
{
    counter_uninit(c);

    return 1;
}

int
counter_set_walk(void *c, void *val)
{
    assert(val);

    counter_set(c, *(count_type*)val);

    return 1;
}

int
counter_equal_val_walk(void *c, void *val)
{
    assert(val);

    return counter_equal_val(c, *(count_type*)val) <= 0 ? -1 : 1;
}

