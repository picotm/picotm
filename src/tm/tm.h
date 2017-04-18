/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdbool.h>

int picotm_tm_lock(void);
int picotm_tm_unlock(void);
int picotm_tm_validate(bool eotx);
int picotm_tm_apply(void);
int picotm_tm_undo(void);
int picotm_tm_finish(void);
int picotm_tm_release(void);
