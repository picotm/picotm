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

#include <stdatomic.h>
#include <stdbool.h>
#include "compiler.h"

/**
 * \ingroup group_lib
 * \file
 */

PICOTM_BEGIN_DECLS

struct picotm_error;

/**
 * \ingroup group_lib
 * \brief A reader-writer lock.
 *
 * The `struct picotm_rwlock` data structure implements a reader-writer
 * lock. Each instance can only be acquired at most once by each transaction.
 * The implementation does not track which transactions acquired the lock, so
 * transactions have to ensure that they acquire each lock at most once.
 *
 * Before using an instance of `struct picotm_rwlock`, initialize as show
 * below.
 *
 * ~~~ c
 *      struct picotm_rwlock rwlock;
 *
 *      picotm_rwlock_init(&rwlock);
 * ~~~
 *
 * The function `picotm_rwlock_init()` cannot fail. All fields of the
 * structure are private.
 *
 * Alternatively, the macro `PICOTM_RWLOCK_INITIALIZER` initializes a
 * static or stack-allocated R/W-lock variable. If both, function and
 * macro initialization, is possible, the macro form is prefered.
 *
 * ~~~ c
 *      struct picotm_rwlock rwlock = PICOTM_RWLOCK_INITIALIZER;
 * ~~~
 *
 * Likewise, uninitialize with `picotm_rwlock_uninit()` as shown below.
 *
 * ~~~ c
 *      picotm_rwlock_uninit(&rwlock);
 * ~~~
 *
 * The function `picotm_rwlock_try_rdlock()` acquires a reader lock. With
 * too many readers, the function fails with a conflict. You should always
 * check the error parameter and perform recovery if necessary.
 *
 * ~~~ c
 *      struct picotm_error error;
 *
 *      picotm_rwlock_try_rdlock(&rwlock, &error);
 *      if (picotm_error_is_set(&error)) {
 *          // probably busy; start recovery
 *      }
 * ~~~
 *
 * The function `picotm_rwlock_try_wrlock()` acquires a writer lock, or
 * upgrades a transaction's reader lock to a writer lock. With concurrent
 * users, the function fails with a conflict. A reader lock can be updated
 * to a writer lock at most once.
 *
 * As with `picotm_rwlock_try_rdlock()`, you should always check the error
 * parameter and perform recovery if necessary.
 *
 * ~~~ c
 *      // Upgrade to writer lock
 *      picotm_rwlock_try_wrlock(&rwlock, true, &error);
 *      if (picotm_error_is_set(&error)) {
 *          // concurrent users; start recovery
 *      }
 * ~~~
 */
struct picotm_rwlock {
    /** counter variable */
    atomic_uint_least8_t  n;
};

/**
 * \ingroup group_lib
 * \brief Initializer macro for R/W state variables.
 */
#define PICOTM_RWLOCK_INITIALIZER   \
    {                               \
        0                           \
    }

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Initializes a reader-writer lock.
 *
 * \param self  The reader-writer lock to initialize.
 */
void
picotm_rwlock_init(struct picotm_rwlock* self);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Uninitializes a reader-writer lock.
 *
 * \param self  The reader-writer lock to uninitialize.
 */
void
picotm_rwlock_uninit(struct picotm_rwlock* self);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Tries to acquire a read lock. If the lock could not be acquired, the
 * error parameter will return a conflict.
 *
 * \param       self    The reader lock to acquire.
 * \param[out]  error   Returns a error.
 */
void
picotm_rwlock_try_rdlock(struct picotm_rwlock* self,
                         struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Tries to acquire a writer lock or upgrade an acquired reader lock to
 * a writer lock. If the lock could not be acquired, the error parameter
 * will return a conflict.
 *
 * \param       self    The writer lock to acquire.
 * \param       upgrade True to upgrade a previously acquired reader lock.
 * \param[out]  error   Returns a error.
 */
void
picotm_rwlock_try_wrlock(struct picotm_rwlock* self, bool upgrade,
                         struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Releases a reader-writer lock.
 *
 * \param   self    The reader-writer lock to release.
 */
void
picotm_rwlock_unlock(struct picotm_rwlock* self);

PICOTM_END_DECLS
