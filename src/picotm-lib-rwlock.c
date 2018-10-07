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

#include "picotm-lib-rwlock.h"
#include <assert.h>
#include <stdint.h>
#include "picotm/picotm-error.h"
#include "picotm/picotm-module.h"
#include "picotm_lock_manager.h"
#include "picotm_lock_owner.h"
#include "picotm_os_timespec.h"

enum {
    /** \brief Offset of the waiter index in 'struct picotm_rwlock::n' */
    INDEX_BIT_SHIFT = 4,
    /** \brief Bitmask of the counter in 'struct picotm_rwlock::n' */
    COUNTER_BIT_MASK = (1ul << INDEX_BIT_SHIFT) - 1
};

/** \brief Bitmask of the waiter index in 'struct picotm_rwlock::n' */
static const uint8_t INDEX_BIT_MASK = ~COUNTER_BIT_MASK;

/** \brief Writer is present if this counter value is set. */
static const uint8_t WRITER_COUNTER = COUNTER_BIT_MASK;

PICOTM_EXPORT
void
picotm_rwlock_init(struct picotm_rwlock* self)
{
    assert(self);

    atomic_init(&self->n, 0);
}

PICOTM_EXPORT
void
picotm_rwlock_uninit(struct picotm_rwlock* self)
{
    assert(self);
}

static unsigned long
get_first_index(void* slist)
{
    struct picotm_rwlock* rwlock = slist;
    assert(rwlock);

    return picotm_rwlock_get_index(rwlock);
}

static unsigned long
cmpxchg_first_index(void* slist,
                    unsigned long expected_index,
                    unsigned long desired_index)
{
    struct picotm_rwlock* rwlock = slist;
    assert(rwlock);

    return picotm_rwlock_cmpxchg_index(rwlock, expected_index, desired_index);
}

static const struct picotm_lock_slist_funcs s_picotm_rwlock_slist_funcs = {
    get_first_index,
    cmpxchg_first_index
};

static uint8_t
rw_counter_bits(unsigned long value)
{
    assert((value & COUNTER_BIT_MASK) == value);

    return value & COUNTER_BIT_MASK;
}

static unsigned long
rw_counter(uint8_t n)
{
    return n & COUNTER_BIT_MASK;
}

static uint8_t
rw_index_bits(unsigned long value)
{
    return (value << INDEX_BIT_SHIFT) & INDEX_BIT_MASK;
}

static uint8_t
rw_index(uint8_t n)
{
    return (n & INDEX_BIT_MASK) >> INDEX_BIT_SHIFT;
}

static void
try_lock_or_wait(struct picotm_rwlock* self,
                 bool (*try_lock)(struct picotm_rwlock*),
                 struct picotm_error* error)
{
    static const struct timespec sleep_time = {0, 50};

    assert(try_lock);

    struct picotm_lock_owner* waiter = NULL;

    unsigned int nretries = 0;

    do {

        bool succ = try_lock(self);

        if (succ) {
            /* We successfully acquired the lock. */
            return;
        } else if (waiter) {
            /* Neither an error nor success, but we already waited
             * for the lock to become available. This time we signal
             * a conflict to the caller. */
            picotm_error_set_conflicting(error, self);
            return;
        } else {

            if (nretries) {
                picotm_os_nanosleep(&sleep_time, error);
                if (picotm_error_is_set(error)) {
                    return;
                }
                --nretries;
                continue;
            }

            /* Neither an error nor success as the lock is currently
             * blocked. We wait until the lock becomes available or
             * a timeout fires. */

            assert(!waiter); /* We haven't waited already. */

            waiter = picotm_lock_owner_get_thread_local_instance();

            struct picotm_lock_manager* lmanager =
                picotm_lock_owner_get_lock_manager(waiter);

            picotm_lock_manager_wait(lmanager, waiter, false,
                                     &s_picotm_rwlock_slist_funcs, self,
                                     error);

            if (picotm_error_is_set(error)) {
                return;
            }
        }
    } while (true);
}

static bool
try_rdlock(struct picotm_rwlock* self)
{
    uint8_t n = atomic_load_explicit(&self->n, memory_order_acquire);

    do {
        if (rw_counter(n) == WRITER_COUNTER) {
            /* writer present; cannot read-lock */
            return false;
        } else if (rw_counter(n) == (WRITER_COUNTER - 1)) {
            /* maximum number of readers reached; cannot read-lock */
            return false;
        } else if (rw_index(n)) {
            /* other transactions are already waiting; cannot read-lock */
            return false;
        }

        uint8_t expected = rw_counter_bits(rw_counter(n));
        uint8_t desired  = rw_counter_bits(rw_counter(n) + 1);

        bool succ = atomic_compare_exchange_strong_explicit(&self->n,
                                                            &expected, desired,
                                                            memory_order_acq_rel,
                                                            memory_order_acquire);
        if (succ) {
            return true;
        }

        n = expected;

    } while (true);
}

PICOTM_EXPORT
void
picotm_rwlock_try_rdlock(struct picotm_rwlock* self,
                         struct picotm_error* error)
{
    try_lock_or_wait(self, try_rdlock, error);
}

static bool
try_wrlock(struct picotm_rwlock* self)
{
    assert(self);

    uint8_t n = atomic_load_explicit(&self->n, memory_order_acquire);

    do {

        if (rw_counter(n)) {
            /* other transactions present; cannot write-lock */
            return false;
        } else if (rw_index(n)) {
            /* other waiters are already present; cannot write-lock */
            return false;
        }

        /* Expect us to be the only transaction. */
        uint8_t expected = rw_counter_bits(0);
        uint8_t desired  = rw_counter_bits(WRITER_COUNTER);

        bool succ = atomic_compare_exchange_strong_explicit(&self->n,
                                                            &expected, desired,
                                                            memory_order_acq_rel,
                                                            memory_order_acquire);
        if (succ) {
            return true;
        }

        n = expected;

    } while (true);
}

static bool
try_uplock(struct picotm_rwlock* self)
{
    assert(self);

    uint8_t n = atomic_load_explicit(&self->n, memory_order_acquire);

    do {

        assert(rw_counter(n));

        if (rw_counter(n) != 1) {
            /* other transactions present; cannot write-lock */
            return false;
        }

        /* Expect us to be the only reader if we're upgrading. Waiting
         * transactions can be ignored as we already hold the lock in
         * reader mode. */

        uint8_t expected = rw_index_bits(rw_index(n)) |
                           rw_counter_bits(1);

        uint8_t desired = rw_index_bits(rw_index(n)) |
                          rw_counter_bits(WRITER_COUNTER);

        bool succ =
            atomic_compare_exchange_strong_explicit(&self->n,
                                                    &expected,
                                                    desired,
                                                    memory_order_acq_rel,
                                                    memory_order_acquire);
        if (succ) {
            return true;
        } else if (rw_counter(n) == rw_counter(expected)) {
            /* We ignore the waiters when upgrading a lock. So if we
             * failed, but the counters remained the same, then only
             * the waiter field changed. We retry with the new waiter. */
            n = expected;
            continue;
        } else {
            return false;
        }
    } while (true);
}

PICOTM_EXPORT
void
picotm_rwlock_try_wrlock(struct picotm_rwlock* self, bool upgrade,
                         struct picotm_error* error)
{
    static bool (* const try_lock[])(struct picotm_rwlock*) = {
        try_wrlock,
        try_uplock
    };

    try_lock_or_wait(self, try_lock[upgrade], error);
}

PICOTM_EXPORT
void
picotm_rwlock_unlock(struct picotm_rwlock* self)
{
    assert(self);

    uint8_t n = atomic_load_explicit(&self->n, memory_order_acquire);

    assert(rw_counter(n) != 0);

    if (rw_counter(n) == WRITER_COUNTER) {

        /* We're a writer; set counter to zero. */
        //atomic_store_explicit(&self->n, 0, memory_order_release);
        assert(rw_counter(n) == WRITER_COUNTER);
        atomic_fetch_sub_explicit(&self->n, WRITER_COUNTER, memory_order_acq_rel);

    } else {

        /* We're a reader; decrement counter. */
        assert(rw_counter(n) != WRITER_COUNTER);
        atomic_fetch_sub_explicit(&self->n, 1, memory_order_acq_rel);
    }

    if (rw_index(n)) {

        /* We've released the lock at this point. Waiters can now
         * be woken up. */

        struct picotm_lock_owner* waiter =
            picotm_lock_owner_get_thread_local_instance();

        struct picotm_lock_manager* lm =
            picotm_lock_owner_get_lock_manager(waiter);

        do {
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_lock_manager_wake_up(lm, true,
                                        &s_picotm_rwlock_slist_funcs, self,
                                        &error);
            if (picotm_error_is_set(&error)) {
                picotm_error_mark_as_non_recoverable(&error);
                picotm_recover_from_error(&error);
                continue;
            }
            break;
        } while (true);
    }
}

unsigned long
picotm_rwlock_set_index(struct picotm_rwlock* self,
                        unsigned long desired_index)
{
    unsigned long expected_index = picotm_rwlock_get_index(self);

    do {
        unsigned long current_index =
            picotm_rwlock_cmpxchg_index(self, expected_index, desired_index);

        if (current_index == expected_index) {
            return current_index;
        }

        expected_index = current_index;

    } while (true);
}

unsigned long
picotm_rwlock_get_index(struct picotm_rwlock* self)
{
    assert(self);

    uint8_t n = atomic_load_explicit(&self->n, memory_order_acquire);

    return rw_index(n);
}

unsigned long
picotm_rwlock_cmpxchg_index(struct picotm_rwlock* self,
                            unsigned long expected_index,
                            unsigned long desired_index)
{
    assert(self);

    uint8_t n = atomic_load_explicit(&self->n, memory_order_acquire);

    do {
        /* Test and ... */

        uint8_t current_index = rw_index(n);
        if (current_index != expected_index) {
            return current_index;
        }

        /* ... Test-And-Set. */

        uint8_t expected = rw_counter_bits(rw_counter(n)) |
                           rw_index_bits(expected_index);

        uint8_t desired = rw_counter_bits(rw_counter(n)) |
                          rw_index_bits(desired_index);

        bool succ =
            atomic_compare_exchange_strong_explicit(&self->n,
                                                    &expected,
                                                    desired,
                                                    memory_order_acq_rel,
                                                    memory_order_acquire);
        if (succ) {

            /* Success */
            return rw_index(expected);

        } else if (rw_index(expected) != rw_index(n)) {

            /* 'Expected' now contains the then-current waiter
             * index. We return it for the caller to compare. */
            return rw_index(expected);

        } else {

            /* Only the counter changed meanwhile; we try again
             * re-using the value of 'expected'. */
            n = expected;
            continue;
        }
    } while (true);
}
