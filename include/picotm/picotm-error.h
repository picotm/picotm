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

/**
 * \ingroup group_modules
 * \file
 *
 * \brief Contains struct picotm_error and helper functions.
 */

#include "picotm/config/picotm-config.h"
#if defined(__MACH__)
#include <mach/mach.h>
#endif
#if defined(PICOTM_HAVE_SIGNAL_H) && PICOTM_HAVE_SIGNAL_H
#include <signal.h>
#endif
#include <stdbool.h>
#include <stddef.h>
#include "compiler.h"
#include "picotm-error-base.h"

PICOTM_BEGIN_DECLS

struct picotm_rwlock;

/**
 * Describes an error.
 */
struct picotm_error {
    /** The status code of the error. */
    enum picotm_error_status status;

    /** True is the error is non-recoverable, or false otherwise. */
    bool is_non_recoverable;

    /** A string with additonal information about the error. */
    const char* description;

    union {
        /**
         * The conflicting lock for PICOTM_CONFLICTING if known, or
         * NULL otherwise.
         */
        struct picotm_rwlock* conflicting_lock;

        /**
         * The picotm error code for PICOTM_ERROR_CODE if known, or
         * PICOTM_GENERAL_ERROR otherwise.
         */
        enum picotm_error_code error_hint;

        /**
         * The picotm errno for PICOTM_ERRNO if known, or 0 otherwise.
         */
        int errno_hint;

#if defined(PICOTM_HAVE_TYPE_KERN_RETURN_T) && \
        PICOTM_HAVE_TYPE_KERN_RETURN_T || \
    defined(__PICOTM_DOXYGEN)
        /**
         * The picotm kern_return_t value for PICOTM_KERN_RETURN_T.
         */
        kern_return_t kern_return_t_value;
#endif

#if defined(PICOTM_HAVE_TYPE_SIGINFO_T) && \
            PICOTM_HAVE_TYPE_SIGINFO_T || \
    defined(__PICOTM_DOXYGEN)
        /**
         * The picotm siginfo_t information for PICOTM_SIGINFO_T.
         */
        siginfo_t siginfo_t_info;
#endif
    } value;
};

#define PICOTM_ERROR_INITIALIZER    \
    { \
        .status = 0, \
        .is_non_recoverable = false, \
        .description = NULL \
    }

PICOTM_NOTHROW
/**
 * Clears an error structure.
 *
 * \param error The error to clear.
 */
void
picotm_error_clear(struct picotm_error* error);

PICOTM_NOTHROW
/**
 * Sets an error of type PICOTM_CONFLICTING.
 *
 * \param error             The error to set.
 * \param conflicting_lock  The conflicting lock if known, or NULL
 *                          otherwise.
 */
void
picotm_error_set_conflicting(struct picotm_error* error,
                             struct picotm_rwlock* conflicting_lock);

PICOTM_NOTHROW
/**
 * Sets an error of type PICOTM_REVOCABLE.
 *
 * \param error The error to set.
 */
void
picotm_error_set_revocable(struct picotm_error* error);

PICOTM_NOTHROW
/**
 * Sets an error of type PICOTM_ERROR_CODE.
 *
 * \param error         The error to set.
 * \param error_hint    The picotm error code if known, or
 *                      PICOTM_GENERAL_ERROR otherwise.
 */
void
picotm_error_set_error_code(struct picotm_error* error,
                            enum picotm_error_code error_hint);

PICOTM_NOTHROW
/**
 * Sets an error of type PICOTM_ERRNO.
 *
 * \param error         The error to set.
 * \param errno_hint    The errno code if known, or 0 otherwise.
 */
void
picotm_error_set_errno(struct picotm_error* error, int errno_hint);

#if defined(PICOTM_HAVE_TYPE_KERN_RETURN_T) && \
        PICOTM_HAVE_TYPE_KERN_RETURN_T || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Sets an error of type PICOTM_KERN_RETURN_T.
 *
 * \param error The error to set.
 * \param value The kern_return_t value.
 */
void
picotm_error_set_kern_return_t(struct picotm_error* error,
                               kern_return_t value);
#endif

#if defined(PICOTM_HAVE_TYPE_SIGINFO_T) && \
            PICOTM_HAVE_TYPE_SIGINFO_T || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Sets an error of type PICOTM_SIGINFO_T.
 *
 * \param[in,out]   error   The error to set.
 * \param[in]       info    The signal information.
 */
void
picotm_error_set_siginfo_t(struct picotm_error* error, const siginfo_t* info);
#endif

/**
 * Tests if an error has been set.
 *
 * \param error The error to set.
 * \returns     True if an error has been set, or false otherwise.
 */
static inline bool
picotm_error_is_set(const struct picotm_error* error)
{
    return !!error->status;
}

PICOTM_NOTHROW
/**
 * Tests if an error has been set to CONFLICTING status.
 *
 * \param error The error to set.
 * \returns     True if an error has been set to CONFLICTING, or false
 *              otherwise.
 */
bool
picotm_error_is_conflicting(const struct picotm_error* error);

PICOTM_NOTHROW
/**
 * Tests if an error has been set to REVOCABLE status.
 *
 * \param error The error to set.
 * \returns     True if an error has been set to REVOCABLE, or false
 *              otherwise.
 */
bool
picotm_error_is_revocable(const struct picotm_error* error);

PICOTM_NOTHROW
/**
 * Tests if an error has been set to an error status.
 *
 * \param error The error to set.
 * \returns     True if an error has been set to an error status, or false
 *              otherwise.
 */
bool
picotm_error_is_error(const struct picotm_error* error);

PICOTM_NOTHROW
/**
 * Marks an error as non-recoverable.
 *
 * By default, errors are expected to recoverable. For irrevocable
 * transactions or after applying events from the transaction log,
 * errors cannot easily be recovered, because the program state is
 * undefined in these situations. An errors that happens under such
 * conditions is marked as being non-recoverable.
 *
 * Normally, modules should not care baout this flag. The commit logic
 * within picotm will set this flag if appropriate.
 *
 * \param error The error to mark as non-recoverable.
 */
void
picotm_error_mark_as_non_recoverable(struct picotm_error* error);

PICOTM_NOTHROW
/**
 * Sets an error's description.
 *
 * \param error         The error.
 * \param description   A descriptive string.
 */
void
picotm_error_set_description(struct picotm_error* error,
                             const char* description);

PICOTM_NOTHROW
/**
 * Returns an error's description.
 *
 * \param error The error.
 * \returns     The error description if set, or NULL otherwise.
 */
const char*
picotm_error_get_description(const struct picotm_error* error);

PICOTM_END_DECLS
