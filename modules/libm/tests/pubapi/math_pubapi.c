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

#include <picotm/math.h>
#include <picotm/picotm-tm.h>
#include "ptr.h"
#include "test.h"
#include "testhlp.h"
#include "testmacro.h"

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
#if !defined(__CYGWIN__)
TEST_SUCCESS(long double, copysignl, 1.0, -1.0)
#else
/* Broken on Cygwin. */
TEST_SUCCESS_SKIP(copysignl)
#endif
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
TEST_SUCCESS(double,      fabs,  tid)
TEST_SUCCESS(float,       fabsf, tid)
TEST_SUCCESS(long double, fabsl, tid)
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
TEST_SUCCESS(double,      lgamma,  1 + tid)
TEST_SUCCESS(float,       lgammaf, 1 + tid)
TEST_SUCCESS(long double, lgammal, 1 + tid)
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
    TEST_SUCCESS_FUNC(lgamma),
    TEST_SUCCESS_FUNC(lgammaf),
    TEST_SUCCESS_FUNC(lgammal),
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
    TEST_SUCCESS_FUNC(yn)
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
