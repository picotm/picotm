/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
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

/**
 * \ingroup group_modules
 * \file
 *
 * \brief Contains macros for dealing with compiler extensions.
 */

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
    #define PICOTM_END_DECLS    }
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
#if defined(__CYGWIN__)
    #define PICOTM_EXPORT   __declspec(dllexport)
#elif __GNUC__ >= 4
    #define PICOTM_EXPORT   __attribute__((visibility("default")))
#else
    #define PICOTM_EXPORT
#endif
