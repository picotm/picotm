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
