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

#include <time.h>

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

struct picotm_error;
struct timespec;

#if !defined(__MACH__) || defined(__PICOTM_DOXYGEN)
/**
 * CLOCK_REALTIME is the only clock supported by GNU libc's condition
 * variables. Something like CLOCK_BOOTTIME or CLOCK_MONOTONIC_RAW
 * would be ideal.
 */
static const clockid_t PICOTM_OS_TIMESPEC_CLOCKID = CLOCK_REALTIME;
#endif

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
 * \brief Compares two timespec values.
 * \param lhs Left-hand-side operator.
 * \param rhs Right-hand-side operator.
 * \returns Returns a value less than, equal to, or larger than 0 if the
 *          value of lhs is less than, equal to, or larger than the value
 *          of rhs.
 */
int
picotm_os_timespec_compare(const struct timespec* restrict lhs,
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
