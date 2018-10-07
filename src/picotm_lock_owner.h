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

#pragma once

#include <stdbool.h>
#include <time.h>
#include "picotm_os_cond.h"
#include "picotm_os_mutex.h"

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

struct picotm_error;

/**
 * \brief Lock-owner state flags.
 */
enum picotm_picotm_lock_owner_flags {
    /** \brief Lock owner is waiting */
    LOCK_OWNER_WT = 1ul << 29,
    /** \brief Lock owner is waiting to acquire a reader lock */
    LOCK_OWNER_RD = 1ul << 30,
    /** \brief Lock owner is waiting to acquire a writer lock*/
    LOCK_OWNER_WR = 1ul << 31
};

/**
 * \brief Represents the potential owner of a lock.
 */
struct picotm_lock_owner {

    /**
     * \brief Encodes information about the owner of a lock.
     *
     * | 31 | 30 | 29 | 19    ...     10 | 9 .. 0 |
     * | WR | RD | WT |    < empty >     | index  |
     */
    unsigned long flags;

    /** Entry in waiter list */
    struct picotm_lock_owner* next;

    /** Start time of the lock owner's transaction*/
    struct timespec timestamp;

    struct picotm_os_cond  wait_cond;
    struct picotm_os_mutex mutex;
};

/**
 * \brief Initializes a lock-owner instance.
 * \param self The lock owner to initialize.
 * \param[out] error Returns an error to the caller.
 */
void
picotm_lock_owner_init(struct picotm_lock_owner* self,
                       struct picotm_error* error);

/**
 * \brief Cleans up a lock-owner instance.
 * \param self The lock owner to clean up.
 */
void
picotm_lock_owner_uninit(struct picotm_lock_owner* self);

/**
 * \brief Sets the index of the next list element after a lock owner.
 * \param self The lock owner.
 * \param index The index of the next list element after the lock owner.
 */
void
picotm_lock_owner_set_index(struct picotm_lock_owner* self,
                            unsigned long index);

/**
 * \brief Returns a lock owner's index.
 * \param self The lock owner.
 * \returns The lock owner's index.
 */
unsigned long
picotm_lock_owner_get_index(const struct picotm_lock_owner* self);

/**
 * \brief Sets the index of the next list element after a lock owner.
 * \param self The lock owner.
 * \param next The index of the next list element after the lock owner.
 */
void
picotm_lock_owner_set_next(struct picotm_lock_owner* self,
                           unsigned long next);

/**
 * \brief Returns the index of the next list element after a lock owner.
 * \param self The lock owner.
 * \returns The index of the next list element after the lock owner.
 */
unsigned long
picotm_lock_owner_get_next(const struct picotm_lock_owner* self);

/**
 * \brief Resets a lock owner's timestamp.
 * \param self The lock owner.
 * \param[out] error Returns an error to the caller.
 */
void
picotm_lock_owner_reset_timestamp(struct picotm_lock_owner* self,
                                  struct picotm_error* error);

/**
 * \brief Returns a lock owner's timestamp.
 * \param self The lock owner.
 * \returns The timestamp of the lock owner.
 */
const struct timespec*
picotm_lock_owner_get_timestamp(const struct picotm_lock_owner* self);

/**
 * \brief Acquires an exclusive lock on a lock owner.
 * \param self The lock owner.
 * \param[out] error Returns an error to the caller.
 *
 * After successfully acquiring a lock on a lock owner, the caller is
 * responsible to release the lock afterwards with a call to
 * `picotm_lock_owner_unlock()`.
 */
void
picotm_lock_owner_lock(struct picotm_lock_owner* self,
                       struct picotm_error* error);

/**
 * \brief Releases a lock on a lock owner.
 * \param self The lock owner.
 *
 * Calls to this function are only allowed after acquiring a lock
 * with `picotm_lock_owner_lock()`.
 */
void
picotm_lock_owner_unlock(struct picotm_lock_owner* self);

/**
 * \brief Waits for a lock owner to be woken up, or until a timeout expires.
 * \param self The lock owner.
 * \param timeout The wake-up timeout.
 * \param[out] error Returns an error to the caller.
 * \returns True if the lock owner was woken up, or false if the timeout
 *          expired.
 *
 * This function possibly sleeps and blocks the calling thread. Sleeping for
 * and unbounded amount of time is not allowed, so the timeout must contain
 * some absolute point of time in the future.
 *
 * Callers of this function are required to successfully acquire the lock
 * on the lock owner via `picotm_lock_owner_lock()` *before* calling this
 * function, and release the lock afterwards via `picotm_lock_owner_unlock()`.
 * While the thread is sleeping, the lock will be unlocked.
 *
 * Threads should probably only wait for their own transaction's lock-owner
 * instance. Waiting for another thread's lock owner can result in undefined
 * behavior.
 */
bool
picotm_lock_owner_wait_until(struct picotm_lock_owner* self,
                             const struct timespec* timeout,
                             struct picotm_error* error);

/**
 * \brief Wakes up a thread waiting for a lock owner.
 * \param self The lock owner.
 * \param[out] error Returns an error to the caller.
 */
void
picotm_lock_owner_wake_up(struct picotm_lock_owner* self,
                          struct picotm_error* error);

/*
 * Look-up functions.
 *
 * These functions are implemented by the thread-local state handling.
 */

/**
 * \brief Returns the local thread's lock-owner structure.
 * \returns The thread-local lock owner.
 */
struct picotm_lock_owner*
picotm_lock_owner_get_thread_local_instance(void);
