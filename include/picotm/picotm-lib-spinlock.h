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
