/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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
