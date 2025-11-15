/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann
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

#include <pthread.h>
#include <stdbool.h>

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

struct picotm_error;
struct picotm_os_mutex;
struct timespec;

/**
 * \brief The condition variable of the operating system.
 *
 * The data structure `struct picotm_os_cond` represents a condition
 * variable provides by the operating system. A condition variable waits
 * for a signal while atomically releasing and acquiring a mutex lock
 * during the process. Waiting operations are allowed to fail spuriously.
 * An implementation of `struct picotm_os_cond` shall be able to cooperate
 * with `struct picotm_os_mutex`.
 */
struct picotm_os_cond {
    pthread_cond_t instance;
};

/**
 * \brief Initializes a condition variable.
 * \param self The condition variable to initialize.
 * \param[out] error Returns an error to the caller.
 */
void
picotm_os_cond_init(struct picotm_os_cond* self, struct picotm_error* error);

/**
 * \brief Cleans-up a condition variable.
 * \param self The condition variable to clean up.
 */
void
picotm_os_cond_uninit(struct picotm_os_cond* self);

/**
 * \brief Waits for a wake-up signal on a condition variable.
 * \param self The condition variable to wait at.
 * \param mutex The mutex to release and acquire during the waiting.
 * \param[out] error Returns an error to the caller.
 *
 * The mutex has to be locked by the caller before calling this function.
 */
void
picotm_os_cond_wait(struct picotm_os_cond* self,
                    struct picotm_os_mutex* mutex,
                    struct picotm_error* error);

/**
 * \brief Waits for a wake-up signal on a condition variable until a
 *        timeout expires.
 * \param self The condition variable to wait at.
 * \param mutex The mutex to release and acquire during the waiting.
 * \param timeout The timeout for waiting.
 * \param[out] error Returns an error to the caller.
 * \returns True if the waiter wa swoken up by a signal, false if the
 *          timeout has expired.
 *
 * The mutex has to be locked by the caller before calling this function.
 */
bool
picotm_os_cond_wait_until(struct picotm_os_cond* self,
                          struct picotm_os_mutex* mutex,
                          const struct timespec* timeout,
                          struct picotm_error* error);

/**
 * \brief Wakes up a single waiter of a condition variable.
 * \param self The condition variable to wait at.
 * \param[out] error Returns an error to the caller.
 */
void
picotm_os_cond_wake_up(struct picotm_os_cond* self,
                       struct picotm_error* error);

/**
 * \brief Wakes up all waiters of a condition variable.
 * \param self The condition variable to wait at.
 * \param[out] error Returns an error to the caller.
 */
void
picotm_os_cond_wake_up_all(struct picotm_os_cond* self,
                           struct picotm_error* error);
