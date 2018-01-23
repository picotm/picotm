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

/**
 * \brief Creates a complex number if type `double _Complex`
 * \param   r   The real part.
 * \param   i   The imaginary part.
 * \returns The complex number (r + i * I).
 */
static double _Complex
cnum(double r, double i)
{
    return r + i * _Complex_I;
}

/**
 * \brief Creates a complex number if type `float _Complex`
 * \param   r   The real part.
 * \param   i   The imaginary part.
 * \returns The complex number (r + i * I).
 */
static float _Complex
cnumf(float r, float i)
{
    return r + i * _Complex_I;
}

/**
 * \brief Creates a complex number if type `long double _Complex`
 * \param   r   The real part.
 * \param   i   The imaginary part.
 * \returns The complex number (r + i * I).
 */
static long double _Complex
cnuml(long double r, long double i)
{
    return r + i * _Complex_I;
}

/*
 * Test interfaces of <picotm/complex.h>
 */

TEST_SUCCESS(double,      cabs,  cnum(tid, tid))
TEST_SUCCESS(float,       cabsf, cnumf(tid, tid))
TEST_SUCCESS(long double, cabsl, cnuml(tid, tid))
TEST_SUCCESS(double complex, cacos,   cnum(1.0 / (tid + 1), tid))
TEST_SUCCESS(float complex,  cacosf,  cnumf(1.0 / (tid + 1), tid))
TEST_SUCCESS(double complex, cacosh,  cnum(tid + 1, tid))
TEST_SUCCESS(float complex,  cacoshf, cnumf(tid + 1, tid))
#if defined(PICOTM_LIBM_HAVE_CACOSHL) && PICOTM_LIBM_HAVE_CACOSHL
TEST_SUCCESS(long double complex, cacoshl, cnuml(tid + 1, tid))
#else
TEST_SUCCESS_SKIP(cacoshl)
#endif
#if defined(PICOTM_LIBM_HAVE_CACOSL) && PICOTM_LIBM_HAVE_CACOSL
TEST_SUCCESS(long double complex, cacosl,  cnuml(1.0 / (tid + 1), tid))
#else
TEST_SUCCESS_SKIP(cacosl)
#endif
TEST_SUCCESS(double,      carg,  cnum(tid, tid))
TEST_SUCCESS(float,       cargf, cnumf(tid, tid))
TEST_SUCCESS(long double, cargl, cnuml(tid, tid))
TEST_SUCCESS(double complex, casin,   cnum(1.0 / (tid + 1), tid))
TEST_SUCCESS(float complex,  casinf,  cnumf(1.0 / (tid + 1), tid))
TEST_SUCCESS(double complex, casinh,  cnum(1.0 / (tid + 1), tid))
TEST_SUCCESS(float complex,  casinhf, cnumf(1.0 / (tid + 1), tid))
#if defined(PICOTM_LIBM_HAVE_CASINHL) && PICOTM_LIBM_HAVE_CASINHL
TEST_SUCCESS(long double complex, casinhl, cnuml(1.0 / (tid + 1), tid))
#else
TEST_SUCCESS_SKIP(casinhl)
#endif
#if defined(PICOTM_LIBM_HAVE_CASINL) && PICOTM_LIBM_HAVE_CASINL
TEST_SUCCESS(long double complex, casinl,  cnuml(1.0 / (tid + 1), tid))
#else
TEST_SUCCESS_SKIP(casinl)
#endif
TEST_SUCCESS(double complex, catan,   cnum(1.0 / (tid + 2), tid))
TEST_SUCCESS(float complex,  catanf,  cnumf(1.0 / (tid + 2), tid))
TEST_SUCCESS(double complex, catanh,  cnum(1.0 / (tid + 2), tid))
TEST_SUCCESS(float complex,  catanhf, cnumf(1.0 / (tid + 2), tid))
#if defined(PICOTM_LIBM_HAVE_CATANHL) && PICOTM_LIBM_HAVE_CATANHL
TEST_SUCCESS(long double complex, catanhl, cnuml(1.0 / (tid + 2), tid))
#else
TEST_SUCCESS_SKIP(catanhl)
#endif
#if defined(PICOTM_LIBM_HAVE_CATANL) && PICOTM_LIBM_HAVE_CATANL
TEST_SUCCESS(long double complex, catanl,  cnuml(1.0 / (tid + 2), tid))
#else
TEST_SUCCESS_SKIP(catanl)
#endif
TEST_SUCCESS(double complex, ccos,   cnum(1.0 / (tid + 1), tid))
TEST_SUCCESS(float complex,  ccosf,  cnumf(1.0 / (tid + 1), tid))
TEST_SUCCESS(double complex, ccosh,  cnum(tid + 1, tid))
TEST_SUCCESS(float complex,  ccoshf, cnumf(tid + 1, tid))
#if defined(PICOTM_LIBM_HAVE_CCOSHL) && PICOTM_LIBM_HAVE_CCOSHL
TEST_SUCCESS(long double complex, ccoshl, cnuml(tid + 1, tid))
#else
TEST_SUCCESS_SKIP(ccoshl)
#endif
#if defined(PICOTM_LIBM_HAVE_CCOSL) && PICOTM_LIBM_HAVE_CCOSL
TEST_SUCCESS(long double complex, ccosl,  cnuml(1.0 / (tid + 1), tid))
#else
TEST_SUCCESS_SKIP(ccosl)
#endif
TEST_SUCCESS(double complex, cexp,  cnum(tid, tid))
TEST_SUCCESS(float complex,  cexpf, cnumf(tid, tid))
#if defined(PICOTM_LIBM_HAVE_CEXPL) && PICOTM_LIBM_HAVE_CEXPL
TEST_SUCCESS(long double complex, cexpl, cnuml(tid, tid))
#else
TEST_SUCCESS_SKIP(cexpl)
#endif
TEST_SUCCESS(double,      cimag,  cnum(tid, tid))
TEST_SUCCESS(float,       cimagf, cnumf(tid, tid))
TEST_SUCCESS(long double, cimagl, cnuml(tid, tid))
#if defined(PICOTM_LIBM_HAVE_CLOG) && PICOTM_LIBM_HAVE_CLOG
TEST_SUCCESS(double complex, clog, cnum(0.5 + tid, tid))
#else
TEST_SUCCESS_SKIP(clog)
#endif
#if defined(PICOTM_LIBM_HAVE_CLOGF) && PICOTM_LIBM_HAVE_CLOGF
TEST_SUCCESS(float complex,  clogf, cnumf(0.5 + tid, tid))
#else
TEST_SUCCESS_SKIP(clogf)
#endif
#if defined(PICOTM_LIBM_HAVE_CLOGL) && PICOTM_LIBM_HAVE_CLOGL
TEST_SUCCESS(long double complex, clogl, cnuml(0.5 + tid, tid))
#else
TEST_SUCCESS_SKIP(clogl)
#endif
TEST_SUCCESS(double complex,      conj,  cnum(tid, tid))
TEST_SUCCESS(float complex,       conjf, cnumf(tid, tid))
TEST_SUCCESS(long double complex, conjl, cnuml(tid, tid))
#if defined(PICOTM_LIBM_HAVE_CPOW) && PICOTM_LIBM_HAVE_CPOW
TEST_SUCCESS(double complex, cpow, cnum(tid, tid + 1), cnum(tid, tid))
#else
TEST_SUCCESS_SKIP(cpow)
#endif
#if defined(PICOTM_LIBM_HAVE_CPOWF) && PICOTM_LIBM_HAVE_CPOWF
TEST_SUCCESS(float complex, cpowf, cnumf(tid, tid + 1), cnumf(tid, tid))
#else
TEST_SUCCESS_SKIP(cpowf)
#endif
#if defined(PICOTM_LIBM_HAVE_CPOWL) && PICOTM_LIBM_HAVE_CPOWL
TEST_SUCCESS(long double complex, cpowl, cnuml(tid, tid + 1), cnuml(tid, tid))
#else
TEST_SUCCESS_SKIP(cpowl)
#endif
TEST_SUCCESS(double complex,      cproj,  cnum(tid, tid))
TEST_SUCCESS(float complex,       cprojf, cnumf(tid, tid))
TEST_SUCCESS(long double complex, cprojl, cnuml(tid, tid))
TEST_SUCCESS(double,      creal,  cnum(tid, tid))
TEST_SUCCESS(float,       crealf, cnumf(tid, tid))
TEST_SUCCESS(long double, creall, cnuml(tid, tid))
TEST_SUCCESS(double complex,      csin,   cnum(tid, tid))
TEST_SUCCESS(float complex,       csinf,  cnumf(tid, tid))
TEST_SUCCESS(double complex,      csinh,  cnum(tid, tid))
TEST_SUCCESS(float complex,       csinhf, cnumf(tid, tid))
#if defined(PICOTM_LIBM_HAVE_CSINHL) && PICOTM_LIBM_HAVE_CSINHL
TEST_SUCCESS(long double complex, csinhl, cnuml(tid, tid))
#else
TEST_SUCCESS_SKIP(csinhl)
#endif
#if defined(PICOTM_LIBM_HAVE_CSINL) && PICOTM_LIBM_HAVE_CSINL
TEST_SUCCESS(long double complex, csinl, cnuml(tid, tid))
#else
TEST_SUCCESS_SKIP(csinl)
#endif
TEST_SUCCESS(double complex,      csqrt,  cnum(tid + 1, tid + 1))
TEST_SUCCESS(float complex,       csqrtf, cnumf(tid + 1, tid + 1))
TEST_SUCCESS(long double complex, csqrtl, cnuml(tid + 1, tid + 1))
TEST_SUCCESS(double complex,      ctan,   cnum(M_PI_2 / (tid + 1), tid))
TEST_SUCCESS(float complex,       ctanf,  cnumf(M_PI_2 / (tid + 1), tid))
TEST_SUCCESS(double complex,      ctanh,  cnum(M_PI_2 / (tid + 1), tid))
TEST_SUCCESS(float complex,       ctanhf, cnumf(M_PI_2 / (tid + 1), tid))
#if defined(PICOTM_LIBM_HAVE_CTANHL) && PICOTM_LIBM_HAVE_CTANHL
TEST_SUCCESS(long double complex, ctanhl, cnuml(M_PI_2 / (tid + 1), tid))
#else
TEST_SUCCESS_SKIP(ctanhl)
#endif
#if defined(PICOTM_LIBM_HAVE_CTANL) && PICOTM_LIBM_HAVE_CTANL
TEST_SUCCESS(long double complex, ctanl, cnuml(M_PI_2 / (tid + 1), tid))
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
