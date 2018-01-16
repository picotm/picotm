/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include <math.h>
#include <picotm/complex.h>
#include <picotm/picotm-tm.h>
#include "ptr.h"
#include "test.h"
#include "testhlp.h"
#include "testmacro.h"

/*
 * Test interfaces of <picotm/complex.h>
 */

TEST_SUCCESS(double,      cabs,  tid + tid * I)
TEST_SUCCESS(float,       cabsf, tid + tid * I)
TEST_SUCCESS(long double, cabsl, tid + tid * I)
TEST_SUCCESS(double complex, cacos,   1.0 / (tid + 1) + tid * I)
TEST_SUCCESS(float complex,  cacosf,  1.0 / (tid + 1) + tid * I)
TEST_SUCCESS(double complex, cacosh,  tid + 1 + tid * I)
TEST_SUCCESS(float complex,  cacoshf, tid + 1 + tid * I)
#if defined(PICOTM_LIBM_HAVE_CACOSHL) && PICOTM_LIBM_HAVE_CACOSHL
TEST_SUCCESS(long double complex, cacoshl, tid + 1 + tid * I)
#else
TEST_SUCCESS_SKIP(cacoshl)
#endif
#if defined(PICOTM_LIBM_HAVE_CACOSL) && PICOTM_LIBM_HAVE_CACOSL
TEST_SUCCESS(long double complex, cacosl,  1.0 / (tid + 1) + tid * I)
#else
TEST_SUCCESS_SKIP(cacosl)
#endif
TEST_SUCCESS(double,      carg,  tid + tid * I)
TEST_SUCCESS(float,       cargf, tid + tid * I)
TEST_SUCCESS(long double, cargl, tid + tid * I)
TEST_SUCCESS(double complex, casin,   1.0 / (tid + 1) + tid * I)
TEST_SUCCESS(float complex,  casinf,  1.0 / (tid + 1) + tid * I)
TEST_SUCCESS(double complex, casinh,  1.0 / (tid + 1) + tid * I)
TEST_SUCCESS(float complex,  casinhf, 1.0 / (tid + 1) + tid * I)
#if defined(PICOTM_LIBM_HAVE_CASINHL) && PICOTM_LIBM_HAVE_CASINHL
TEST_SUCCESS(long double complex, casinhl, 1.0 / (tid + 1) + tid * I)
#else
TEST_SUCCESS_SKIP(casinhl)
#endif
#if defined(PICOTM_LIBM_HAVE_CASINL) && PICOTM_LIBM_HAVE_CASINL
TEST_SUCCESS(long complex, casinl,  1.0 / (tid + 1) + tid * I)
#else
TEST_SUCCESS_SKIP(casinl)
#endif
TEST_SUCCESS(double complex, catan,   1.0 / (tid + 2) + tid * I)
TEST_SUCCESS(float complex,  catanf,  1.0 / (tid + 2) + tid * I)
TEST_SUCCESS(double complex, catanh,  1.0 / (tid + 2) + tid * I)
TEST_SUCCESS(float complex,  catanhf, 1.0 / (tid + 2) + tid * I)
#if defined(PICOTM_LIBM_HAVE_CATANHL) && PICOTM_LIBM_HAVE_CATANHL
TEST_SUCCESS(long double complex, catanhl, 1.0 / (tid + 2) + tid * I)
#else
TEST_SUCCESS_SKIP(catanhl)
#endif
#if defined(PICOTM_LIBM_HAVE_CATANL) && PICOTM_LIBM_HAVE_CATANL
TEST_SUCCESS(long complex, catanl,  1.0 / (tid + 2) + tid * I)
#else
TEST_SUCCESS_SKIP(catanl)
#endif
TEST_SUCCESS(double complex, ccos,   1.0 / (tid + 1) + tid * I)
TEST_SUCCESS(float complex,  ccosf,  1.0 / (tid + 1) + tid * I)
TEST_SUCCESS(double complex, ccosh,  tid + 1 + tid * I)
TEST_SUCCESS(float complex,  ccoshf, tid + 1 + tid * I)
#if defined(PICOTM_LIBM_HAVE_CCOSHL) && PICOTM_LIBM_HAVE_CCOSHL
TEST_SUCCESS(long double complex, ccoshl, tid + 1 + tid * I)
#else
TEST_SUCCESS_SKIP(ccoshl)
#endif
#if defined(PICOTM_LIBM_HAVE_CCOSL) && PICOTM_LIBM_HAVE_CCOSL
TEST_SUCCESS(long double complex, ccosl,  1.0 / (tid + 1) + tid * I)
#else
TEST_SUCCESS_SKIP(ccosl)
#endif
TEST_SUCCESS(double complex, cexp,  tid + tid * I)
TEST_SUCCESS(float complex,  cexpf, tid + tid * I)
#if defined(PICOTM_LIBM_HAVE_CEXPL) && PICOTM_LIBM_HAVE_CEXPL
TEST_SUCCESS(long double complex, cexpl, tid + tid * I)
#else
TEST_SUCCESS_SKIP(cexpl)
#endif
TEST_SUCCESS(double,      cimag,  tid + tid * I)
TEST_SUCCESS(float,       cimagf, tid + tid * I)
TEST_SUCCESS(long double, cimagl, tid + tid * I)
#if defined(PICOTM_LIBM_HAVE_CLOG) && PICOTM_LIBM_HAVE_CLOG
TEST_SUCCESS(double complex, clog,  0.5 + tid + tid * I)
#else
TEST_SUCCESS_SKIP(clog)
#endif
#if defined(PICOTM_LIBM_HAVE_CLOGF) && PICOTM_LIBM_HAVE_CLOGF
TEST_SUCCESS(float complex,  clogf, 0.5 + tid + tid * I)
#else
TEST_SUCCESS_SKIP(clogf)
#endif
#if defined(PICOTM_LIBM_HAVE_CLOGL) && PICOTM_LIBM_HAVE_CLOGL
TEST_SUCCESS(long double complex, clogl, 0.5 + tid + tid * I)
#else
TEST_SUCCESS_SKIP(clogl)
#endif
TEST_SUCCESS(double complex,      conj,  tid + tid * I)
TEST_SUCCESS(float complex,       conjf, tid + tid * I)
TEST_SUCCESS(long double complex, conjl, tid + tid * I)
#if defined(PICOTM_LIBM_HAVE_CPOW) && PICOTM_LIBM_HAVE_CPOW
TEST_SUCCESS(double complex, cpow,  tid + (tid + 1) * I, tid + tid * I)
#else
TEST_SUCCESS_SKIP(cpow)
#endif
#if defined(PICOTM_LIBM_HAVE_CPOWF) && PICOTM_LIBM_HAVE_CPOWF
TEST_SUCCESS(float complex, cpowf, tid + (tid + 1) * I, tid + tid * I)
#else
TEST_SUCCESS_SKIP(cpowf)
#endif
#if defined(PICOTM_LIBM_HAVE_CPOWL) && PICOTM_LIBM_HAVE_CPOWL
TEST_SUCCESS(long double complex, cpowl, tid + (tid + 1) * I, tid + tid * I)
#else
TEST_SUCCESS_SKIP(cpowl)
#endif
TEST_SUCCESS(double complex,      cproj,  tid + tid * I)
TEST_SUCCESS(float complex,       cprojf, tid + tid * I)
TEST_SUCCESS(long double complex, cprojl, tid + tid * I)
TEST_SUCCESS(double,      creal,  tid + tid * I)
TEST_SUCCESS(float,       crealf, tid + tid * I)
TEST_SUCCESS(long double, creall, tid + tid * I)
TEST_SUCCESS(double complex,      csin,   tid + tid * I)
TEST_SUCCESS(float complex,       csinf,  tid + tid * I)
TEST_SUCCESS(double complex,      csinh,  tid + tid * I)
TEST_SUCCESS(float complex,       csinhf, tid + tid * I)
#if defined(PICOTM_LIBM_HAVE_CSINHL) && PICOTM_LIBM_HAVE_CSINHL
TEST_SUCCESS(long double complex, csinhl, tid + tid * I)
#else
TEST_SUCCESS_SKIP(csinhl)
#endif
#if defined(PICOTM_LIBM_HAVE_CSINL) && PICOTM_LIBM_HAVE_CSINL
TEST_SUCCESS(long complex, csinl,  tid + tid * I)
#else
TEST_SUCCESS_SKIP(csinl)
#endif
TEST_SUCCESS(double complex,      csqrt,  (tid + 1) + (tid + 1) * I)
TEST_SUCCESS(float complex,       csqrtf, (tid + 1) + (tid + 1) * I)
TEST_SUCCESS(long double complex, csqrtl, (tid + 1) + (tid + 1) * I)
TEST_SUCCESS(double complex,      ctan,   (M_PI_2 / (tid + 1)) + tid * I)
TEST_SUCCESS(float complex,       ctanf,  (M_PI_2 / (tid + 1)) + tid * I)
TEST_SUCCESS(double complex,      ctanh,  (M_PI_2 / (tid + 1)) + tid * I)
TEST_SUCCESS(float complex,       ctanhf, (M_PI_2 / (tid + 1)) + tid * I)
#if defined(PICOTM_LIBM_HAVE_CTANHL) && PICOTM_LIBM_HAVE_CTANHL
TEST_SUCCESS(long double complex, ctanhl, (M_PI_2 / (tid + 1)) + tid * I)
#else
TEST_SUCCESS_SKIP(ctanhl)
#endif
#if defined(PICOTM_LIBM_HAVE_CTANL) && PICOTM_LIBM_HAVE_CTANL
TEST_SUCCESS(long complex, ctanl,  (M_PI_2 / (tid + 1)) + tid * I)
#else
TEST_SUCCESS_SKIP(ctanl)
#endif

static const struct test_func test[] = {
    TEST_SUCCESS_FUNC(cabs),
    TEST_SUCCESS_FUNC(cabsf),
    TEST_SUCCESS_FUNC(cabsl),
    TEST_SUCCESS_FUNC(cacos),
    TEST_SUCCESS_FUNC(cacosf),
    TEST_SUCCESS_FUNC(cacosh),
    TEST_SUCCESS_FUNC(cacoshf),
    TEST_SUCCESS_FUNC(cacoshl),
    TEST_SUCCESS_FUNC(cacosl),
    TEST_SUCCESS_FUNC(carg),
    TEST_SUCCESS_FUNC(cargf),
    TEST_SUCCESS_FUNC(cargl),
    TEST_SUCCESS_FUNC(casin),
    TEST_SUCCESS_FUNC(casinf),
    TEST_SUCCESS_FUNC(casinh),
    TEST_SUCCESS_FUNC(casinhf),
    TEST_SUCCESS_FUNC(casinhl),
    TEST_SUCCESS_FUNC(casinl),
    TEST_SUCCESS_FUNC(catan),
    TEST_SUCCESS_FUNC(catanf),
    TEST_SUCCESS_FUNC(catanhf),
    TEST_SUCCESS_FUNC(catanhl),
    TEST_SUCCESS_FUNC(catanh),
    TEST_SUCCESS_FUNC(catanl),
    TEST_SUCCESS_FUNC(ccos),
    TEST_SUCCESS_FUNC(ccosf),
    TEST_SUCCESS_FUNC(ccosh),
    TEST_SUCCESS_FUNC(ccoshf),
    TEST_SUCCESS_FUNC(ccoshl),
    TEST_SUCCESS_FUNC(ccosl),
    TEST_SUCCESS_FUNC(cexp),
    TEST_SUCCESS_FUNC(cexpf),
    TEST_SUCCESS_FUNC(cexpl),
    TEST_SUCCESS_FUNC(cimag),
    TEST_SUCCESS_FUNC(cimagf),
    TEST_SUCCESS_FUNC(cimagl),
    TEST_SUCCESS_FUNC(clog),
    TEST_SUCCESS_FUNC(clogf),
    TEST_SUCCESS_FUNC(clogl),
    TEST_SUCCESS_FUNC(conj),
    TEST_SUCCESS_FUNC(conjf),
    TEST_SUCCESS_FUNC(conjl),
    TEST_SUCCESS_FUNC(cpow),
    TEST_SUCCESS_FUNC(cpowf),
    TEST_SUCCESS_FUNC(cpowl),
    TEST_SUCCESS_FUNC(cproj),
    TEST_SUCCESS_FUNC(cprojf),
    TEST_SUCCESS_FUNC(cprojl),
    TEST_SUCCESS_FUNC(creal),
    TEST_SUCCESS_FUNC(crealf),
    TEST_SUCCESS_FUNC(creall),
    TEST_SUCCESS_FUNC(csin),
    TEST_SUCCESS_FUNC(csinf),
    TEST_SUCCESS_FUNC(csinh),
    TEST_SUCCESS_FUNC(csinhf),
    TEST_SUCCESS_FUNC(csinhl),
    TEST_SUCCESS_FUNC(csinl),
    TEST_SUCCESS_FUNC(csqrt),
    TEST_SUCCESS_FUNC(csqrtf),
    TEST_SUCCESS_FUNC(csqrtl),
    TEST_SUCCESS_FUNC(ctan),
    TEST_SUCCESS_FUNC(ctanf),
    TEST_SUCCESS_FUNC(ctanhf),
    TEST_SUCCESS_FUNC(ctanhl),
    TEST_SUCCESS_FUNC(ctanh),
    TEST_SUCCESS_FUNC(ctanl)
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
