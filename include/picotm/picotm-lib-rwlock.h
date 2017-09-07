/* Permission is hereby granted, free of charge, to any person obtaining a
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
 */

#pragma once

#include <stdatomic.h>
#include <stdbool.h>
#include "compiler.h"

/**
 * \ingroup group_modules
 * \file
 */

PICOTM_BEGIN_DECLS

struct picotm_error;

/**
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
    /** counter variable; UINT8_T means 'exclusive writer present' */
    atomic_uint_least8_t  n;
};

PICOTM_NOTHROW
/**
 * Initializes a reader-writer lock.
 *
 * \param self  The reader-writer lock to initialize.
 */
void
picotm_rwlock_init(struct picotm_rwlock* self);

PICOTM_NOTHROW
/**
 * Uninitializes a reader-writer lock.
 *
 * \param self  The reader-writer lock to uninitialize.
 */
void
picotm_rwlock_uninit(struct picotm_rwlock* self);

PICOTM_NOTHROW
/**
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
 * Releases a reader-writer lock.
 *
 * \param   self    The reader-writer lock to release.
 */
void
picotm_rwlock_unlock(struct picotm_rwlock* self);

PICOTM_END_DECLS
