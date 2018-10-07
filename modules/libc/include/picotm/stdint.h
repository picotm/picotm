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
#include <stdint.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <stdint.h>.
 */

#if defined(PICOTM_LIBC_HAVE_TYPE_INT_FAST8_T) && \
            PICOTM_LIBC_HAVE_TYPE_INT_FAST8_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(int_fast8_t, int_fast8_t);
PICOTM_TM_STORE_TX(int_fast8_t, int_fast8_t);
PICOTM_TM_PRIVATIZE_TX(int_fast8_t, int_fast8_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_INT_FAST16_T) && \
            PICOTM_LIBC_HAVE_TYPE_INT_FAST16_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(int_fast16_t, int_fast16_t);
PICOTM_TM_STORE_TX(int_fast16_t, int_fast16_t);
PICOTM_TM_PRIVATIZE_TX(int_fast16_t, int_fast16_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_INT_FAST32_T) && \
            PICOTM_LIBC_HAVE_TYPE_INT_FAST32_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(int_fast32_t, int_fast32_t);
PICOTM_TM_STORE_TX(int_fast32_t, int_fast32_t);
PICOTM_TM_PRIVATIZE_TX(int_fast32_t, int_fast32_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_INT_FAST64_T) && \
            PICOTM_LIBC_HAVE_TYPE_INT_FAST64_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(int_fast64_t, int_fast64_t);
PICOTM_TM_STORE_TX(int_fast64_t, int_fast64_t);
PICOTM_TM_PRIVATIZE_TX(int_fast64_t, int_fast64_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_INT_LEAST8_T) && \
            PICOTM_LIBC_HAVE_TYPE_INT_LEAST8_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(int_least8_t, int_least8_t);
PICOTM_TM_STORE_TX(int_least8_t, int_least8_t);
PICOTM_TM_PRIVATIZE_TX(int_least8_t, int_least8_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_INT_LEAST16_T) && \
            PICOTM_LIBC_HAVE_TYPE_INT_LEAST16_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(int_least16_t, int_least16_t);
PICOTM_TM_STORE_TX(int_least16_t, int_least16_t);
PICOTM_TM_PRIVATIZE_TX(int_least16_t, int_least16_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_INT_LEAST32_T) && \
            PICOTM_LIBC_HAVE_TYPE_INT_LEAST32_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(int_least32_t, int_least32_t);
PICOTM_TM_STORE_TX(int_least32_t, int_least32_t);
PICOTM_TM_PRIVATIZE_TX(int_least32_t, int_least32_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_INT_LEAST64_T) && \
            PICOTM_LIBC_HAVE_TYPE_INT_LEAST64_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(int_least64_t, int_least64_t);
PICOTM_TM_STORE_TX(int_least64_t, int_least64_t);
PICOTM_TM_PRIVATIZE_TX(int_least64_t, int_least64_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_INT8_T) && PICOTM_LIBC_HAVE_TYPE_INT8_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(int8_t, int8_t);
PICOTM_TM_STORE_TX(int8_t, int8_t);
PICOTM_TM_PRIVATIZE_TX(int8_t, int8_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_INT16_T) && \
            PICOTM_LIBC_HAVE_TYPE_INT16_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(int16_t, int16_t);
PICOTM_TM_STORE_TX(int16_t, int16_t);
PICOTM_TM_PRIVATIZE_TX(int16_t, int16_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_INT32_T) && \
            PICOTM_LIBC_HAVE_TYPE_INT32_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(int32_t, int32_t);
PICOTM_TM_STORE_TX(int32_t, int32_t);
PICOTM_TM_PRIVATIZE_TX(int32_t, int32_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_INT64_T) && \
            PICOTM_LIBC_HAVE_TYPE_INT64_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(int64_t, int64_t);
PICOTM_TM_STORE_TX(int64_t, int64_t);
PICOTM_TM_PRIVATIZE_TX(int64_t, int64_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_INTMAX_T) && \
            PICOTM_LIBC_HAVE_TYPE_INTMAX_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(intmax_t, intmax_t);
PICOTM_TM_STORE_TX(intmax_t, intmax_t);
PICOTM_TM_PRIVATIZE_TX(intmax_t, intmax_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_INTPTR_T) && \
            PICOTM_LIBC_HAVE_TYPE_INTPTR_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(intptr_t, intptr_t);
PICOTM_TM_STORE_TX(intptr_t, intptr_t);
PICOTM_TM_PRIVATIZE_TX(intptr_t, intptr_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_UINT_FAST8_T) && \
            PICOTM_LIBC_HAVE_TYPE_UINT_FAST8_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(uint_fast8_t, uint_fast8_t);
PICOTM_TM_STORE_TX(uint_fast8_t, uint_fast8_t);
PICOTM_TM_PRIVATIZE_TX(uint_fast8_t, uint_fast8_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_UINT_FAST16_T) && \
            PICOTM_LIBC_HAVE_TYPE_UINT_FAST16_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(uint_fast16_t, uint_fast16_t);
PICOTM_TM_STORE_TX(uint_fast16_t, uint_fast16_t);
PICOTM_TM_PRIVATIZE_TX(uint_fast16_t, uint_fast16_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_UINT_FAST32_T) && \
            PICOTM_LIBC_HAVE_TYPE_UINT_FAST32_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(uint_fast32_t, uint_fast32_t);
PICOTM_TM_STORE_TX(uint_fast32_t, uint_fast32_t);
PICOTM_TM_PRIVATIZE_TX(uint_fast32_t, uint_fast32_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_UINT_FAST64_T) && \
            PICOTM_LIBC_HAVE_TYPE_UINT_FAST64_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(uint_fast64_t, uint_fast64_t);
PICOTM_TM_STORE_TX(uint_fast64_t, uint_fast64_t);
PICOTM_TM_PRIVATIZE_TX(uint_fast64_t, uint_fast64_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_UINT_LEAST8_T) && \
            PICOTM_LIBC_HAVE_TYPE_UINT_LEAST8_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(uint_least8_t, uint_least8_t);
PICOTM_TM_STORE_TX(uint_least8_t, uint_least8_t);
PICOTM_TM_PRIVATIZE_TX(uint_least8_t, uint_least8_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_UINT_LEAST16_T) && \
            PICOTM_LIBC_HAVE_TYPE_UINT_LEAST16_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(uint_least16_t, uint_least16_t);
PICOTM_TM_STORE_TX(uint_least16_t, uint_least16_t);
PICOTM_TM_PRIVATIZE_TX(uint_least16_t, uint_least16_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_UINT_LEAST32_T) && \
            PICOTM_LIBC_HAVE_TYPE_UINT_LEAST32_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(uint_least32_t, uint_least32_t);
PICOTM_TM_STORE_TX(uint_least32_t, uint_least32_t);
PICOTM_TM_PRIVATIZE_TX(uint_least32_t, uint_least32_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_UINT_LEAST64_T) && \
            PICOTM_LIBC_HAVE_TYPE_UINT_LEAST64_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(uint_least64_t, uint_least64_t);
PICOTM_TM_STORE_TX(uint_least64_t, uint_least64_t);
PICOTM_TM_PRIVATIZE_TX(uint_least64_t, uint_least64_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_UINT8_T) && \
            PICOTM_LIBC_HAVE_TYPE_UINT8_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(uint8_t, uint8_t);
PICOTM_TM_STORE_TX(uint8_t, uint8_t);
PICOTM_TM_PRIVATIZE_TX(uint8_t, uint8_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_UINT16_T) && \
            PICOTM_LIBC_HAVE_TYPE_UINT16_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(uint16_t, uint16_t);
PICOTM_TM_STORE_TX(uint16_t, uint16_t);
PICOTM_TM_PRIVATIZE_TX(uint16_t, uint16_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_UINT32_T) && \
            PICOTM_LIBC_HAVE_TYPE_UINT32_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(uint32_t, uint32_t);
PICOTM_TM_STORE_TX(uint32_t, uint32_t);
PICOTM_TM_PRIVATIZE_TX(uint32_t, uint32_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_UINT64_T) && \
            PICOTM_LIBC_HAVE_TYPE_UINT64_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(uint64_t, uint64_t);
PICOTM_TM_STORE_TX(uint64_t, uint64_t);
PICOTM_TM_PRIVATIZE_TX(uint64_t, uint64_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_UINTMAX_T) && \
            PICOTM_LIBC_HAVE_TYPE_UINTMAX_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(uintmax_t, uintmax_t);
PICOTM_TM_STORE_TX(uintmax_t, uintmax_t);
PICOTM_TM_PRIVATIZE_TX(uintmax_t, uintmax_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_UINTPTR_T) && \
            PICOTM_LIBC_HAVE_TYPE_UINTPTR_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(uintptr_t, uintptr_t);
PICOTM_TM_STORE_TX(uintptr_t, uintptr_t);
PICOTM_TM_PRIVATIZE_TX(uintptr_t, uintptr_t);
/** \} */
#endif

PICOTM_END_DECLS
