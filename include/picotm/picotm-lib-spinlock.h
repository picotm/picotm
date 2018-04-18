/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/**
 * \ingroup group_lib
 * \file
 * \brief Contains `struct picotm_spinlock` and helper functions.
 *
 * The `struct picotm_spinlock` data structure provides an spin lock that is
 * independent from the operating system. It's useful for portable modules or
 * creating more complex data structures.
 *
 * The spin lock uses a very simple implementation. It's non-recursive, so
 * a thread cannot acquire a spin lock twice at the same time. It's also not
 * dead-lock safe. The advantage of the simple implmentation is that it
 * cannot fail with an error.
 *
 * An instance of `struct picotm_spinlock` is initialized by a call
 * to `picotm_spinlock_init()`.
 *
 * ~~~ c
 *      struct picotm_spinlock lock;
 *
 *      spinlock_init(&lock);
 * ~~~
 *
 * Alternatively, the macro `PICOTM_SPINLOCK_INITIALIZER` initializes
 * static lock variables, or stack-allocated locks. When both, function
 * and macro initialization, is possible, the macro form os the prefered
 * one.
 *
 * ~~~ c
 *      struct picotm_spinlock lock = PICOTM_SPINLOCK_INITIALIZER;
 * ~~~
 *
 * After the initialization, the spinlock is in an unlocked state. A call
 * to `picotm_spinlock_lock()` acquires the spin lock, and a call to
 * `picotm_spinlock_unlock()` release the spin lock.
 *
 * ~~~ c
 *      picotm_spinlock_lock(&lock);
 *
 *          // critical section
 *
 *      picotm_spinlock_unlock(&lock);
 * ~~~
 *
 * A call to `picotm_spinlock_lock()` can possibly block the thread for an
 * unbounded amount of time. A call to `picotm_spinlock_try_lock()` only
 * tries once to acquire the lock, and returns a boolean value signalling
 * success or failure.
 *
 * ~~~ c
 *      if (picotm_spinlock_try_lock(&lock)) {
 *
 *              // critical section
 *
 *          picotm_spinlock_unlock(&lock);
 *      }
 * ~~~
 *
 * The spin lock's locking and unlocking functions act as memory barriers.
 * Therefore load and store operations before, within, or after a critical
 * section take effect ontheir side of the respective barrier.
 */

#include <assert.h>
#include <stdatomic.h>
#include "compiler.h"

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_lib
 * \brief Provides an operating-system-neutral, non-recursive spin-lock type.
 */
struct picotm_spinlock {
    atomic_char is_locked;
};

/**
 * \ingroup group_lib
 * \brief Initializer macro for picotm spin locks.
 */
#define PICOTM_SPINLOCK_INITIALIZER \
    {                               \
        0                           \
    }

static inline void
picotm_spinlock_init(struct picotm_spinlock* self)
{
    assert(self);

    self->is_locked = 0;
}

static inline void
picotm_spinlock_uninit(struct picotm_spinlock* self)
{
    assert(self);

    /* nothing to do */
}

static inline _Bool
picotm_spinlock_try_lock(struct picotm_spinlock* self)
{
    assert(self);

    char expected = 0;

    return atomic_compare_exchange_strong_explicit(&self->is_locked,
                                                   &expected, 1,
                                                   memory_order_acq_rel,
                                                   memory_order_acquire);
}

static inline void
picotm_spinlock_lock(struct picotm_spinlock* self)
{
    assert(self);

    _Bool succ;

    do {
        char expected = 0;

        /* The 'weak compare-exchange' might be faster on some platforms. */
        succ = atomic_compare_exchange_weak_explicit(&self->is_locked,
                                                     &expected, 1,
                                                     memory_order_acq_rel,
                                                     memory_order_acquire);
    } while (!succ);
}

static inline void
picotm_spinlock_unlock(struct picotm_spinlock* self)
{
    assert(self);

    atomic_store_explicit(&self->is_locked, 0, memory_order_release);
}

PICOTM_END_DECLS
