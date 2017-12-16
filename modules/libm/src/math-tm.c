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
 */

#include "picotm/math-tm.h"
#include <fenv.h>
#include <math.h>
#include "matherr.h"

#if defined(PICOTM_LIBM_HAVE_FREXP) && PICOTM_LIBM_HAVE_FREXP
double
frexp_tm(double num, int* exp)
{
    return frexp(num, exp);
}
#endif

#if defined(PICOTM_LIBM_HAVE_FREXPF) && PICOTM_LIBM_HAVE_FREXPF
PICOTM_EXPORT
float
frexpf_tm(float num, int* exp)
{
    return frexpf(num, exp);
}
#endif

#if defined(PICOTM_LIBM_HAVE_FREXPL) && PICOTM_LIBM_HAVE_FREXPL
PICOTM_EXPORT
long double
frexpl_tm(long double num, int* exp)
{
    return frexpl(num, exp);
}
#endif

#if defined(PICOTM_LIBM_HAVE_MODF) && PICOTM_LIBM_HAVE_MODF
PICOTM_EXPORT
double
modf_tm(double x, double* iptr)
{
    return modf(x, iptr);
}
#endif

#if defined(PICOTM_LIBM_HAVE_MODFF) && PICOTM_LIBM_HAVE_MODFF
PICOTM_EXPORT
float
modff_tm(float x, float* iptr)
{
    return modff(x, iptr);
}
#endif

#if defined(PICOTM_LIBM_HAVE_MODFL) && PICOTM_LIBM_HAVE_MODFL
PICOTM_EXPORT
long double
modfl_tm(long double x, long double* iptr)
{
    return modfl(x, iptr);
}
#endif

#if defined(PICOTM_LIBM_HAVE_NAN) && PICOTM_LIBM_HAVE_NAN
PICOTM_EXPORT
double
nan_tm(const char* tagp)
{
    return nan(tagp);
}
#endif

#if defined(PICOTM_LIBM_HAVE_NANF) && PICOTM_LIBM_HAVE_NANF
PICOTM_EXPORT
float
nanf_tm(const char* tagp)
{
    return nanf(tagp);
}
#endif

#if defined(PICOTM_LIBM_HAVE_NANL) && PICOTM_LIBM_HAVE_NANL
PICOTM_EXPORT
long double
nanl_tm(const char* tagp)
{
    return nanl(tagp);
}
#endif

#if defined(PICOTM_LIBM_HAVE_REMQUO) && PICOTM_LIBM_HAVE_REMQUO
PICOTM_EXPORT
double
remquo_tm(double x, double y, int* quo)
{
    MATHERR(double, res, remquo(x, y, quo), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_REMQUOF) && PICOTM_LIBM_HAVE_REMQUOF
PICOTM_EXPORT
float
remquof_tm(float x, float y, int* quo)
{
    MATHERR(float, res, remquof(x, y, quo), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_REMQUOL) && PICOTM_LIBM_HAVE_REMQUOL
PICOTM_EXPORT
long double
remquol_tm(long double x, long double y, int* quo)
{
    MATHERR(long double, res, remquol(x, y, quo), FE_INVALID);
    return res;
}
#endif
