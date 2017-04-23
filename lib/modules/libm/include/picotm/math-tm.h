/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <math.h>
#include <picotm/compiler.h>

PICOTM_BEGIN_DECLS

PICOTM_NOTHROW
/**
 * Executes frexp() within a transaction.
 */
double
frexp_tm(double num, int* exp);

PICOTM_NOTHROW
/**
 * Executes frexpf() within a transaction.
 */
float
frexpf_tm(float num, int* exp);

PICOTM_NOTHROW
/**
 * Executes frexpl() within a transaction.
 */
long double
frexpl_tm(long double num, int* exp);

PICOTM_NOTHROW
/**
 * Executes modf() within a transaction.
 */
double
modf_tm(double x, double* iptr);

PICOTM_NOTHROW
/**
 * Executes modff() within a transaction.
 */
float
modff_tm(float x, float* iptr);

PICOTM_NOTHROW
/**
 * Executes modfl() within a transaction.
 */
long double
modfl_tm(long double x, long double* iptr);

PICOTM_NOTHROW
/**
 * Executes nan() within a transaction.
 */
double
nan_tm(const char* tagp);

PICOTM_NOTHROW
/**
 * Executes nanf() within a transaction.
 */
float
nanf_tm(const char* tagp);

PICOTM_NOTHROW
/**
 * Executes nanl() within a transaction.
 */
long double
nanl_tm(const char* tagp);

PICOTM_NOTHROW
/**
 * Executes remquo() within a transaction.
 */
double
remquo_tm(double x, double y, int* quo);

PICOTM_NOTHROW
/**
 * Executes remquof() within a transaction.
 */
float
remquof_tm(float x, float y, int* quo);

PICOTM_NOTHROW
/**
 * Executes remquol() within a transaction.
 */
long double
remquol_tm(long double x, long double y, int* quo);

PICOTM_END_DECLS
