/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

ceuta_hdrl(#ifndef TANGER_STM_STD_MATH_H);
ceuta_hdrl(#define TANGER_STM_STD_MATH_H);
ceuta_hdrl(#include <math.h>);

#include <math.h>

/* Trigonometric functions
 */

ceuta_pure(double,      sin,  sin,  double x);
ceuta_pure(double,      cos,  cos,  double x);
ceuta_pure(double,      tan,  tan,  double x);
ceuta_pure(float,       sinf, sinf, float x);
ceuta_pure(float,       cosf, cosf, float x);
ceuta_pure(float,       tanf, tanf, float x);
ceuta_pure(long double, sinl, sinl, long double x);
ceuta_pure(long double, cosl, cosl, long double x);
ceuta_pure(long double, tanl, tanl, long double x);

/* Inverse trigonometric functions
 */

ceuta_pure(double,      asin,   asin,   double x);
ceuta_pure(double,      acos,   acos,   double x);
ceuta_pure(double,      atan,   atan,   double x);
ceuta_pure(float,       asinf,  asinf,  float x);
ceuta_pure(float,       acosf,  acosf,  float x);
ceuta_pure(float,       atanf,  atanf,  float x);
ceuta_pure(long double, asinl,  asinl,  long double x);
ceuta_pure(long double, acosl,  acisl,  long double x);
ceuta_pure(long double, atanl,  atanl,  long double x);
ceuta_pure(double,      atan2,  atan2,  double y, double x);
ceuta_pure(float,       atan2f, atan2f, float y, float x);
ceuta_pure(long double, atan2l, atan2l, long double y, long double x);

/* Exponents and logarithms
 */

ceuta_pure(double,      exp,     exp,     double x);
ceuta_pure(double,      exp2,    exp2,    double x);
ceuta_pure(float,       expf,    expf,    float x);
ceuta_pure(float,       exp2f,   exp2f,   float x);
ceuta_pure(long double, expl,    expl,    long double x);
ceuta_pure(long double, exp2l,   exp2l,   long double x);
ceuta_pure(double,      log,     log,     double x);
ceuta_pure(double,      log2,    log2,    double x);
ceuta_pure(double,      log10,   log10,   double x);
ceuta_pure(float,       logf,    logf,    float x);
ceuta_pure(float,       log2f,   log2f,   float x);
ceuta_pure(float,       log10f,  log10f,  float x);
ceuta_pure(long double, logl,    logl,    long double x);
ceuta_pure(long double, log2l,   log2l,   long double x);
ceuta_pure(long double, log10l,  log10l,  long double x);
ceuta_pure(double,      logb,    logb,    double x);
ceuta_pure(float,       logbf,   logbf,   float x);
ceuta_pure(long double, logbl,   logbl,   long double x);
ceuta_pure(double,      ilogb,   ilogb,   double x);
ceuta_pure(float,       ilogbf,  ilogbf,  float x);
ceuta_pure(long double, ilogbl,  ilogbl,  long double x);
ceuta_pure(double,      pow,     pow,     double x, double y);
ceuta_pure(float,       powf,    powf,    float x, float y);
ceuta_pure(long double, powl,    powl,    long double x, long double y);
ceuta_pure(double,      sqrt,    sqrt,    double x);
ceuta_pure(float,       sqrtf,   sqrtf,   float x);
ceuta_pure(long double, sqrtl,   sqrtl,   long double x);
ceuta_pure(double,      cbrt,    cbrt,    double x);
ceuta_pure(float,       cbrtf,   cbrtf,   float x);
ceuta_pure(long double, cbrtl,   cbrtl,   long double x);
ceuta_pure(double,      hypot,   hypot,   double x, double y);
ceuta_pure(float,       hypotf,  hoypotf, float x, float y);
ceuta_pure(long double, hypotl,  hypotl,  long double x, long double y);
ceuta_pure(double,      expm1,   expm1,   double x);
ceuta_pure(float,       expm1fm, expm1fm, float x);
ceuta_pure(long double, expm1l,  expm1l,  long double x);
ceuta_pure(double,      log1p,   log1p,   double x);
ceuta_pure(float,       log1pf,  log1pf,  float x);
ceuta_pure(long double, log1pl,  log1pl,  long double x);

/* Hyperbolic functions
 */

ceuta_pure(double,      sinh,   sinh,   double x);
ceuta_pure(float,       sinhf,  sinhf,  float x);
ceuta_pure(long double, sinhl,  sinhl,  long double x);
ceuta_pure(double,      cosh,   cosh,   double x);
ceuta_pure(float,       coshf,  coshf,  float x);
ceuta_pure(long double, coshl,  coshl,  long double x);
ceuta_pure(double,      tanh,   tanh,   double x);
ceuta_pure(float,       tanhf,  tanhf,  float x);
ceuta_pure(long double, tanhl,  tanhl,  long double x);
ceuta_pure(double,      asinh,  asinh,  double x);
ceuta_pure(float,       asinhf, asinhf, float x);
ceuta_pure(long double, asinhl, asinhl, long double x);
ceuta_pure(double,      acosh,  acosh,  double x);
ceuta_pure(float,       acoshf, acoshf, float x);
ceuta_pure(long double, acoshl, acoshl, long double x);
ceuta_pure(double,      atanh,  atanh,  double x);
ceuta_pure(float,       atanhf, atanhf, float x);
ceuta_pure(long double, atanhl, atanhl, long double x);

/* Special functions
 */

ceuta_pure(double,      erf,     erf,                double x);
ceuta_pure(float,       erff,    erff,               float x);
ceuta_pure(long double, erfl,    erfl,               long double x);
ceuta_pure(double,      erfc,    erfc,               double x);
ceuta_pure(float,       erfcf,   erfcf,              float x);
ceuta_pure(long double, erfcl,   erfcl,              long double x);
ceuta_decl(double,      lgamma,  lgamma,  double x);
ceuta_decl(float,       lgammaf, lgammaf, float x);
ceuta_decl(long double, lgammal, lgammal, long double x);
ceuta_pure(double,      tgamma,  tgamma,             double x);
ceuta_pure(float,       tgammaf, tgammaf,            float x);
ceuta_pure(long double, tgammal, tgammal,            long double x);
ceuta_pure(double,      j0,      j0,                 double x);
ceuta_pure(double,      j1,      j1,                 double x);
ceuta_pure(double,      jn,      jn,                 int n, double x);
ceuta_pure(double,      y0,      y0,                 double x);
ceuta_pure(double,      y1,      y1,                 double x);
ceuta_pure(double,      yn,      yn,                 int n, double x);

double
tanger_stm_std_lgamma(double x)
{
    int sign;

    double res = lgamma_r(x, &sign);

    tanger_stm_tx_t *tx = tanger_stm_get_tx();
    assert(tx);

    tanger_stm_store32(tx, (uint32_t*)&signgam, sign);

    return res;
}

float
tanger_stm_std_lgammaf(float x)
{
    int sign;

    float res = lgamma_r(x, &sign);

    tanger_stm_tx_t *tx = tanger_stm_get_tx();
    assert(tx);

    tanger_stm_store32(tx, (uint32_t*)&signgam, sign);

    return res;
}

long double
tanger_stm_std_lgammal(long double x)
{
    int sign;

    long double res = lgamma_r(x, &sign);

    tanger_stm_tx_t *tx = tanger_stm_get_tx();
    assert(tx);

    tanger_stm_store32(tx, (uint32_t*)&signgam, sign);

    return res;
}

ceuta_hdrl(#endif);

