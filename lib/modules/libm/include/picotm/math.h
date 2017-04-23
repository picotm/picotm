/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <math.h>
#include <picotm/compiler.h>
#include <picotm/picotm-tm.h>

PICOTM_BEGIN_DECLS

PICOTM_TM_LOAD_TX(float_t, float_t);
PICOTM_TM_LOAD_TX(double_t, double_t);

PICOTM_TM_STORE_TX(float_t, float_t);
PICOTM_TM_STORE_TX(double_t, double_t);

PICOTM_TM_PRIVATIZE_TX(float_t, float_t);
PICOTM_TM_PRIVATIZE_TX(double_t, double_t);

PICOTM_NOTHROW
/**
 * Executes acos() within a transaction.
 */
double
acos_tx(double x);

PICOTM_NOTHROW
/**
 * Executes acosf() within a transaction.
 */
float
acosf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes acosh() within a transaction.
 */
double
acosh_tx(double x);

PICOTM_NOTHROW
/**
 * Executes acoshf() within a transaction.
 */
float
acoshf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes acoshl() within a transaction.
 */
long double
acoshl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes acosl() within a transaction.
 */
long double
acosl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes asin() within a transaction.
 */
double
asin_tx(double x);

PICOTM_NOTHROW
/**
 * Executes asinf() within a transaction.
 */
float
asinf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes asinh() within a transaction.
 */
double
asinh_tx(double x);

PICOTM_NOTHROW
/**
 * Executes asinhf() within a transaction.
 */
float
asinhf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes asinhl() within a transaction.
 */
long double
asinhl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes asinl() within a transaction.
 */
long double
asinl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes atan() within a transaction.
 */
double
atan_tx(double x);

PICOTM_NOTHROW
/**
 * Executes atan2() within a transaction.
 */
double
atan2_tx(double y, double x);

PICOTM_NOTHROW
/**
 * Executes atan2f() within a transaction.
 */
float
atan2f_tx(float y, float x);

PICOTM_NOTHROW
/**
 * Executes atan2l() within a transaction.
 */
long double
atan2l_tx(long double y, long double x);

PICOTM_NOTHROW
/**
 * Executes atanf() within a transaction.
 */
float
atanf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes atanh() within a transaction.
 */
double
atanh_tx(double x);

PICOTM_NOTHROW
/**
 * Executes atanhf() within a transaction.
 */
float
atanhf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes atanhl() within a transaction.
 */
long double
atanhl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes atanl() within a transaction.
 */
long double
atanl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes cbrt() within a transaction.
 */
double
cbrt_tx(double x);

PICOTM_NOTHROW
/**
 * Executes cbrtf() within a transaction.
 */
float
cbrtf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes cbrtl() within a transaction.
 */
long double
cbrtl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes ceil() within a transaction.
 */
double
ceil_tx(double x);

PICOTM_NOTHROW
/**
 * Executes ceilf() within a transaction.
 */
float
ceilf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes ceill() within a transaction.
 */
long double
ceill_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes copysign() within a transaction.
 */
double
copysign_tx(double x, double y);

PICOTM_NOTHROW
/**
 * Executes copysignf() within a transaction.
 */
float
copysignf_tx(float x, float y);

PICOTM_NOTHROW
/**
 * Executes copysignl() within a transaction.
 */
long double
copysignl_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * Executes cos() within a transaction.
 */
double
cos_tx(double x);

PICOTM_NOTHROW
/**
 * Executes cosf() within a transaction.
 */
float
cosf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes cosh() within a transaction.
 */
double
cosh_tx(double x);

PICOTM_NOTHROW
/**
 * Executes coshf() within a transaction.
 */
float
coshf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes coshl() within a transaction.
 */
long double
coshl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes cosl() within a transaction.
 */
long double
cosl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes erf() within a transaction.
 */
double
erf_tx(double x);

PICOTM_NOTHROW
/**
 * Executes erfc() within a transaction.
 */
double
erfc_tx(double x);

PICOTM_NOTHROW
/**
 * Executes erfcf() within a transaction.
 */
float
erfcf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes erfcl() within a transaction.
 */
long double
erfcl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes erff() within a transaction.
 */
float
erff_tx(float x);

PICOTM_NOTHROW
/**
 * Executes erfl() within a transaction.
 */
long double
erfl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes exp() within a transaction.
 */
double
exp_tx(double x);

PICOTM_NOTHROW
/**
 * Executes exp2() within a transaction.
 */
double
exp2_tx(double x);

PICOTM_NOTHROW
/**
 * Executes exp2f() within a transaction.
 */
float
exp2f_tx(float x);

PICOTM_NOTHROW
/**
 * Executes exp2l() within a transaction.
 */
long double
exp2l_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes expf() within a transaction.
 */
float
expf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes expl() within a transaction.
 */
long double
expl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes expm1() within a transaction.
 */
double
expm1_tx(double x);

PICOTM_NOTHROW
/**
 * Executes expm1f() within a transaction.
 */
float
expm1f_tx(float x);

PICOTM_NOTHROW
/**
 * Executes expm1l() within a transaction.
 */
long double
expm1l_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes fabs() within a transaction.
 */
double
fabs_tx(double x);

PICOTM_NOTHROW
/**
 * Executes fabsf() within a transaction.
 */
float
fabsf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes fabsl() within a transaction.
 */
long double
fabsl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes fdim() within a transaction.
 */
double
fdim_tx(double x, double y);

PICOTM_NOTHROW
/**
 * Executes fdimf() within a transaction.
 */
float
fdimf_tx(float x, float y);

PICOTM_NOTHROW
/**
 * Executes fdiml() within a transaction.
 */
long double
fdiml_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * Executes floor() within a transaction.
 */
double
floor_tx(double x);

PICOTM_NOTHROW
/**
 * Executes floorf() within a transaction.
 */
float
floorf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes floorl() within a transaction.
 */
long double
floorl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes fma() within a transaction.
 */
double
fma_tx(double x, double y, double z);

PICOTM_NOTHROW
/**
 * Executes fmaf() within a transaction.
 */
float
fmaf_tx(float x, float y, float z);

PICOTM_NOTHROW
/**
 * Executes fmal() within a transaction.
 */
long double
fmal_tx(long double x, long double y, long double z);

PICOTM_NOTHROW
/**
 * Executes fmax() within a transaction.
 */
double
fmax_tx(double x, double y);

PICOTM_NOTHROW
/**
 * Executes fmaxf() within a transaction.
 */
float
fmaxf_tx(float x, float y);

PICOTM_NOTHROW
/**
 * Executes fmaxl() within a transaction.
 */
long double
fmaxl_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * Executes fmin() within a transaction.
 */
double
fmin_tx(double x, double y);

PICOTM_NOTHROW
/**
 * Executes fminf() within a transaction.
 */
float
fminf_tx(float x, float y);

PICOTM_NOTHROW
/**
 * Executes fminl() within a transaction.
 */
long double
fminl_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * Executes fmodl() within a transaction.
 */
double
fmod_tx(double x, double y);

PICOTM_NOTHROW
/**
 * Executes fmodf() within a transaction.
 */
float
fmodf_tx(float x, float y);

PICOTM_NOTHROW
/**
 * Executes fmodl() within a transaction.
 */
long double
fmodl_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * Executes frexp() within a transaction.
 */
double
frexp_tx(double num, int* exp);

PICOTM_NOTHROW
/**
 * Executes frexpf() within a transaction.
 */
float
frexpf_tx(float num, int* exp);

PICOTM_NOTHROW
/**
 * Executes frexpl() within a transaction.
 */
long double
frexpl_tx(long double num, int* exp);

PICOTM_NOTHROW
/**
 * Executes hypot() within a transaction.
 */
double
hypot_tx(double x, double y);

PICOTM_NOTHROW
/**
 * Executes hypotf() within a transaction.
 */
float
hypotf_tx(float x, float y);

PICOTM_NOTHROW
/**
 * Executes hypotl() within a transaction.
 */
long double
hypotl_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * Executes ilogb() within a transaction.
 */
double
ilogb_tx(double x);

PICOTM_NOTHROW
/**
 * Executes ilogbf() within a transaction.
 */
float
ilogbf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes ilogbl() within a transaction.
 */
long double
ilogbl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes j0() within a transaction.
 */
double
j0_tx(double x);

PICOTM_NOTHROW
/**
 * Executes j1() within a transaction.
 */
double
j1_tx(double x);

PICOTM_NOTHROW
/**
 * Executes jn() within a transaction.
 */
double
jn_tx(int n, double x);

PICOTM_NOTHROW
/**
 * Executes ldexp() within a transaction.
 */
double
ldexp_tx(double x, int exp);

PICOTM_NOTHROW
/**
 * Executes ldexpf() within a transaction.
 */
float
ldexpf_tx(float x, int exp);

PICOTM_NOTHROW
/**
 * Executes ldexpl() within a transaction.
 */
long double
ldexpl_tx(long double x, int exp);

PICOTM_NOTHROW
/**
 * Executes lgamma() within a transaction.
 */
double
lgamma_tx(double x);

PICOTM_NOTHROW
/**
 * Executes lgammaf() within a transaction.
 */
float
lgammaf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes lgammal() within a transaction.
 */
long double
lgammal_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes llrint() within a transaction.
 */
long long
llrint_tx(double x);

PICOTM_NOTHROW
/**
 * Executes llrintf() within a transaction.
 */
long long
llrintf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes llrintl() within a transaction.
 */
long long
llrintl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes llround() within a transaction.
 */
long long
llround_tx(double x);

PICOTM_NOTHROW
/**
 * Executes llroundf() within a transaction.
 */
long long
llroundf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes llroundl() within a transaction.
 */
long long
llroundl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes log() within a transaction.
 */
double
log_tx(double x);

PICOTM_NOTHROW
/**
 * Executes log10() within a transaction.
 */
double
log10_tx(double x);

PICOTM_NOTHROW
/**
 * Executes log10f() within a transaction.
 */
float
log10f_tx(float x);

PICOTM_NOTHROW
/**
 * Executes log10l() within a transaction.
 */
long double
log10l_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes log1p() within a transaction.
 */
double
log1p_tx(double x);

PICOTM_NOTHROW
/**
 * Executes log1pf() within a transaction.
 */
float
log1pf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes log1pl() within a transaction.
 */
long double
log1pl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes log2() within a transaction.
 */
double
log2_tx(double x);

PICOTM_NOTHROW
/**
 * Executes log2f() within a transaction.
 */
float
log2f_tx(float x);

PICOTM_NOTHROW
/**
 * Executes log2l() within a transaction.
 */
long double
log2l_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes logb() within a transaction.
 */
double
logb_tx(double x);

PICOTM_NOTHROW
/**
 * Executes logbf() within a transaction.
 */
float
logbf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes logbl() within a transaction.
 */
long double
logbl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes logf() within a transaction.
 */
float
logf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes logl() within a transaction.
 */
long double
logl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes lrint() within a transaction.
 */
long
lrint_tx(double x);

PICOTM_NOTHROW
/**
 * Executes lrintf() within a transaction.
 */
long
lrintf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes lrintl() within a transaction.
 */
long
lrintl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes lround() within a transaction.
 */
long
lround_tx(double x);

PICOTM_NOTHROW
/**
 * Executes lroundf() within a transaction.
 */
long
lroundf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes lroundl() within a transaction.
 */
long
lroundl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes modf() within a transaction.
 */
double
modf_tx(double x, double* iptr);

PICOTM_NOTHROW
/**
 * Executes modff() within a transaction.
 */
float
modff_tx(float x, float* iptr);

PICOTM_NOTHROW
/**
 * Executes modfl() within a transaction.
 */
long double
modfl_tx(long double x, long double* iptr);

PICOTM_NOTHROW
/**
 * Executes nan() within a transaction.
 */
double
nan_tx(const char* tagp);

PICOTM_NOTHROW
/**
 * Executes nanf() within a transaction.
 */
float
nanf_tx(const char* tagp);

PICOTM_NOTHROW
/**
 * Executes nanl() within a transaction.
 */
long double
nanl_tx(const char* tagp);

PICOTM_NOTHROW
/**
 * Executes nearbyint() within a transaction.
 */
double
nearbyint_tx(double x);

PICOTM_NOTHROW
/**
 * Executes nearbyintf() within a transaction.
 */
float
nearbyintf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes nearbyintl() within a transaction.
 */
long double
nearbyintl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes nextafter() within a transaction.
 */
double
nextafter_tx(double x, double y);

PICOTM_NOTHROW
/**
 * Executes nearafterf() within a transaction.
 */
float
nextafterf_tx(float x, float y);

PICOTM_NOTHROW
/**
 * Executes nearafterl() within a transaction.
 */
long double
nextafterl_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * Executes nexttoward() within a transaction.
 */
double
nexttoward_tx(double x, long double y);

PICOTM_NOTHROW
/**
 * Executes nexttowardf() within a transaction.
 */
float
nexttowardf_tx(float x, long double y);

PICOTM_NOTHROW
/**
 * Executes nexttowardl() within a transaction.
 */
long double
nexttowardl_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * Executes pow() within a transaction.
 */
double
pow_tx(double x, double y);

PICOTM_NOTHROW
/**
 * Executes powf() within a transaction.
 */
float
powf_tx(float x, float y);

PICOTM_NOTHROW
/**
 * Executes powl() within a transaction.
 */
long double
powl_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * Executes remainder() within a transaction.
 */
double
remainder_tx(double x, double y);

PICOTM_NOTHROW
/**
 * Executes remainderf() within a transaction.
 */
float
remainderf_tx(float x, float y);

PICOTM_NOTHROW
/**
 * Executes remainderl() within a transaction.
 */
long double
remainderl_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * Executes remquo() within a transaction.
 */
double
remquo_tx(double x, double y, int* quo);

PICOTM_NOTHROW
/**
 * Executes remquof() within a transaction.
 */
float
remquof_tx(float x, float y, int* quo);

PICOTM_NOTHROW
/**
 * Executes remquol() within a transaction.
 */
long double
remquol_tx(long double x, long double y, int* quo);

PICOTM_NOTHROW
/**
 * Executes rint() within a transaction.
 */
double
rint_tx(double x);

PICOTM_NOTHROW
/**
 * Executes rintl() within a transaction.
 */
float
rintf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes rintl() within a transaction.
 */
long double
rintl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes round() within a transaction.
 */
double
round_tx(double x);

PICOTM_NOTHROW
/**
 * Executes roundf() within a transaction.
 */
float
roundf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes roundl() within a transaction.
 */
long double
roundl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes scalbln() within a transaction.
 */
double
scalbln_tx(double x, long n);

PICOTM_NOTHROW
/**
 * Executes scalblnf() within a transaction.
 */
float
scalblnf_tx(float x, long n);

PICOTM_NOTHROW
/**
 * Executes scalblnl() within a transaction.
 */
long double
scalblnl_tx(long double x, long n);

PICOTM_NOTHROW
/**
 * Executes scalbn() within a transaction.
 */
double
scalbn_tx(double x, int n);

PICOTM_NOTHROW
/**
 * Executes scalbnf() within a transaction.
 */
float
scalbnf_tx(float x, int n);

PICOTM_NOTHROW
/**
 * Executes scalbnl() within a transaction.
 */
long double
scalbnl_tx(long double x, int n);

PICOTM_NOTHROW
/**
 * Executes sin() within a transaction.
 */
double
sin_tx(double x);

PICOTM_NOTHROW
/**
 * Executes sinf() within a transaction.
 */
float
sinf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes sinh() within a transaction.
 */
double
sinh_tx(double x);

PICOTM_NOTHROW
/**
 * Executes sinhf() within a transaction.
 */
float
sinhf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes sinhl() within a transaction.
 */
long double
sinhl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes sinl() within a transaction.
 */
long double
sinl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes sqrt() within a transaction.
 */
double
sqrt_tx(double x);

PICOTM_NOTHROW
/**
 * Executes sqrtf() within a transaction.
 */
float
sqrtf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes sqrtl() within a transaction.
 */
long double
sqrtl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes tan() within a transaction.
 */
double
tan_tx(double x);

PICOTM_NOTHROW
/**
 * Executes tanf() within a transaction.
 */
float
tanf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes tanh() within a transaction.
 */
double
tanh_tx(double x);

PICOTM_NOTHROW
/**
 * Executes tanhf() within a transaction.
 */
float
tanhf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes tanhl() within a transaction.
 */
long double
tanhl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes tanl() within a transaction.
 */
long double
tanl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes tgamma() within a transaction.
 */
double
tgamma_tx(double x);

PICOTM_NOTHROW
/**
 * Executes tgammaf() within a transaction.
 */
float
tgammaf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes tgammal() within a transaction.
 */
long double
tgammal_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes trunc() within a transaction.
 */
double
trunc_tx(double x);

PICOTM_NOTHROW
/**
 * Executes truncf() within a transaction.
 */
float
truncf_tx(float x);

PICOTM_NOTHROW
/**
 * Executes truncl() within a transaction.
 */
long double
truncl_tx(long double x);

PICOTM_NOTHROW
/**
 * Executes y0() within a transaction.
 */
double
y0_tx(double x);

PICOTM_NOTHROW
/**
 * Executes y1() within a transaction.
 */
double
y1_tx(double x);

PICOTM_NOTHROW
/**
 * Executes yn() within a transaction.
 */
double
yn_tx(int n, double x);

PICOTM_END_DECLS
