/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <math.h>
#include <picotm/compiler.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libm
 * \file
 *
 * \brief Transactional wrappers for interfaces of <math.h>.
 */

PICOTM_NOTHROW
/**
 * Variant of frexp_tx() that operates on transactional memory.
 */
double
frexp_tm(double num, int* exp);

PICOTM_NOTHROW
/**
 * Variant of frexpf_tx() that operates on transactional memory.
 */
float
frexpf_tm(float num, int* exp);

PICOTM_NOTHROW
/**
 * Variant of frexpl_tx() that operates on transactional memory.
 */
long double
frexpl_tm(long double num, int* exp);

PICOTM_NOTHROW
/**
 * Variant of modf_tx() that operates on transactional memory.
 */
double
modf_tm(double x, double* iptr);

PICOTM_NOTHROW
/**
 * Variant of modff_tx() that operates on transactional memory.
 */
float
modff_tm(float x, float* iptr);

PICOTM_NOTHROW
/**
 * Variant of modfl_tx() that operates on transactional memory.
 */
long double
modfl_tm(long double x, long double* iptr);

PICOTM_NOTHROW
/**
 * Variant of nan_tx() that operates on transactional memory.
 */
double
nan_tm(const char* tagp);

PICOTM_NOTHROW
/**
 * Variant of nanf_tx() that operates on transactional memory.
 */
float
nanf_tm(const char* tagp);

PICOTM_NOTHROW
/**
 * Variant of nanl_tx() that operates on transactional memory.
 */
long double
nanl_tm(const char* tagp);

PICOTM_NOTHROW
/**
 * Variant of remquo_tx() that operates on transactional memory.
 */
double
remquo_tm(double x, double y, int* quo);

PICOTM_NOTHROW
/**
 * Variant of remquof_tx() that operates on transactional memory.
 */
float
remquof_tm(float x, float y, int* quo);

PICOTM_NOTHROW
/**
 * Variant of remquol_tx() that operates on transactional memory.
 */
long double
remquol_tm(long double x, long double y, int* quo);

PICOTM_END_DECLS
