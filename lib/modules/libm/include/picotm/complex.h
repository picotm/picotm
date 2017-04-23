/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <complex.h>
#include <picotm/compiler.h>
#include <picotm/picotm-tm.h>

PICOTM_BEGIN_DECLS

PICOTM_TM_LOAD_TX(cdouble, double _Complex);
PICOTM_TM_LOAD_TX(cfloat, float _Complex);
PICOTM_TM_LOAD_TX(cldouble, long double _Complex);

PICOTM_TM_STORE_TX(cdouble, double _Complex);
PICOTM_TM_STORE_TX(cfloat, float _Complex);
PICOTM_TM_STORE_TX(cldouble, long double _Complex);

PICOTM_TM_PRIVATIZE_TX(cdouble, double _Complex);
PICOTM_TM_PRIVATIZE_TX(cfloat, float _Complex);
PICOTM_TM_PRIVATIZE_TX(cldouble, long double _Complex);

PICOTM_NOTHROW
/**
 * Executes cabs() within a transaction.
 */
double
cabs_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes cabsf() within a transaction.
 */
float
cabsf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes cabsl() within a transaction.
 */
long double
cabsl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes cacos() within a transaction.
 */
double complex
cacos_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes cacosf() within a transaction.
 */
float complex
cacosf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes cacosh() within a transaction.
 */
double complex
cacosh_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes cacoshf() within a transaction.
 */
float complex
cacoshf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes cacoshl() within a transaction.
 */
long double complex
cacoshl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes cacosl() within a transaction.
 */
long double complex
cacosl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes carg() within a transaction.
 */
double
carg_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes cargf() within a transaction.
 */
float
cargf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes cargl() within a transaction.
 */
long double
cargl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes casin() within a transaction.
 */
double complex
casin_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes casinf() within a transaction.
 */
float complex
casinf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes casinh() within a transaction.
 */
double complex
casinh_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes casinhf() within a transaction.
 */
float complex
casinhf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes casinhl() within a transaction.
 */
long double complex
casinhl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes casinl() within a transaction.
 */
long double complex
casinl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes catan() within a transaction.
 */
double complex
catan_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes catanf() within a transaction.
 */
float complex
catanf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes catanh() within a transaction.
 */
double complex
catanh_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes catanhf() within a transaction.
 */
float complex
catanhf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes catanhl() within a transaction.
 */
long double complex
catanhl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes catanl() within a transaction.
 */
long double complex
catanl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes ccos() within a transaction.
 */
double complex
ccos_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes ccosf() within a transaction.
 */
float complex
ccosf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes ccosh() within a transaction.
 */
double complex
ccosh_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes ccoshf() within a transaction.
 */
float complex
ccoshf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes ccoshl() within a transaction.
 */
long double complex
ccoshl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes ccosl() within a transaction.
 */
long double complex
ccosl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes cexp() within a transaction.
 */
double complex
cexp_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes cexpf() within a transaction.
 */
float complex
cexpf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes cexpl() within a transaction.
 */
long double complex
cexpl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes cimag() within a transaction.
 */
double
cimag_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes cimagf() within a transaction.
 */
float
cimagf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes cimagl() within a transaction.
 */
long double
cimagl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes clog() within a transaction.
 */
double complex
clog_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes clogf() within a transaction.
 */
float complex
clogf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes clogl() within a transaction.
 */
long double complex
clogl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes conj() within a transaction.
 */
double complex
conj_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes conjf() within a transaction.
 */
float complex
conjf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes conjl() within a transaction.
 */
long double complex
conjl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes cpow() within a transaction.
 */
double complex
cpow_tx(double complex x, double complex y);

PICOTM_NOTHROW
/**
 * Executes cpowf() within a transaction.
 */
float complex
cpowf_tx(float complex x, float complex y);

PICOTM_NOTHROW
/**
 * Executes cpowl() within a transaction.
 */
long double complex
cpowl_tx(long double complex x, long double complex y);

PICOTM_NOTHROW
/**
 * Executes cproj() within a transaction.
 */
double complex
cproj_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes cprojf() within a transaction.
 */
float complex
cprojf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes cprojl() within a transaction.
 */
long double complex
cprojl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes creal() within a transaction.
 */
double
creal_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes crealf() within a transaction.
 */
float
crealf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes creall() within a transaction.
 */
long double
creall_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes csin() within a transaction.
 */
double complex
csin_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes csinf() within a transaction.
 */
float complex
csinf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes csinh() within a transaction.
 */
double complex
csinh_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes csinhf() within a transaction.
 */
float complex
csinhf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes csinhl() within a transaction.
 */
long double complex
csinhl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes csinl() within a transaction.
 */
long double complex
csinl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes csqrt() within a transaction.
 */
double complex
csqrt_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes csqrtf() within a transaction.
 */
float complex
csqrtf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes csqrtl() within a transaction.
 */
long double complex
csqrtl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes ctan() within a transaction.
 */
double complex
ctan_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes ctanf() within a transaction.
 */
float complex
ctanf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes ctanh() within a transaction.
 */
double complex
ctanh_tx(double complex z);

PICOTM_NOTHROW
/**
 * Executes ctanhf() within a transaction.
 */
float complex
ctanhf_tx(float complex z);

PICOTM_NOTHROW
/**
 * Executes ctanhl() within a transaction.
 */
long double complex
ctanhl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * Executes ctanhl() within a transaction.
 */
long double complex
ctanl_tx(long double complex z);

PICOTM_END_DECLS
