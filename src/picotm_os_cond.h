/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
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
 *
 * SPDX-License-Identifier: MIT
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
