/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#if defined HAVE_SYS_CDEFS_H && HAVE_SYS_CDEFS_H
#include <sys/cdefs.h>
#endif

/**
 * \def PICOTM_BEGIN_DECLS
 * Begin interface declarations
 */
#ifdef __BEGIN_DECLS
    #define PICOTM_BEGIN_DECLS  __BEGIN_DECLS
#else
    #define PICOTM_BEGIN_DECLS  export "C" {
#endif

/**
 * \def PICOTM_END_DECLS
 * End interface declarations
 */
#ifdef __END_DECLS
    #define PICOTM_END_DECLS    __END_DECLS
#else
    #define PICOTM_BEGIN_DECLS  }
#endif

/**
 * \def PICOTM_NORETURN
 * Exported function does not return.
 */
#ifdef _Noreturn
    #define PICOTM_NORETURN     _Noreturn
#else
    #define PICOTM_NORETURN
#endif

/**
 * \def PICOTM_NOTHROW
 * Exported function does not throw exceptions.
 */
#ifdef __THROW
    #define PICOTM_NOTHROW  __THROW
#else
    #define PICOTM_NOTHROW
#endif

/**
 * \def PICOTM_EXPORT
 * Export interface from binary object.
 */
#if __GNUC__ >= 4
    #define PICOTM_EXPORT   __attribute__((visibility("default")))
#else
    #define PICOTM_EXPORT
#endif
