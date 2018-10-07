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

#include "picotm/config/picotm-libm-config.h"
#include "picotm/compiler.h"
#include <math.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libm
 * \file
 *
 * \brief Transactional wrappers for interfaces of <math.h>.
 */

#if defined(PICOTM_LIBM_HAVE_FREXP) && PICOTM_LIBM_HAVE_FREXP || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libm
 * Variant of frexp_tx() that operates on transactional memory.
 */
double
frexp_tm(double num, int* exp);
#endif

#if defined(PICOTM_LIBM_HAVE_FREXPF) && PICOTM_LIBM_HAVE_FREXPF || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libm
 * Variant of frexpf_tx() that operates on transactional memory.
 */
float
frexpf_tm(float num, int* exp);
#endif

#if defined(PICOTM_LIBM_HAVE_FREXPL) && PICOTM_LIBM_HAVE_FREXPL || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libm
 * Variant of frexpl_tx() that operates on transactional memory.
 */
long double
frexpl_tm(long double num, int* exp);
#endif

#if defined(PICOTM_LIBM_HAVE_MODF) && PICOTM_LIBM_HAVE_MODF || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libm
 * Variant of modf_tx() that operates on transactional memory.
 */
double
modf_tm(double x, double* iptr);
#endif

#if defined(PICOTM_LIBM_HAVE_MODFF) && PICOTM_LIBM_HAVE_MODFF || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libm
 * Variant of modff_tx() that operates on transactional memory.
 */
float
modff_tm(float x, float* iptr);
#endif

#if defined(PICOTM_LIBM_HAVE_MODFL) && PICOTM_LIBM_HAVE_MODFL || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libm
 * Variant of modfl_tx() that operates on transactional memory.
 */
long double
modfl_tm(long double x, long double* iptr);
#endif

#if defined(PICOTM_LIBM_HAVE_NAN) && PICOTM_LIBM_HAVE_NAN || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libm
 * Variant of nan_tx() that operates on transactional memory.
 */
double
nan_tm(const char* tagp);
#endif

#if defined(PICOTM_LIBM_HAVE_NANF) && PICOTM_LIBM_HAVE_NANF || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libm
 * Variant of nanf_tx() that operates on transactional memory.
 */
float
nanf_tm(const char* tagp);
#endif

#if defined(PICOTM_LIBM_HAVE_NANL) && PICOTM_LIBM_HAVE_NANL || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libm
 * Variant of nanl_tx() that operates on transactional memory.
 */
long double
nanl_tm(const char* tagp);
#endif

#if defined(PICOTM_LIBM_HAVE_REMQUO) && PICOTM_LIBM_HAVE_REMQUO || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libm
 * Variant of remquo_tx() that operates on transactional memory.
 */
double
remquo_tm(double x, double y, int* quo);
#endif

#if defined(PICOTM_LIBM_HAVE_REMQUOF) && PICOTM_LIBM_HAVE_REMQUOF || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libm
 * Variant of remquof_tx() that operates on transactional memory.
 */
float
remquof_tm(float x, float y, int* quo);
#endif

#if defined(PICOTM_LIBM_HAVE_REMQUOL) && PICOTM_LIBM_HAVE_REMQUOL || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libm
 * Variant of remquol_tx() that operates on transactional memory.
 */
long double
remquol_tm(long double x, long double y, int* quo);
#endif

PICOTM_END_DECLS
