/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2019  Thomas Zimmermann <contact@tzimmermann.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "picotm/math.h"
#include "picotm/picotm-tm.h"
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <stdbool.h>
#include "ptr.h"
#include "sysenv.h"
#include "test.h"
#include "testhlp.h"
#include "testmacro.h"

/*
 * Thread-safe wrappers
 */

static double wrap_lgamma(double x)
{
    int sign;
    return lgamma_r(x, &sign);
}
#define wrap_lgamma_tx(x)   lgamma_tx(x)

static float wrap_lgammaf(float x)
{
    int sign;
    return lgammaf_r(x, &sign);
}
#define wrap_lgammaf_tx(x)  lgammaf_tx(x)

static long double wrap_lgammal(long double x)
{
    int sign;
    return lgammal_r(x, &sign);
}
#define wrap_lgammal_tx(x)  lgammal_tx(x)

/*
 * Test interfaces of <picotm/math.h>
 */

/* Returns -1 for odd numbers, +1 for even numbers. */
static long
negsignodd(unsigned long x)
{
    return ((long)(x % 2)) * 2 - 1;
}

TEST_SUCCESS(double,      acos,   1.0 / (tid + 1))
TEST_SUCCESS(float,       acosf,  1.0 / (tid + 1))
TEST_SUCCESS(double,      acosh,  tid + 1)
TEST_SUCCESS(float,       acoshf, tid + 1)
TEST_SUCCESS(long double, acoshl, tid + 1)
TEST_SUCCESS(long double, acosl,  1.0 / (tid + 1))
TEST_SUCCESS(double,      asin,   1.0 / (tid + 1))
TEST_SUCCESS(float,       asinf,  1.0 / (tid + 1))
TEST_SUCCESS(double,      asinh,  1.0 / (tid + 1))
TEST_SUCCESS(float,       asinhf, 1.0 / (tid + 1))
TEST_SUCCESS(long double, asinhl, 1.0 / (tid + 1))
TEST_SUCCESS(long,        asinl,  1.0 / (tid + 1))
TEST_SUCCESS(double,      atan,   1.0 / (tid + 1))
TEST_SUCCESS(double,      atan2,  1.0 / (tid + 1), 1.0 / (tid + 1))
TEST_SUCCESS(float,       atan2f, 1.0 / (tid + 1), 1.0 / (tid + 1))
TEST_SUCCESS(long double, atan2l, 1.0 / (tid + 1), 1.0 / (tid + 1))
TEST_SUCCESS(float,       atanf,  1.0 / (tid + 1))
TEST_SUCCESS(double,      atanh,  1.0 / (tid + 2))
TEST_SUCCESS(float,       atanhf, 1.0 / (tid + 2))
TEST_SUCCESS(long double, atanhl, 1.0 / (tid + 2))
TEST_SUCCESS(long,        atanl,  1.0 / (tid + 1))
TEST_SUCCESS(double,      cbrt,  1.0)
TEST_SUCCESS(float,       cbrtf, 1.0)
TEST_SUCCESS(long double, cbrtl, 1.0)
TEST_SUCCESS(double,      ceil,  1.0)
TEST_SUCCESS(float,       ceilf, 1.0)
TEST_SUCCESS(long double, ceill, 1.0)
TEST_SUCCESS(double,      copysign,  1.0, -1.0)
TEST_SUCCESS(float,       copysignf, 1.0, -1.0)
TEST_SUCCESS_IF(!is_cygwin(), long double, copysignl, 1.0, -1.0)
TEST_SUCCESS(double,      cos,   1.0 / (tid + 1))
TEST_SUCCESS(float,       cosf,  1.0 / (tid + 1))
TEST_SUCCESS(double,      cosh,  tid + 1)
TEST_SUCCESS(float,       coshf, tid + 1)
TEST_SUCCESS(long double, coshl, tid + 1)
TEST_SUCCESS(long double, cosl,  1.0 / (tid + 1))
TEST_SUCCESS(double,      erf,   1.0 / (tid + 1))
TEST_SUCCESS(double,      erfc,  1.0 / (tid + 1))
TEST_SUCCESS(float,       erfcf, 1.0 / (tid + 1))
TEST_SUCCESS(long double, erfcl, 1.0 / (tid + 1))
TEST_SUCCESS(float,       erff,  1.0 / (tid + 1))
TEST_SUCCESS(long double, erfl,  1.0 / (tid + 1))
TEST_SUCCESS(double,      exp,   tid)
TEST_SUCCESS(double,      exp2,  tid)
TEST_SUCCESS(float,       exp2f, tid)
TEST_SUCCESS(long double, exp2l, tid)
TEST_SUCCESS(float,       expf,  tid)
TEST_SUCCESS(long double, expl,  tid)
TEST_SUCCESS(double,      expm1,  tid)
TEST_SUCCESS(float,       expm1f, tid)
TEST_SUCCESS(long double, expm1l, tid)
TEST_SUCCESS(double,      fabs,  (double)tid)
TEST_SUCCESS(float,       fabsf, (float)tid)
TEST_SUCCESS(long double, fabsl, (long double)tid)
TEST_SUCCESS(double,      fdim,  1, tid)
TEST_SUCCESS(float,       fdimf, 1, tid)
TEST_SUCCESS(long double, fdiml, 1, tid)
TEST_SUCCESS(double,      floor,  0.5 + tid)
TEST_SUCCESS(float,       floorf, 0.5 + tid)
TEST_SUCCESS(long double, floorl, 0.5 + tid)
TEST_SUCCESS(double,      fma,  0.5, tid, tid)
TEST_SUCCESS(float,       fmaf, 0.5, tid, tid)
TEST_SUCCESS(long double, fmal, 0.5, tid, tid)
TEST_SUCCESS(double,      fmax,  tid, -tid)
TEST_SUCCESS(float,       fmaxf, tid, -tid)
TEST_SUCCESS(long double, fmaxl, tid, -tid)
TEST_SUCCESS(double,      fmin,  tid, -tid)
TEST_SUCCESS(float,       fminf, tid, -tid)
TEST_SUCCESS(long double, fminl, tid, -tid)
TEST_SUCCESS(double,      fmod,  tid, 3)
TEST_SUCCESS(float,       fmodf, tid, 3)
TEST_SUCCESS(long double, fmodl, tid, 3)
TEST_SUCCESS_P(double,      frexp,  int, 0.5 * tid)
TEST_SUCCESS_P(float,       frexpf, int, 0.5 * tid)
TEST_SUCCESS_P(long double, frexpl, int, 0.5 * tid)
TEST_SUCCESS(double,      hypot,  tid, -tid)
TEST_SUCCESS(float,       hypotf, tid, -tid)
TEST_SUCCESS(long double, hypotl, tid, -tid)
TEST_SUCCESS(double,      ilogb,  0.5 + tid)
TEST_SUCCESS(float,       ilogbf, 0.5 + tid)
TEST_SUCCESS(long double, ilogbl, 0.5 + tid)
TEST_SUCCESS(double, j0, tid)
TEST_SUCCESS(double, j1, tid)
TEST_SUCCESS(double, jn, tid, tid)
TEST_SUCCESS(double,      ldexp,  tid, tid)
TEST_SUCCESS(float,       ldexpf, tid, tid)
TEST_SUCCESS(long double, ldexpl, tid, tid)
TEST_SUCCESS(double,      wrap_lgamma,  1 + tid)
TEST_SUCCESS(float,       wrap_lgammaf, 1 + tid)
TEST_SUCCESS(long double, wrap_lgammal, 1 + tid)
TEST_SUCCESS(long long,   llrint,  0.75 * tid)
TEST_SUCCESS(long long,   llrintf, 0.75 * tid)
TEST_SUCCESS(long long,   llrintl, 0.75 * tid)
TEST_SUCCESS(long long,   llround,  0.75 * tid)
TEST_SUCCESS(long long,   llroundf, 0.75 * tid)
TEST_SUCCESS(long long,   llroundl, 0.75 * tid)
TEST_SUCCESS(double,      log,   0.5 + tid)
TEST_SUCCESS(double,      log10,  0.5 + tid)
TEST_SUCCESS(float,       log10f, 0.5 + tid)
TEST_SUCCESS(long double, log10l, 0.5 + tid)
TEST_SUCCESS(double,      log1p,  0.5 + tid)
TEST_SUCCESS(float,       log1pf, 0.5 + tid)
TEST_SUCCESS(long double, log1pl, 0.5 + tid)
TEST_SUCCESS(double,      log2,  0.5 + tid)
TEST_SUCCESS(float,       log2f, 0.5 + tid)
TEST_SUCCESS(long double, log2l, 0.5 + tid)
TEST_SUCCESS(double,      logb,  0.5 + tid)
TEST_SUCCESS(float,       logbf, 0.5 + tid)
TEST_SUCCESS(long double, logbl, 0.5 + tid)
TEST_SUCCESS(float,       logf, 0.5 + tid)
TEST_SUCCESS(long double, logl, 0.5 + tid)
TEST_SUCCESS(long, lrint,  0.75 * tid)
TEST_SUCCESS(long, lrintf, 0.75 * tid)
TEST_SUCCESS(long, lrintl, 0.75 * tid)
TEST_SUCCESS(long, lround,  0.75 * tid)
TEST_SUCCESS(long, lroundf, 0.75 * tid)
TEST_SUCCESS(long, lroundl, 0.75 * tid)
TEST_SUCCESS_P(double,      modf,  double,      0.5 * tid)
TEST_SUCCESS_P(float,       modff, float,       0.5 * tid)
TEST_SUCCESS_P(long double, modfl, long double, 0.5 * tid)
TEST_SUCCESS_NAN(double,      nan,  "NaN")
TEST_SUCCESS_NAN(float,       nanf, "NaN")
TEST_SUCCESS_NAN(long double, nanl, "NaN")
TEST_SUCCESS(double,      nearbyint,  0.5 * tid)
TEST_SUCCESS(float,       nearbyintf, 0.5 * tid)
TEST_SUCCESS(long double, nearbyintl, 0.5 * tid)
TEST_SUCCESS(double,      nextafter,  0.5 * tid, tid * negsignodd(tid))
TEST_SUCCESS(float,       nextafterf, 0.5 * tid, tid * negsignodd(tid))
TEST_SUCCESS(long double, nextafterl, 0.5 * tid, tid * negsignodd(tid))
TEST_SUCCESS(double,      nexttoward,  0.5 * tid, tid * negsignodd(tid))
TEST_SUCCESS(float,       nexttowardf, 0.5 * tid, tid * negsignodd(tid))
TEST_SUCCESS(long double, nexttowardl, 0.5 * tid, tid * negsignodd(tid))
TEST_SUCCESS(double,      pow,  tid, tid)
TEST_SUCCESS(float,       powf, tid, tid)
TEST_SUCCESS(long double, powl, tid, tid)
TEST_SUCCESS(double,      remainder,  tid, 3)
TEST_SUCCESS(float,       remainderf, tid, 3)
TEST_SUCCESS(long double, remainderl, tid, 3)
TEST_SUCCESS_P(double,      remquo,  int, tid, 3)
TEST_SUCCESS_P(float,       remquof, int, tid, 3)
TEST_SUCCESS_P(long double, remquol, int, tid, 3)
TEST_SUCCESS(double,      rint,  0.75 * tid)
TEST_SUCCESS(float,       rintf, 0.75 * tid)
TEST_SUCCESS(long double, rintl, 0.75 * tid)
TEST_SUCCESS(double,      round,  0.75 * tid)
TEST_SUCCESS(float,       roundf, 0.75 * tid)
TEST_SUCCESS(long double, roundl, 0.75 * tid)
TEST_SUCCESS(double,      scalbln,  tid, tid)
TEST_SUCCESS(float,       scalblnf, tid, tid)
TEST_SUCCESS(long double, scalblnl, tid, tid)
TEST_SUCCESS(double,      scalbn,  tid, tid)
TEST_SUCCESS(float,       scalbnf, tid, tid)
TEST_SUCCESS(long double, scalbnl, tid, tid)
TEST_SUCCESS(double,      sin,   tid)
TEST_SUCCESS(float,       sinf,  tid)
TEST_SUCCESS(double,      sinh,  tid)
TEST_SUCCESS(float,       sinhf, tid)
TEST_SUCCESS(long double, sinhl, tid)
TEST_SUCCESS(long,        sinl,  tid)
TEST_SUCCESS(double,      sqrt,  tid)
TEST_SUCCESS(float,       sqrtf, tid)
TEST_SUCCESS(long double, sqrtl, tid)
TEST_SUCCESS(double,      tan,   M_PI_2 / (tid + 1))
TEST_SUCCESS(float,       tanf,  M_PI_2 / (tid + 1))
TEST_SUCCESS(double,      tanh,  M_PI_2 / (tid + 1))
TEST_SUCCESS(float,       tanhf, M_PI_2 / (tid + 1))
TEST_SUCCESS(long double, tanhl, M_PI_2 / (tid + 1))
TEST_SUCCESS(long,        tanl,  M_PI_2 / (tid + 1))
TEST_SUCCESS(double,      tgamma,  1 + tid)
TEST_SUCCESS(float,       tgammaf, 1 + tid)
TEST_SUCCESS(long double, tgammal, 1 + tid)
TEST_SUCCESS(double,      trunc,  0.75 * tid)
TEST_SUCCESS(float,       truncf, 0.75 * tid)
TEST_SUCCESS(long double, truncl, 0.75 * tid)
TEST_SUCCESS(double, y0, 1 + tid)
TEST_SUCCESS(double, y1, 1 + tid)
TEST_SUCCESS(double, yn, tid, 1 + tid)

TEST_E_ERRNO(   EDOM, acos, INFINITY)
TEST_E_ERRNO(   EDOM, acosf, INFINITY)
TEST_E_ERRNO(   EDOM, acosh, 0)
TEST_E_ERRNO(   EDOM, acoshf, 0)
TEST_E_ERRNO(   EDOM, acoshl, 0)
TEST_E_ERRNO_IF(!is_cygwin(),
                EDOM, acosl, INFINITY)
TEST_E_ERRNO(   EDOM, asin, INFINITY)
TEST_E_ERRNO(   EDOM, asinf, INFINITY)
TEST_E_ERRNO_IF(!is_cygwin(),
                EDOM, asinl, INFINITY)
TEST_E_ERRNO_IF(!is_cygwin(),
                ERANGE, atanh, 1)
TEST_E_ERRNO_IF(!is_cygwin(),
                ERANGE, atanhf, 1)
TEST_E_ERRNO(   ERANGE, atanhl, 1)
TEST_E_ERRNO_IF(!is_freebsd(/* 11.1 */),
                ERANGE, cosh, DBL_MAX)
TEST_E_ERRNO_IF(!is_freebsd(/* 11.1 */),
                ERANGE, coshf, FLT_MAX)
TEST_E_ERRNO_IF(!is_valgrind(/* 3.13 */),
                ERANGE, coshl, LDBL_MAX)
TEST_E_ERRNO_IF(!is_cygwin() && !is_macos(/* 10.6 */),
                ERANGE, erfc, DBL_MAX)
TEST_E_ERRNO_IF(!is_cygwin() && !is_macos(/* 10.6 */),
                ERANGE, erfcf, FLT_MAX)
TEST_E_ERRNO_IF(!is_macos(/* 10.6 */) && !is_valgrind(/* 3.13 */),
                ERANGE, erfcl, LDBL_MAX)
TEST_E_ERRNO(   ERANGE, exp, DBL_MAX)
TEST_E_ERRNO(   ERANGE, exp2, DBL_MAX)
TEST_E_ERRNO(   ERANGE, exp2f, FLT_MAX)
TEST_E_ERRNO_IF(!is_cygwin() && !is_valgrind(/* 3.13 */),
                ERANGE, exp2l, LDBL_MAX)
TEST_E_ERRNO(   ERANGE, expf, FLT_MAX)
TEST_E_ERRNO_IF(!is_valgrind(/* 3.13 */),
                ERANGE, expl, LDBL_MAX)
TEST_E_ERRNO_IF(!is_cygwin(),
                ERANGE, expm1, DBL_MAX)
TEST_E_ERRNO_IF(!is_cygwin(),
                ERANGE, expm1f, FLT_MAX)
TEST_E_ERRNO_IF(!is_valgrind(/* 3.13 */),
                ERANGE, expm1l, LDBL_MAX)
TEST_E_ERRNO_IF(!is_cygwin(),
                ERANGE, fdim, DBL_MAX, -DBL_MAX)
TEST_E_ERRNO_IF(!is_cygwin(),
                ERANGE, fdimf, FLT_MAX, -FLT_MAX)
TEST_E_ERRNO_IF(!is_valgrind(/* 3.13 */),
                ERANGE, fdiml, LDBL_MAX, -LDBL_MAX)
TEST_E_ERRNO(   ERANGE, hypot, DBL_MAX, DBL_MAX)
TEST_E_ERRNO(   ERANGE, hypotf, FLT_MAX, FLT_MAX)
TEST_E_ERRNO_IF(!is_valgrind(/* 3.13 */),
                ERANGE, hypotl, LDBL_MAX, LDBL_MAX)
TEST_E_ERRNO(   ERANGE, ldexp, DBL_MAX, 1)
TEST_E_ERRNO(   ERANGE, ldexpf, FLT_MAX, 1)
TEST_E_ERRNO_IF(!is_valgrind(/* 3.13 */),
                ERANGE, ldexpl, LDBL_MAX, 1)
TEST_E_ERRNO_IF(!is_cygwin(),
                ERANGE, lgamma, -(long)tid)
TEST_E_ERRNO_IF(!is_cygwin(),
                ERANGE, lgammaf, -(long)tid)
TEST_E_ERRNO_IF(!is_cygwin() && !is_valgrind(/* 3.13 */),
                ERANGE, lgammal, -(long)tid)
TEST_E_ERRNO(   EDOM, log, -(long)(tid + 1))
TEST_E_ERRNO(   EDOM, log10, -(long)(tid + 1))
TEST_E_ERRNO(   EDOM, log10f, -(long)(tid + 1))
TEST_E_ERRNO_IF(!is_cygwin(),
                EDOM, log10l, -(long)(tid + 1))
TEST_E_ERRNO_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                EDOM, log1p, -(long)(tid + 2))
TEST_E_ERRNO_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                EDOM, log1pf, -(long)(tid + 2))
TEST_E_ERRNO_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                EDOM, log1pl, -(long)(tid + 2))
TEST_E_ERRNO(   EDOM, log2, -(long)(tid + 1))
TEST_E_ERRNO(   EDOM, log2f, -(long)(tid + 1))
TEST_E_ERRNO_IF(!is_cygwin(),
                EDOM, log2l, -(long)(tid + 1))
TEST_E_ERRNO(   EDOM, logf, -(long)(tid + 1))
TEST_E_ERRNO(   EDOM, logl, -(long)(tid + 1))
TEST_E_ERRNO(   ERANGE, log, 0)
TEST_E_ERRNO(   ERANGE, log10, 0)
TEST_E_ERRNO(   ERANGE, log10f, 0)
TEST_E_ERRNO_IF(!is_cygwin(),
                ERANGE, log10l, 0)
TEST_E_ERRNO_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                ERANGE, log1p, -1)
TEST_E_ERRNO_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                ERANGE, log1pf, -1)
TEST_E_ERRNO_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                ERANGE, log1pl, -1)
TEST_E_ERRNO(   ERANGE, log2, 0)
TEST_E_ERRNO(   ERANGE, log2f, 0)
TEST_E_ERRNO_IF(!is_cygwin(),
                ERANGE, log2l, 0)
TEST_E_ERRNO(   ERANGE, logf, 0)
TEST_E_ERRNO(   ERANGE, logl, 0)
TEST_E_ERRNO_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                ERANGE, nexttoward, DBL_MAX, LDBL_MAX)
TEST_E_ERRNO_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                ERANGE, nexttowardf, FLT_MAX, LDBL_MAX)
/* Can we make this overflow?
TEST_E_ERRNO_IF(!is_cygwin() && !is_linux(),
                ERANGE, nexttowardl, LDBL_MAX, LDBL_MAX)*/
TEST_E_ERRNO_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                ERANGE, scalbln, DBL_MAX, tid + 2)
TEST_E_ERRNO_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                ERANGE, scalblnf, FLT_MAX, tid + 2)
TEST_E_ERRNO_IF(!is_cygwin() && !is_valgrind(/* 3.13 */),
                ERANGE, scalblnl, LDBL_MAX, tid + 2)
TEST_E_ERRNO_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                ERANGE, scalbn, DBL_MAX, tid + 2)
TEST_E_ERRNO_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                ERANGE, scalbnf, FLT_MAX, tid + 2)
TEST_E_ERRNO_IF(!is_cygwin() && !is_valgrind(/* 3.13 */),
                ERANGE, scalbnl, LDBL_MAX, tid + 2)
TEST_E_ERRNO(   ERANGE, sinh, DBL_MAX)
TEST_E_ERRNO(   ERANGE, sinhf, FLT_MAX)
TEST_E_ERRNO_IF(!is_valgrind(/* 3.13 */),
                ERANGE, sinhl, LDBL_MAX)
TEST_E_ERRNO(   EDOM, sqrt, -(long)(tid + 1))
TEST_E_ERRNO(   EDOM, sqrtf, -(long)(tid + 1))
TEST_E_ERRNO(   EDOM, sqrtl, -(long)(tid + 1))
TEST_E_ERRNO(   ERANGE, tgamma, DBL_MAX)
TEST_E_ERRNO(   ERANGE, tgammaf, FLT_MAX)
TEST_E_ERRNO_IF(!is_valgrind(/* 3.13 */),
                ERANGE, tgammal, LDBL_MAX)
TEST_E_ERRNO(   EDOM, y0, -(long)(tid + 1))
TEST_E_ERRNO(   EDOM, y1, -(long)(tid + 1))
TEST_E_ERRNO(   EDOM, yn, tid + 1, -(long)(tid + 1))
TEST_E_ERRNO_IF(!is_cygwin(),
                ERANGE, y0, 0)
TEST_E_ERRNO_IF(!is_cygwin(),
                ERANGE, y1, 0)
TEST_E_ERRNO_IF(!is_cygwin(),
                ERANGE, yn, tid + 1, 0)

TEST_E_EXCPT(   FE_INVALID, acos, INFINITY)
TEST_E_EXCPT(   FE_INVALID, acosf, INFINITY)
TEST_E_EXCPT(   FE_INVALID, acosh, 0)
TEST_E_EXCPT(   FE_INVALID, acoshf, 0)
TEST_E_EXCPT(   FE_INVALID, acoshl, 0)
TEST_E_EXCPT_IF(!is_cygwin(),
                FE_INVALID, acosl, INFINITY)
TEST_E_EXCPT(   FE_INVALID, asin, INFINITY)
TEST_E_EXCPT(   FE_INVALID, asinf, INFINITY)
TEST_E_EXCPT_IF(!is_cygwin(),
                FE_INVALID, asinl, INFINITY)
TEST_E_EXCPT_IF(!is_cygwin(),
                FE_DIVBYZERO, atanh, 1)
TEST_E_EXCPT_IF(!is_cygwin(),
                FE_DIVBYZERO, atanhf, 1)
TEST_E_EXCPT(   FE_DIVBYZERO, atanhl, 1)
TEST_E_EXCPT_IF(!is_freebsd(/* 11.1 */),
                FE_OVERFLOW, cosh, DBL_MAX)
TEST_E_EXCPT_IF(!is_freebsd(/* 11.1 */),
                FE_OVERFLOW, coshf, FLT_MAX)
TEST_E_EXCPT_IF(!is_valgrind(/* 3.13 */),
                FE_OVERFLOW, coshl, LDBL_MAX)
TEST_E_EXCPT_IF(!is_cygwin() && !is_macos(/* 10.6 */),
                FE_UNDERFLOW, erfc, DBL_MAX)
TEST_E_EXCPT_IF(!is_cygwin() && !is_macos(/* 10.6 */),
                FE_UNDERFLOW, erfcf, FLT_MAX)
TEST_E_EXCPT_IF(!is_macos(/* 10.6 */) && !is_valgrind(/* 3.13 */),
                FE_UNDERFLOW, erfcl, LDBL_MAX)
TEST_E_EXCPT(   FE_OVERFLOW, exp, DBL_MAX)
TEST_E_EXCPT(   FE_OVERFLOW, exp2, DBL_MAX)
TEST_E_EXCPT(   FE_OVERFLOW, exp2f, FLT_MAX)
TEST_E_EXCPT_IF(!is_cygwin() && !is_valgrind(/* 3.13 */),
                FE_OVERFLOW, exp2l, LDBL_MAX)
TEST_E_EXCPT(   FE_OVERFLOW, expf, FLT_MAX)
TEST_E_EXCPT_IF(!is_valgrind(/* 3.13 */),
                FE_OVERFLOW, expl, LDBL_MAX)
TEST_E_EXCPT_IF(!is_cygwin(),
                FE_OVERFLOW, expm1, DBL_MAX)
TEST_E_EXCPT_IF(!is_cygwin(),
                FE_OVERFLOW, expm1f, FLT_MAX)
TEST_E_EXCPT_IF(!is_valgrind(/* 3.13 */),
                FE_OVERFLOW, expm1l, LDBL_MAX)
TEST_E_EXCPT_IF(!is_cygwin(),
                FE_OVERFLOW, fdim, DBL_MAX, -DBL_MAX)
TEST_E_EXCPT_IF(!is_cygwin(),
                FE_OVERFLOW, fdimf, FLT_MAX, -FLT_MAX)
TEST_E_EXCPT_IF(!is_valgrind(/* 3.13 */),
                FE_OVERFLOW, fdiml, LDBL_MAX, -LDBL_MAX)
TEST_E_EXCPT(   FE_OVERFLOW, hypot, DBL_MAX, DBL_MAX)
TEST_E_EXCPT(   FE_OVERFLOW, hypotf, FLT_MAX, FLT_MAX)
TEST_E_EXCPT_IF(!is_valgrind(/* 3.13 */),
                FE_OVERFLOW, hypotl, LDBL_MAX, LDBL_MAX)
TEST_E_EXCPT(   FE_OVERFLOW, ldexp, DBL_MAX, 1)
TEST_E_EXCPT(   FE_OVERFLOW, ldexpf, FLT_MAX, 1)
TEST_E_EXCPT_IF(!is_valgrind(/* 3.13 */),
                FE_OVERFLOW, ldexpl, LDBL_MAX, 1)
TEST_E_EXCPT_IF(!is_cygwin(),
                FE_DIVBYZERO, lgamma, -(long)tid)
TEST_E_EXCPT_IF(!is_cygwin(),
                FE_DIVBYZERO, lgammaf, -(long)tid)
TEST_E_EXCPT_IF(!is_cygwin() && !is_valgrind(/* 3.13 */),
                FE_DIVBYZERO, lgammal, -(long)tid)
TEST_E_EXCPT(   FE_OVERFLOW, lgamma, DBL_MAX)
TEST_E_EXCPT(   FE_OVERFLOW, lgammaf, FLT_MAX)
TEST_E_EXCPT_IF(!is_valgrind(/* 3.13 */),
                FE_OVERFLOW, lgammal, LDBL_MAX)
TEST_E_EXCPT(   FE_INVALID, log, -(long)(tid + 1))
TEST_E_EXCPT(   FE_INVALID, log10, -(long)(tid + 1))
TEST_E_EXCPT(   FE_INVALID, log10f, -(long)(tid + 1))
TEST_E_EXCPT_IF(!is_cygwin(),
                FE_INVALID, log10l, -(long)(tid + 1))
TEST_E_EXCPT_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                FE_INVALID, log1p, -(long)(tid + 2))
TEST_E_EXCPT_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                FE_INVALID, log1pf, -(long)(tid + 2))
TEST_E_EXCPT_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                FE_INVALID, log1pl, -(long)(tid + 2))
TEST_E_EXCPT(   FE_INVALID, log2, -(long)(tid + 1))
TEST_E_EXCPT(   FE_INVALID, log2f, -(long)(tid + 1))
TEST_E_EXCPT_IF(!is_cygwin(),
                FE_INVALID, log2l, -(long)(tid + 1))
TEST_E_EXCPT(   FE_INVALID, logf, -(long)(tid + 1))
TEST_E_EXCPT(   FE_INVALID, logl, -(long)(tid + 1))
TEST_E_EXCPT(   FE_DIVBYZERO, log, 0)
TEST_E_EXCPT(   FE_DIVBYZERO, log10, 0)
TEST_E_EXCPT(   FE_DIVBYZERO, log10f, 0)
TEST_E_EXCPT_IF(!is_cygwin(),
                FE_DIVBYZERO, log10l, 0)
TEST_E_EXCPT_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                FE_DIVBYZERO, log1p, -1)
TEST_E_EXCPT_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                FE_DIVBYZERO, log1pf, -1)
TEST_E_EXCPT_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */) ,
                FE_DIVBYZERO, log1pl, -1)
TEST_E_EXCPT(   FE_DIVBYZERO, log2, 0)
TEST_E_EXCPT(   FE_DIVBYZERO, log2f, 0)
TEST_E_EXCPT_IF(!is_cygwin(),
                FE_DIVBYZERO, log2l, 0)
TEST_E_EXCPT(   FE_DIVBYZERO, logf, 0)
TEST_E_EXCPT(   FE_DIVBYZERO, logl, 0)
TEST_E_EXCPT_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                FE_OVERFLOW, nexttoward, DBL_MAX, LDBL_MAX)
TEST_E_EXCPT_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                FE_OVERFLOW, nexttowardf, FLT_MAX, LDBL_MAX)
/* Can we make this overflow?
TEST_E_EXCPT_IF(!is_cygwin() && !is_linux(),
                FE_OVERFLOW, nexttowardl, LDBL_MAX, LDBL_MAX)*/
TEST_E_EXCPT_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                FE_OVERFLOW, scalbln, DBL_MAX, tid + 2)
TEST_E_EXCPT_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                FE_OVERFLOW, scalblnf, FLT_MAX, tid + 2)
TEST_E_EXCPT_IF(!is_cygwin() && !is_valgrind(/* 3.13 */),
                FE_OVERFLOW, scalblnl, LDBL_MAX, tid + 2)
TEST_E_EXCPT_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                FE_OVERFLOW, scalbn, DBL_MAX, tid + 2)
TEST_E_EXCPT_IF(!is_cygwin() && !is_linux(/* Ubuntu 14.4 */),
                FE_OVERFLOW, scalbnf, FLT_MAX, tid + 2)
TEST_E_EXCPT_IF(!is_cygwin() && !is_valgrind(/* 3.13 */),
                FE_OVERFLOW, scalbnl, LDBL_MAX, tid + 2)
TEST_E_EXCPT(   FE_OVERFLOW, sinh, DBL_MAX)
TEST_E_EXCPT(   FE_OVERFLOW, sinhf, FLT_MAX)
TEST_E_EXCPT_IF(!is_valgrind(/* 3.13 */),
                FE_OVERFLOW, sinhl, LDBL_MAX)
TEST_E_EXCPT(   FE_INVALID, sqrt, -(long)(tid + 1))
TEST_E_EXCPT(   FE_INVALID, sqrtf, -(long)(tid + 1))
TEST_E_EXCPT(   FE_INVALID, sqrtl, -(long)(tid + 1))
TEST_E_EXCPT(   FE_OVERFLOW, tgamma, DBL_MAX)
TEST_E_EXCPT(   FE_OVERFLOW, tgammaf, FLT_MAX)
TEST_E_EXCPT_IF(!is_valgrind(/* 3.13 */),
                FE_OVERFLOW, tgammal, LDBL_MAX)
TEST_E_EXCPT(   FE_INVALID, y0, -(long)(tid + 1))
TEST_E_EXCPT(   FE_INVALID, y1, -(long)(tid + 1))
TEST_E_EXCPT(   FE_INVALID, yn, tid + 1, -(long)(tid + 1))
TEST_E_EXCPT_IF(!is_cygwin(),
                FE_DIVBYZERO, y0, 0)
TEST_E_EXCPT_IF(!is_cygwin(),
                FE_DIVBYZERO, y1, 0)
TEST_E_EXCPT_IF(!is_cygwin() && !is_valgrind(/* 3.13 */),
                FE_DIVBYZERO, yn, tid + 1, 0)

static const struct test_func test[] = {
    TEST_SUCCESS_FUNC(acos),
    TEST_SUCCESS_FUNC(acosf),
    TEST_SUCCESS_FUNC(acosh),
    TEST_SUCCESS_FUNC(acoshf),
    TEST_SUCCESS_FUNC(acoshl),
    TEST_SUCCESS_FUNC(acosl),
    TEST_SUCCESS_FUNC(asin),
    TEST_SUCCESS_FUNC(asinf),
    TEST_SUCCESS_FUNC(asinh),
    TEST_SUCCESS_FUNC(asinhf),
    TEST_SUCCESS_FUNC(asinhl),
    TEST_SUCCESS_FUNC(asinl),
    TEST_SUCCESS_FUNC(atan),
    TEST_SUCCESS_FUNC(atan2),
    TEST_SUCCESS_FUNC(atan2f),
    TEST_SUCCESS_FUNC(atan2l),
    TEST_SUCCESS_FUNC(atanf),
    TEST_SUCCESS_FUNC(atanhf),
    TEST_SUCCESS_FUNC(atanhl),
    TEST_SUCCESS_FUNC(atanh),
    TEST_SUCCESS_FUNC(atanl),
    TEST_SUCCESS_FUNC(cbrt),
    TEST_SUCCESS_FUNC(cbrtf),
    TEST_SUCCESS_FUNC(cbrtl),
    TEST_SUCCESS_FUNC(ceil),
    TEST_SUCCESS_FUNC(ceill),
    TEST_SUCCESS_FUNC(ceilf),
    TEST_SUCCESS_FUNC(copysign),
    TEST_SUCCESS_FUNC(copysignf),
    TEST_SUCCESS_FUNC(copysignl),
    TEST_SUCCESS_FUNC(cos),
    TEST_SUCCESS_FUNC(cosf),
    TEST_SUCCESS_FUNC(cosh),
    TEST_SUCCESS_FUNC(coshf),
    TEST_SUCCESS_FUNC(coshl),
    TEST_SUCCESS_FUNC(cosl),
    TEST_SUCCESS_FUNC(erf),
    TEST_SUCCESS_FUNC(erfc),
    TEST_SUCCESS_FUNC(erfcf),
    TEST_SUCCESS_FUNC(erfcl),
    TEST_SUCCESS_FUNC(erff),
    TEST_SUCCESS_FUNC(erfl),
    TEST_SUCCESS_FUNC(exp),
    TEST_SUCCESS_FUNC(exp2),
    TEST_SUCCESS_FUNC(exp2f),
    TEST_SUCCESS_FUNC(exp2l),
    TEST_SUCCESS_FUNC(expf),
    TEST_SUCCESS_FUNC(expl),
    TEST_SUCCESS_FUNC(expm1),
    TEST_SUCCESS_FUNC(expm1f),
    TEST_SUCCESS_FUNC(expm1l),
    TEST_SUCCESS_FUNC(fabs),
    TEST_SUCCESS_FUNC(fabsf),
    TEST_SUCCESS_FUNC(fabsl),
    TEST_SUCCESS_FUNC(fdim),
    TEST_SUCCESS_FUNC(fdimf),
    TEST_SUCCESS_FUNC(fdiml),
    TEST_SUCCESS_FUNC(floor),
    TEST_SUCCESS_FUNC(floorf),
    TEST_SUCCESS_FUNC(floorl),
    TEST_SUCCESS_FUNC(fma),
    TEST_SUCCESS_FUNC(fmaf),
    TEST_SUCCESS_FUNC(fmal),
    TEST_SUCCESS_FUNC(fmax),
    TEST_SUCCESS_FUNC(fmaxf),
    TEST_SUCCESS_FUNC(fmaxl),
    TEST_SUCCESS_FUNC(fmin),
    TEST_SUCCESS_FUNC(fminf),
    TEST_SUCCESS_FUNC(fminl),
    TEST_SUCCESS_FUNC(fmod),
    TEST_SUCCESS_FUNC(fmodf),
    TEST_SUCCESS_FUNC(fmodl),
    TEST_SUCCESS_FUNC(frexp),
    TEST_SUCCESS_FUNC(frexpf),
    TEST_SUCCESS_FUNC(frexpl),
    TEST_SUCCESS_FUNC(hypot),
    TEST_SUCCESS_FUNC(hypotf),
    TEST_SUCCESS_FUNC(hypotl),
    TEST_SUCCESS_FUNC(ilogb),
    TEST_SUCCESS_FUNC(ilogbf),
    TEST_SUCCESS_FUNC(ilogbl),
    TEST_SUCCESS_FUNC(j0),
    TEST_SUCCESS_FUNC(j1),
    TEST_SUCCESS_FUNC(jn),
    TEST_SUCCESS_FUNC(ldexp),
    TEST_SUCCESS_FUNC(ldexpf),
    TEST_SUCCESS_FUNC(ldexpl),
    TEST_SUCCESS_FUNC(wrap_lgamma),
    TEST_SUCCESS_FUNC(wrap_lgammaf),
    TEST_SUCCESS_FUNC(wrap_lgammal),
    TEST_SUCCESS_FUNC(llrint),
    TEST_SUCCESS_FUNC(llrintf),
    TEST_SUCCESS_FUNC(llrintl),
    TEST_SUCCESS_FUNC(llround),
    TEST_SUCCESS_FUNC(llroundf),
    TEST_SUCCESS_FUNC(llroundl),
    TEST_SUCCESS_FUNC(log),
    TEST_SUCCESS_FUNC(log10),
    TEST_SUCCESS_FUNC(log10f),
    TEST_SUCCESS_FUNC(log10l),
    TEST_SUCCESS_FUNC(log1p),
    TEST_SUCCESS_FUNC(log1pf),
    TEST_SUCCESS_FUNC(log1pl),
    TEST_SUCCESS_FUNC(log2),
    TEST_SUCCESS_FUNC(log2f),
    TEST_SUCCESS_FUNC(log2l),
    TEST_SUCCESS_FUNC(logb),
    TEST_SUCCESS_FUNC(logbf),
    TEST_SUCCESS_FUNC(logbl),
    TEST_SUCCESS_FUNC(logf),
    TEST_SUCCESS_FUNC(logl),
    TEST_SUCCESS_FUNC(lrint),
    TEST_SUCCESS_FUNC(lrintf),
    TEST_SUCCESS_FUNC(lrintl),
    TEST_SUCCESS_FUNC(lround),
    TEST_SUCCESS_FUNC(lroundf),
    TEST_SUCCESS_FUNC(lroundl),
    TEST_SUCCESS_FUNC(modf),
    TEST_SUCCESS_FUNC(modff),
    TEST_SUCCESS_FUNC(modfl),
    TEST_SUCCESS_FUNC(nan),
    TEST_SUCCESS_FUNC(nanf),
    TEST_SUCCESS_FUNC(nanl),
    TEST_SUCCESS_FUNC(nearbyint),
    TEST_SUCCESS_FUNC(nearbyintf),
    TEST_SUCCESS_FUNC(nearbyintl),
    TEST_SUCCESS_FUNC(nextafter),
    TEST_SUCCESS_FUNC(nextafterf),
    TEST_SUCCESS_FUNC(nextafterl),
    TEST_SUCCESS_FUNC(nexttoward),
    TEST_SUCCESS_FUNC(nexttowardf),
    TEST_SUCCESS_FUNC(nexttowardl),
    TEST_SUCCESS_FUNC(pow),
    TEST_SUCCESS_FUNC(powf),
    TEST_SUCCESS_FUNC(powl),
    TEST_SUCCESS_FUNC(remainder),
    TEST_SUCCESS_FUNC(remainderf),
    TEST_SUCCESS_FUNC(remainderl),
    TEST_SUCCESS_FUNC(remquo),
    TEST_SUCCESS_FUNC(remquof),
    TEST_SUCCESS_FUNC(remquol),
    TEST_SUCCESS_FUNC(rint),
    TEST_SUCCESS_FUNC(rintf),
    TEST_SUCCESS_FUNC(rintl),
    TEST_SUCCESS_FUNC(round),
    TEST_SUCCESS_FUNC(roundf),
    TEST_SUCCESS_FUNC(roundl),
    TEST_SUCCESS_FUNC(scalbln),
    TEST_SUCCESS_FUNC(scalblnf),
    TEST_SUCCESS_FUNC(scalblnl),
    TEST_SUCCESS_FUNC(scalbn),
    TEST_SUCCESS_FUNC(scalbnf),
    TEST_SUCCESS_FUNC(scalbnl),
    TEST_SUCCESS_FUNC(sin),
    TEST_SUCCESS_FUNC(sinf),
    TEST_SUCCESS_FUNC(sinh),
    TEST_SUCCESS_FUNC(sinhf),
    TEST_SUCCESS_FUNC(sinhl),
    TEST_SUCCESS_FUNC(sinl),
    TEST_SUCCESS_FUNC(sqrt),
    TEST_SUCCESS_FUNC(sqrtf),
    TEST_SUCCESS_FUNC(sqrtl),
    TEST_SUCCESS_FUNC(tan),
    TEST_SUCCESS_FUNC(tanf),
    TEST_SUCCESS_FUNC(tanhf),
    TEST_SUCCESS_FUNC(tanhl),
    TEST_SUCCESS_FUNC(tanh),
    TEST_SUCCESS_FUNC(tanl),
    TEST_SUCCESS_FUNC(tgamma),
    TEST_SUCCESS_FUNC(tgammaf),
    TEST_SUCCESS_FUNC(tgammal),
    TEST_SUCCESS_FUNC(trunc),
    TEST_SUCCESS_FUNC(truncf),
    TEST_SUCCESS_FUNC(truncl),
    TEST_SUCCESS_FUNC(y0),
    TEST_SUCCESS_FUNC(y1),
    TEST_SUCCESS_FUNC(yn),

    /* Test errno */
    TEST_E_ERRNO_FUNC(EDOM, acos),
    TEST_E_ERRNO_FUNC(EDOM, acosf),
    TEST_E_ERRNO_FUNC(EDOM, acosh),
    TEST_E_ERRNO_FUNC(EDOM, acoshf),
    TEST_E_ERRNO_FUNC(EDOM, acoshl),
    TEST_E_ERRNO_FUNC(EDOM, acosl),
    TEST_E_ERRNO_FUNC(EDOM, asin),
    TEST_E_ERRNO_FUNC(EDOM, asinf),
    TEST_E_ERRNO_FUNC(EDOM, asinl),
    TEST_E_ERRNO_FUNC(ERANGE, atanh),
    TEST_E_ERRNO_FUNC(ERANGE, atanhf),
    TEST_E_ERRNO_FUNC(ERANGE, atanhl),
    TEST_E_ERRNO_FUNC(ERANGE, cosh),
    TEST_E_ERRNO_FUNC(ERANGE, coshf),
    TEST_E_ERRNO_FUNC(ERANGE, coshl),
    TEST_E_ERRNO_FUNC(ERANGE, erfc),
    TEST_E_ERRNO_FUNC(ERANGE, erfcf),
    TEST_E_ERRNO_FUNC(ERANGE, erfcl),
    TEST_E_ERRNO_FUNC(ERANGE, exp),
    TEST_E_ERRNO_FUNC(ERANGE, exp2),
    TEST_E_ERRNO_FUNC(ERANGE, exp2f),
    TEST_E_ERRNO_FUNC(ERANGE, exp2l),
    TEST_E_ERRNO_FUNC(ERANGE, expf),
    TEST_E_ERRNO_FUNC(ERANGE, expl),
    TEST_E_ERRNO_FUNC(ERANGE, expm1),
    TEST_E_ERRNO_FUNC(ERANGE, expm1f),
    TEST_E_ERRNO_FUNC(ERANGE, expm1l),
    TEST_E_ERRNO_FUNC(ERANGE, fdim),
    TEST_E_ERRNO_FUNC(ERANGE, fdimf),
    TEST_E_ERRNO_FUNC(ERANGE, fdiml),
    TEST_E_ERRNO_FUNC(ERANGE, hypot),
    TEST_E_ERRNO_FUNC(ERANGE, hypotf),
    TEST_E_ERRNO_FUNC(ERANGE, hypotl),
    TEST_E_ERRNO_FUNC(ERANGE, ldexp),
    TEST_E_ERRNO_FUNC(ERANGE, ldexpf),
    TEST_E_ERRNO_FUNC(ERANGE, ldexpl),
    TEST_E_ERRNO_FUNC(ERANGE, lgamma),
    TEST_E_ERRNO_FUNC(ERANGE, lgammaf),
    TEST_E_ERRNO_FUNC(ERANGE, lgammal),
    TEST_E_ERRNO_FUNC(EDOM, log),
    TEST_E_ERRNO_FUNC(EDOM, log10),
    TEST_E_ERRNO_FUNC(EDOM, log10f),
    TEST_E_ERRNO_FUNC(EDOM, log10l),
    TEST_E_ERRNO_FUNC(EDOM, log1p),
    TEST_E_ERRNO_FUNC(EDOM, log1pf),
    TEST_E_ERRNO_FUNC(EDOM, log1pl),
    TEST_E_ERRNO_FUNC(EDOM, log2),
    TEST_E_ERRNO_FUNC(EDOM, log2f),
    TEST_E_ERRNO_FUNC(EDOM, log2l),
    TEST_E_ERRNO_FUNC(EDOM, logf),
    TEST_E_ERRNO_FUNC(EDOM, logl),
    TEST_E_ERRNO_FUNC(ERANGE, log),
    TEST_E_ERRNO_FUNC(ERANGE, log10),
    TEST_E_ERRNO_FUNC(ERANGE, log10f),
    TEST_E_ERRNO_FUNC(ERANGE, log10l),
    TEST_E_ERRNO_FUNC(ERANGE, log1p),
    TEST_E_ERRNO_FUNC(ERANGE, log1pf),
    TEST_E_ERRNO_FUNC(ERANGE, log1pl),
    TEST_E_ERRNO_FUNC(ERANGE, log2),
    TEST_E_ERRNO_FUNC(ERANGE, log2f),
    TEST_E_ERRNO_FUNC(ERANGE, log2l),
    TEST_E_ERRNO_FUNC(ERANGE, logf),
    TEST_E_ERRNO_FUNC(ERANGE, logl),
    TEST_E_ERRNO_FUNC(ERANGE, nexttoward),
    TEST_E_ERRNO_FUNC(ERANGE, nexttowardf),
    /*TEST_E_ERRNO_FUNC(ERANGE, nexttowardl),*/
    TEST_E_ERRNO_FUNC(ERANGE, scalbln),
    TEST_E_ERRNO_FUNC(ERANGE, scalblnf),
    TEST_E_ERRNO_FUNC(ERANGE, scalblnl),
    TEST_E_ERRNO_FUNC(ERANGE, scalbn),
    TEST_E_ERRNO_FUNC(ERANGE, scalbnf),
    TEST_E_ERRNO_FUNC(ERANGE, scalbnl),
    TEST_E_ERRNO_FUNC(ERANGE, sinh),
    TEST_E_ERRNO_FUNC(ERANGE, sinhf),
    TEST_E_ERRNO_FUNC(ERANGE, sinhl),
    TEST_E_ERRNO_FUNC(EDOM, sqrt),
    TEST_E_ERRNO_FUNC(EDOM, sqrtf),
    TEST_E_ERRNO_FUNC(EDOM, sqrtl),
    TEST_E_ERRNO_FUNC(ERANGE, tgamma),
    TEST_E_ERRNO_FUNC(ERANGE, tgammaf),
    TEST_E_ERRNO_FUNC(ERANGE, tgammal),
    TEST_E_ERRNO_FUNC(EDOM, y0),
    TEST_E_ERRNO_FUNC(EDOM, y1),
    TEST_E_ERRNO_FUNC(EDOM, yn),
    TEST_E_ERRNO_FUNC(ERANGE, y0),
    TEST_E_ERRNO_FUNC(ERANGE, y1),
    TEST_E_ERRNO_FUNC(ERANGE, yn),

    /* Test float-point exceptions */
    TEST_E_EXCPT_FUNC(FE_INVALID, acos),
    TEST_E_EXCPT_FUNC(FE_INVALID, acosf),
    TEST_E_EXCPT_FUNC(FE_INVALID, acosh),
    TEST_E_EXCPT_FUNC(FE_INVALID, acoshf),
    TEST_E_EXCPT_FUNC(FE_INVALID, acoshl),
    TEST_E_EXCPT_FUNC(FE_INVALID, acosl),
    TEST_E_EXCPT_FUNC(FE_INVALID, asin),
    TEST_E_EXCPT_FUNC(FE_INVALID, asinf),
    TEST_E_EXCPT_FUNC(FE_INVALID, asinl),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, atanh),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, atanhf),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, atanhl),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, cosh),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, coshf),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, coshl),
    TEST_E_EXCPT_FUNC(FE_UNDERFLOW, erfc),
    TEST_E_EXCPT_FUNC(FE_UNDERFLOW, erfcf),
    TEST_E_EXCPT_FUNC(FE_UNDERFLOW, erfcl),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, exp),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, exp2),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, exp2f),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, exp2l),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, expf),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, expl),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, expm1),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, expm1f),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, expm1l),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, fdim),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, fdimf),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, fdiml),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, hypot),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, hypotf),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, hypotl),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, ldexp),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, ldexpf),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, ldexpl),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, lgamma),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, lgammaf),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, lgammal),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, lgamma),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, lgammaf),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, lgammal),
    TEST_E_EXCPT_FUNC(FE_INVALID, log),
    TEST_E_EXCPT_FUNC(FE_INVALID, log10),
    TEST_E_EXCPT_FUNC(FE_INVALID, log10f),
    TEST_E_EXCPT_FUNC(FE_INVALID, log10l),
    TEST_E_EXCPT_FUNC(FE_INVALID, log1p),
    TEST_E_EXCPT_FUNC(FE_INVALID, log1pf),
    TEST_E_EXCPT_FUNC(FE_INVALID, log1pl),
    TEST_E_EXCPT_FUNC(FE_INVALID, log2),
    TEST_E_EXCPT_FUNC(FE_INVALID, log2f),
    TEST_E_EXCPT_FUNC(FE_INVALID, log2l),
    TEST_E_EXCPT_FUNC(FE_INVALID, logf),
    TEST_E_EXCPT_FUNC(FE_INVALID, logl),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, log),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, log10),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, log10f),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, log10l),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, log1p),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, log1pf),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, log1pl),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, log2),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, log2f),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, log2l),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, logf),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, logl),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, nexttoward),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, nexttowardf),
    /*TEST_E_EXCPT_FUNC(FE_OVERFLOW, nexttowardl),*/
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, scalbln),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, scalblnf),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, scalblnl),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, scalbn),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, scalbnf),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, scalbnl),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, sinh),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, sinhf),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, sinhl),
    TEST_E_EXCPT_FUNC(FE_INVALID, sqrt),
    TEST_E_EXCPT_FUNC(FE_INVALID, sqrtf),
    TEST_E_EXCPT_FUNC(FE_INVALID, sqrtl),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, tgamma),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, tgammaf),
    TEST_E_EXCPT_FUNC(FE_OVERFLOW, tgammal),
    TEST_E_EXCPT_FUNC(FE_INVALID, y0),
    TEST_E_EXCPT_FUNC(FE_INVALID, y1),
    TEST_E_EXCPT_FUNC(FE_INVALID, yn),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, y0),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, y1),
    TEST_E_EXCPT_FUNC(FE_DIVBYZERO, yn)
};

/*
 * Entry point
 */

#include "opts.h"
#include "pubapi.h"

int
main(int argc, char* argv[])
{
    return pubapi_main(argc, argv, PARSE_OPTS_STRING(), test, arraylen(test));
}
