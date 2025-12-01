/*
 * picotm - A system-level transaction manager
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

#include "picotm/config/picotm-config.h"

/**
 * \ingroup group_core
 * \file
 *
 * \brief Contains macros for dealing with compiler extensions.
 */

#if defined HAVE_SYS_CDEFS_H && HAVE_SYS_CDEFS_H
#include <sys/cdefs.h>
#endif

/**
 * \ingroup group_core
 * \def PICOTM_BEGIN_DECLS
 * Begin interface declarations
 */
#ifdef __BEGIN_DECLS
    #define PICOTM_BEGIN_DECLS  __BEGIN_DECLS
#else
    #define PICOTM_BEGIN_DECLS  export "C" {
#endif

/**
 * \ingroup group_core
 * \def PICOTM_END_DECLS
 * End interface declarations
 */
#ifdef __END_DECLS
    #define PICOTM_END_DECLS    __END_DECLS
#else
    #define PICOTM_END_DECLS    }
#endif

/**
 * \ingroup group_core
 * \def PICOTM_NORETURN
 * Exported function does not return.
 */
#ifdef _Noreturn
    #define PICOTM_NORETURN     _Noreturn
#else
    #define PICOTM_NORETURN
#endif

/**
 * \ingroup group_core
 * \def PICOTM_NOTHROW
 * Exported function does not throw exceptions.
 */
#ifdef __THROW
    #define PICOTM_NOTHROW  __THROW
#else
    #define PICOTM_NOTHROW
#endif

/**
 * \ingroup group_core
 * \def PICOTM_EXPORT
 * Export interface from binary object.
 */
#if defined(__CYGWIN__)
    #define PICOTM_EXPORT   __declspec(dllexport)
#elif __GNUC__ >= 4
    #define PICOTM_EXPORT   __attribute__((visibility("default")))
#else
    #define PICOTM_EXPORT
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112l) || \
    defined(__PICOTM_DOXYGEN)
    /**
     * \ingroup group_core
     * Provides a portable static assertion.
     * \brief   _cond   The compile-time assertion.
     * \brief   _errmsg An error message that is printed if the condition
     *                  fails.
     */
    #define PICOTM_STATIC_ASSERT(_cond, _errmsg)    \
        _Static_assert((_cond), _errmsg)
#elif defined(__GNUC__)
    #define PICOTM_STATIC_ASSERT(_cond, _errmsg)                            \
        (__extension__({                                                    \
            int assertion_holds[1 - ( 2 *!(_cond))]                         \
                    __attribute__((__unused__));                            \
        }))
#else
    #define PICOTM_STATIC_ASSERT(_cond, _errmsg)
#endif
