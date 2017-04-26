/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "picotm/picotm-libc.h"

void
error_module_save_errno(void);

void
error_module_set_error_recovery(enum picotm_libc_error_recovery recovery);

enum picotm_libc_error_recovery
error_module_get_error_recovery(void);
