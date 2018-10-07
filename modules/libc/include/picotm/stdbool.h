/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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

#pragma once

#include "picotm/config/picotm-libc-config.h"
#include "picotm/compiler.h"
#include "picotm/picotm-tm.h"
#include <stdbool.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <stdbool.h>.
 */

#if defined(PICOTM_LIBC_HAVE_TYPE_BOOL) && \
            PICOTM_LIBC_HAVE_TYPE_BOOL || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(bool, bool)
PICOTM_TM_STORE_TX(bool, bool)
PICOTM_TM_PRIVATIZE_TX(bool, bool)
/** \} */
#endif

PICOTM_END_DECLS
