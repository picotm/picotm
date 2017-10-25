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
