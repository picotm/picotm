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

#include <time.h>

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

struct picotm_error;
struct timespec;

/**
 * CLOCK_REALTIME is the only clock supported by GNU libc's
 * condition variables. Something like CLOCK_BOOTTIME or
 * CLOCK_MONOTONIC_RAW would be ideal instead.
 */
static const clock_t PICOTM_OS_TIMESPEC_CLOCKID = CLOCK_REALTIME;

/**
 * \brief Retrieves the current time from the system.
 * \param[out] self The retrieved time.
 * \param[out] error Returns an error to the caller.
 *
 * The time returned by this function shall be compatible with the
 * implementation of `struct picotm_os_cond`. The actual value, granularity
 * and start time depends on the operating system and can vary among
 * implementations.
 */
void
picotm_os_get_timespec(struct timespec* self,
                       struct picotm_error* error);

/**
 * \brief Adds two timespec values in place.
 * \param[in,out] lhs Left-hand-side operator and result.
 * \param rhs Right-hand-side operator.
 */
void
picotm_os_add_timespec(struct timespec* restrict lhs,
                 const struct timespec* restrict rhs);

/**
 * \brief Subtracts a timespec value from another in place.
 * \param[in,out] lhs Left-hand-side operator and result.
 * \param rhs Right-hand-side operator.
 */
void
picotm_os_sub_timespec(struct timespec* restrict lhs,
                 const struct timespec* restrict rhs);

/**
 * \brief Suspends the thread for a short period of time.
 * \param sleep_time The time to sleep.
 * \param[out] error Returns an error to the caller.
 *
 * This function provides a *low-overhead* way of suspending the calling
 * thread for a short period of time. Implementations may not busy wait
 * if possible. Depending on the system, threads may sleep longer than
 * the specified period of time.
 */
void
picotm_os_nanosleep(const struct timespec* sleep_time,
                    struct picotm_error* error);
