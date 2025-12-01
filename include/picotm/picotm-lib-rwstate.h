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

#include "compiler.h"

/**
 * \ingroup group_lib
 * \file
 *
 * \brief Contains `struct picotm_rwstate` and helpers.
 *
 * The data structure `struct picotm_rwstate` represents a transaction's
 * status wrt. shared state.
 *
 * For example, shared state might be an integer with an associated
 * reader/writer lock.
 *
 * ~~~ c
 *      int i;
 *      struct picotm_rwlock i_lock;
 *
 *      i = 0;
 *      picotm_rwlock_init(&i_lock);
 * ~~~
 *
 * Each transaction accessing the integer `i` contains a reader/writer
 * state value of type `struct picotm_rwstate`. The transaction has to
 * initialize the state value with a call to `picotm_rwstate_init()`.
 *
 * ~~~ c
 *      struct picotm_rwstate i_state;
 *
 *      picotm_rwstate_init(&i_state);
 * ~~~
 *
 * Alternatively, the macro `PICOTM_RWSTATE_INITIALIZER` initializes a
 * static or stack-allocated R/W-state variable. If both, function and
 * macro initialization, is possible, the macro form is prefered.
 *
 * ~~~ c
 *      struct picotm_rwstate i_state = PICOTM_RWSTATE_INITIALIZER;
 * ~~~
 *
 * To safely read the value of resource `i`, a transaction has to acquire
 * the reader lock `i_lock`.
 *
 * ~~~ c
 *      struct picotm_error error = PICOTM_ERROR_INITIALIZER;
 *
 *      picotm_rwstate_try_rdlock(&i_state, &i_lock, &error);
 *      if (picotm_error_is_set(&error)) {
 *          // perform recovery
 *      }
 *
 *      // read i
 * ~~~
 *
 * A call to `picotm_rwstate_try_rdlock()` acquires a reader lock and sets
 * the transaction's reader/writer state accordingly. If the reader lock
 * could not be acquired, an error is returned. In this case, the transaction
 * should initiate a recovery.
 *
 * To safely write, a transaction has to acquire the writer lock `i_lock`.
 *
 * ~~~ c
 *      picotm_rwstate_try_wrlock(&i_state, &i_lock, &error);
 *      if (picotm_error_is_set(&error)) {
 *          // perform recovery
 *      }
 *
 *      // write i
 * ~~~
 *
 * Depending on the transaction's current reader/writer state, the function
 * `picotm_rwstate_try_wrlock()` automatically updates a reader lock to a
 * writer lock.
 *
 * Reader/writer locks can only be acquired once by each transaction, and
 * possibly upgraded once by each transaction. The reader/writer state
 * variable keeps track of the transaction's lock state, so locks can be
 * acquired and upgraded multiple times.
 *
 * ~~~ c
 *      picotm_rwstate_try_wrlock(&i_state, &i_lock, &error);
 *      if (picotm_error_is_set(&error)) {
 *          // perform recovery
 *      }
 *      picotm_rwstate_try_rdlock(&i_state, &i_lock, &error);
 *      if (picotm_error_is_set(&error)) {
 *          // perform recovery
 *      }
 *      picotm_rwstate_try_wrlock(&i_state, &i_lock, &error);
 *      if (picotm_error_is_set(&error)) {
 *          // perform recovery
 *      }
 * ~~~
 *
 * In any case, a reader-writer lock can only be unlocked *once,* using
 * `picotm_rwstate_unlock()`.
 * ~~~ c
 *      picotm_rwstate_unlock(&i_state, &i_lock);
 * ~~~
 *
 * The current reader/writer state can be queried using
 * `picotm_rwstate_get_status()`, which returns one of
 * `enum picotm_rwstate_status`. Using `picotm_rwstate_set_status()`, the
 * status can also be set explicitly. This might be useful when adopting
 * pre-acquired reader/writer locks, but it's generally preferable to use
 * reader/writer state interfaces for locking and unlocking.
 *
 * Finally, to uninitialize a reader/writer state the transaction has to
 * call `picotm_rwstate_uninit()`.
 *
 * ~~~ c
 *      picotm_rwstate_uninit(&i_state);
 * ~~~
 */

PICOTM_BEGIN_DECLS

struct picotm_error;
struct picotm_rwlock;

/**
 * \ingroup group_lib
 * Signals a shared state's status wrt. a transaction
 */
enum picotm_rwstate_status {
    /** The transaction has not acquired the lock. */
    PICOTM_RWSTATE_UNLOCKED = 0,
    /** The transaction has acquired a reader lock. */
    PICOTM_RWSTATE_RDLOCKED,
    /** The transaction has acquired a writer lock. */
    PICOTM_RWSTATE_WRLOCKED
};

/**
 * \ingroup group_lib
 * Represents shared state that can be read or written by a transaction.
 */
struct picotm_rwstate {
    /**
     * The shared state's status wrt. the current transaction.
     * \warning This is a private field. Don't access it in module code.
     */
    unsigned char status;
};

/**
 * \ingroup group_lib
 * \brief Initializer macro for R/W state variables.
 */
#define PICOTM_RWSTATE_INITIALIZER  \
    {                               \
        0                           \
    }

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Initializes a reader-writer state.
 *
 * \param self  The reader-writer state to initialize.
 */
void
picotm_rwstate_init(struct picotm_rwstate* self);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Uninitializes a reader-writer state.
 *
 * \param self  The reader-writer state to uninitialize.
 */
void
picotm_rwstate_uninit(struct picotm_rwstate* self);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Sets a reader/writer state's status.
 *
 * \param   self    The reader/writer state.
 * \param   status  The status to set.
 * \returns The reader/writer state's status.
 */
void
picotm_rwstate_set_status(struct picotm_rwstate* self,
                          enum picotm_rwstate_status status);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Returns a reader/writer state's current status.
 *
 * \param   self    The reader/writer state.
 * \returns The reader/writer state's status.
 */
enum picotm_rwstate_status
picotm_rwstate_get_status(const struct picotm_rwstate* self);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Tries to acquire a reader lock for the state variable. If the lock
 * could not be acquired, the error parameter will return a conflict.
 *
 * \param       self    The reader/writer state.
 * \param       rwlock  The reader lock to acquire.
 * \param[out]  error   Returns a error.
 */
void
picotm_rwstate_try_rdlock(struct picotm_rwstate* self,
                          struct picotm_rwlock* rwlock,
                          struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Tries to acquire a writer lock for the state variable or upgrade
 * an acquired reader lock to a writer lock. If the lock could not be
 * acquired, the error parameter will return a conflict.
 *
 * \param       self    The reader/writer state.
 * \param       rwlock  The reader lock to acquire.
 * \param[out]  error   Returns a error.
 */
void
picotm_rwstate_try_wrlock(struct picotm_rwstate* self,
                          struct picotm_rwlock* rwlock,
                          struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Releases a reader-writer lock.
 *
 * \param   self    The reader/writer state.
 * \param   rwlock  The reader/writer lock to release.
 */
void
picotm_rwstate_unlock(struct picotm_rwstate* self,
                      struct picotm_rwlock* rwlock);

PICOTM_END_DECLS
