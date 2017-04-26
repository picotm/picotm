/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/errno.h"
#include "error/module.h"
#include "picotm/picotm-libc.h"

PICOTM_EXPORT
void
picotm_libc_save_errno()
{
    error_module_save_errno();
}

PICOTM_EXPORT
int*
__errno_location_tx()
{
    error_module_save_errno();
    return &errno;
}

PICOTM_EXPORT
void
picotm_libc_set_error_recovery(enum picotm_libc_error_recovery recovery)
{
    return error_module_set_error_recovery(recovery);
}

PICOTM_EXPORT
enum picotm_libc_error_recovery
picotm_libc_get_error_recovery()
{
    return error_module_get_error_recovery();
}
