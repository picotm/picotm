/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stddef.h>
#include <systx/systx-tm.h>

SYSTX_TM_LOAD_TX(ptrdiff_t, ptrdiff_t);
SYSTX_TM_LOAD_TX(size_t, size_t);
SYSTX_TM_LOAD_TX(wchar_t, wchar_t);

SYSTX_TM_STORE_TX(ptrdiff_t, ptrdiff_t);
SYSTX_TM_STORE_TX(size_t, size_t);
SYSTX_TM_STORE_TX(wchar_t, wchar_t);
