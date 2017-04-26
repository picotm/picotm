/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/errno.h"
#include "error/module.h"

PICOTM_EXPORT
int*
__errno_location_tx()
{
    error_module_save_errno();
    return &errno;
}
