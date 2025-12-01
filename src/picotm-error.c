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

#include "picotm/picotm-error.h"
#include <string.h>

PICOTM_EXPORT
void
picotm_error_clear(struct picotm_error* error)
{
    error->status = 0;
    error->is_non_recoverable = false;
    error->description = NULL;
}

PICOTM_EXPORT
void
picotm_error_set_conflicting(struct picotm_error* error,
                             struct picotm_rwlock* conflicting_lock)
{
    error->status = PICOTM_CONFLICTING;
    error->is_non_recoverable = false;
    error->description = NULL;
    error->value.conflicting_lock = conflicting_lock;
}

PICOTM_EXPORT
void
picotm_error_set_revocable(struct picotm_error* error)
{
    error->status = PICOTM_REVOCABLE;
    error->is_non_recoverable = false;
    error->description = NULL;
}

PICOTM_EXPORT
void
picotm_error_set_error_code(struct picotm_error* error,
                            enum picotm_error_code error_hint)
{
    error->status = PICOTM_ERROR_CODE;
    error->is_non_recoverable = false;
    error->description = NULL;
    error->value.error_hint = error_hint;
}

PICOTM_EXPORT
void
picotm_error_set_errno(struct picotm_error* error, int errno_hint)
{
    error->status = PICOTM_ERRNO;
    error->is_non_recoverable = false;
    error->description = NULL;
    error->value.errno_hint = errno_hint;
}

#if defined(PICOTM_HAVE_TYPE_KERN_RETURN_T) && PICOTM_HAVE_TYPE_KERN_RETURN_T
PICOTM_EXPORT
void
picotm_error_set_kern_return_t(struct picotm_error* error,
                               kern_return_t value)
{
    error->status = PICOTM_KERN_RETURN_T;
    error->is_non_recoverable = false;
    error->description = NULL;
    error->value.kern_return_t_value = value;
}
#endif

#if defined(PICOTM_HAVE_TYPE_SIGINFO_T) && PICOTM_HAVE_TYPE_SIGINFO_T
PICOTM_EXPORT
void
picotm_error_set_siginfo_t(struct picotm_error* error, const siginfo_t* info)
{
    error->status = PICOTM_SIGINFO_T;
    error->is_non_recoverable = false;
    error->description = NULL;
    memcpy(&error->value.siginfo_t_info, info, sizeof(*info));
}
#endif

PICOTM_EXPORT
bool
picotm_error_is_conflicting(const struct picotm_error* error)
{
    return error->status == PICOTM_CONFLICTING;
}

PICOTM_EXPORT
bool
picotm_error_is_revocable(const struct picotm_error* error)
{
    return error->status == PICOTM_REVOCABLE;
}

PICOTM_EXPORT
bool
picotm_error_is_error(const struct picotm_error* error)
{
    return picotm_error_is_set(error) && !picotm_error_is_conflicting(error);
}

PICOTM_EXPORT
void
picotm_error_mark_as_non_recoverable(struct picotm_error* error)
{
    error->is_non_recoverable = true;
}

PICOTM_EXPORT
void
picotm_error_set_description(struct picotm_error* error,
                             const char* description)
{
    error->description = description;
}

PICOTM_EXPORT
const char*
picotm_error_get_description(const struct picotm_error* error)
{
    return error->description;
}
