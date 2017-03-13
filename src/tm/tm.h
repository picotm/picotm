/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdbool.h>

int systx_tm_lock(void);
int systx_tm_unlock(void);
int systx_tm_validate(bool eotx);
int systx_tm_apply(void);
int systx_tm_undo(void);
int systx_tm_finish(void);
int systx_tm_release(void);
