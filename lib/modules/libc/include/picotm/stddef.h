/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stddef.h>
#include <picotm/picotm-tm.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <stddef.h>.
 */

PICOTM_TM_LOAD_TX(ptrdiff_t, ptrdiff_t);
PICOTM_TM_LOAD_TX(size_t, size_t);
PICOTM_TM_LOAD_TX(wchar_t, wchar_t);

PICOTM_TM_STORE_TX(ptrdiff_t, ptrdiff_t);
PICOTM_TM_STORE_TX(size_t, size_t);
PICOTM_TM_STORE_TX(wchar_t, wchar_t);

PICOTM_END_DECLS
