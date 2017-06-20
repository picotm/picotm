/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/picotm-error.h"

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
picotm_error_set_conflicting(struct picotm_error* error, void* conflicting_tx)
{
    error->status = PICOTM_CONFLICTING;
    error->is_non_recoverable = false;
    error->description = NULL;
    error->value.conflicting_tx = conflicting_tx;
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
