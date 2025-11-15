/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann
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
#include <stddef.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <stddef.h>.
 */

#if defined(PICOTM_LIBC_HAVE_TYPE_PTRDIFF_T) && \
            PICOTM_LIBC_HAVE_TYPE_PTRDIFF_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(ptrdiff_t, ptrdiff_t);
PICOTM_TM_STORE_TX(ptrdiff_t, ptrdiff_t);
PICOTM_TM_PRIVATIZE_TX(ptrdiff_t, ptrdiff_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_SIZE_T) && PICOTM_LIBC_HAVE_TYPE_SIZE_T || \
    defined(__PICOTM_DOXYGEN)
#if !defined(__PICOTM_LOAD_SIZE_T_TX) || !__PICOTM_LOAD_SIZE_T_TX
#undef __PICOTM_LOAD_SIZE_T_TX
#define __PICOTM_LOAD_SIZE_T_TX     (1)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(size_t, size_t);
/** \} */
#endif
#if !defined(__PICOTM_STORE_SIZE_T_TX) || !__PICOTM_STORE_SIZE_T_TX
#undef __PICOTM_STORE_SIZE_T_TX
#define __PICOTM_STORE_SIZE_T_TX    (1)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_STORE_TX(size_t, size_t);
/** \} */
#endif
#if !defined(__PICOTM_PRIVATIZE_SIZE_T_TX) || !__PICOTM_PRIVATIZE_SIZE_T_TX
#undef __PICOTM_PRIVATIZE_SIZE_T_TX
#define __PICOTM_PRIVATIZE_SIZE_T_TX    (1)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_PRIVATIZE_TX(size_t, size_t);
/** \} */
#endif
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_WCHAR_T) && \
            PICOTM_LIBC_HAVE_TYPE_WCHAR_T || \
    defined(__PICOTM_DOXYGEN)
#if !defined(__PICOTM_LOAD_WCHAR_T_TX) || !__PICOTM_LOAD_WCHAR_T_TX
#undef __PICOTM_LOAD_WCHAR_T_TX
#define __PICOTM_LOAD_WCHAR_T_TX    (1)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(wchar_t, wchar_t);
/** \} */
#endif
#if !defined(__PICOTM_STORE_WCHAR_T_TX) || !__PICOTM_STORE_WCHAR_T_TX
#undef __PICOTM_STORE_WCHAR_T_TX
#define __PICOTM_STORE_WCHAR_T_TX   (1)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_STORE_TX(wchar_t, wchar_t);
/** \} */
#endif
#if !defined(__PICOTM_PRIVATIZE_WCHAR_T_TX) || \
    !__PICOTM_PRIVATIZE_WCHAR_T_TX
#undef __PICOTM_PRIVATIZE_WCHAR_T_TX
#define __PICOTM_PRIVATIZE_WCHAR_T_TX   (1)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_PRIVATIZE_TX(wchar_t, wchar_t);
/** \} */
#endif
#endif

PICOTM_END_DECLS
