/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#if defined HAVE_SYS_CDEFS_H && HAVE_SYS_CDEFS_H
#include <sys/cdefs.h>
#endif

/**
 * \def SYSTX_BEGIN_DECLS
 * Begin interface declarations
 */
#ifdef __BEGIN_DECLS
    #define SYSTX_BEGIN_DECLS   __BEGIN_DECLS
#else
    #define SYSTX_BEGIN_DECLS   export "C" {
#endif

/**
 * \def SYSTX_END_DECLS
 * End interface declarations
 */
#ifdef __END_DECLS
    #define SYSTX_END_DECLS     __END_DECLS
#else
    #define SYSTX_BEGIN_DECLS   }
#endif

/**
 * \def SYSTX_NOTHROW
 * Exported function does not throw exceptions.
 */
#ifdef __THROW
    #define SYSTX_NOTHROW   __THROW
#else
    #define SYSTX_NOTHROW
#endif

/**
 * \def SYSTX_EXPORT
 * Export interface from binary object.
 */

#if __GNUC__ >= 4
    #define SYSTX_EXPORT    __attribute__((visibility("default")))
#else
    #define SYSTX_EXPORT
#endif
