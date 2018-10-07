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

#include <pthread.h>

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

struct picotm_error;

/**
 * \brief The reader/writer lock of the operating system.
 *
 * The data structure `struct picotm_os_rwlock` is a wrapper around
 * the reader/writer lock provided by the operating system. R/W locks
 * can be acquired multiple concurrent readers or a single writer. An
 * implementation of `struct picotm_os_rwlock` shall provide support
 * for multiple concurrent readers if the operating system supports it.
 */
struct picotm_os_rwlock {
    pthread_rwlock_t instance;
};

/**
 * \brief Initializes an R/W-lock instance.
 * \param self The R/W-lock instance to initialize.
 * \param[out] error Returns an error to the caller.
 */
void
picotm_os_rwlock_init(struct picotm_os_rwlock* self,
                      struct picotm_error* error);

/**
 * \brief Cleans-up an R/W-lock instance.
 * \param self The R/W-lock instance to clean up.
 */
void
picotm_os_rwlock_uninit(struct picotm_os_rwlock* self);

/**
 * \brief Acquires a reader lock.
 * \param self The R/W-lock instance to acquire.
 * \param[out] error Returns an error to the caller.
 */
void
picotm_os_rwlock_rdlock(struct picotm_os_rwlock* self,
                        struct picotm_error* error);

/**
 * \brief Acquires a writer lock.
 * \param self The R/W-lock instance to acquire.
 * \param[out] error Returns an error to the caller.
 */
void
picotm_os_rwlock_wrlock(struct picotm_os_rwlock* self,
                        struct picotm_error* error);

/**
 * \brief Releases an R/W lock.
 * \param self The R/W-lock instance to release.
 */
void
picotm_os_rwlock_unlock(struct picotm_os_rwlock* self);
