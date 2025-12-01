/*
 * picotm - A system-level transaction manager
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

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

struct picotm_error;

/**
 * \brief The mutex lock of the operating system.
 *
 * A mutex provides an exclusive lock. The data structure
 * `struct picotm_os_mutex` is a wrapper around the mutex data
 * structure povided by the operating system. Implementations
 * of this structure shall be able to cooperate with instances
 * of `struct picotm_os_cond`.
 */
struct picotm_os_mutex {
    pthread_mutex_t instance;
};

/**
 * \brief Initializes a mutex instance.
 * \param self The mutex instance to initialize.
 * \param[out] error Returns an error to the caller.
 */
void
picotm_os_mutex_init(struct picotm_os_mutex* self,
                     struct picotm_error* error);

/**
 * \brief Cleans-up a mutex instance.
 * \param self The mutex instance to initialize.
 */
void
picotm_os_mutex_uninit(struct picotm_os_mutex* self);

/**
 * \brief Acquires a mutex lock.
 * \param self The mutex to acquire.
 * \param[out] error Returns an error to the caller.
 */
void
picotm_os_mutex_lock(struct picotm_os_mutex* self,
                     struct picotm_error* error);

/**
 * \brief Releases a mutex lock.
 * \param self The mutex to release.
 */
void
picotm_os_mutex_unlock(struct picotm_os_mutex* self);
