/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
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
