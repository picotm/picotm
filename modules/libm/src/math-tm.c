/* Permission is hereby granted, free of charge, to any person obtaining a
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

PICOTM_EXPORT
double
frexp_tm(double num, int* exp)
{
    return frexp(num, exp);
}

PICOTM_EXPORT
float
frexpf_tm(float num, int* exp)
{
    return frexpf(num, exp);
}

PICOTM_EXPORT
long double
frexpl_tm(long double num, int* exp)
{
    return frexpl(num, exp);
}

PICOTM_EXPORT
double
modf_tm(double x, double* iptr)
{
    return modf(x, iptr);
}

PICOTM_EXPORT
float
modff_tm(float x, float* iptr)
{
    return modff(x, iptr);
}

PICOTM_EXPORT
long double
modfl_tm(long double x, long double* iptr)
{
    return modfl(x, iptr);
}

PICOTM_EXPORT
double
nan_tm(const char* tagp)
{
    return nan(tagp);
}

PICOTM_EXPORT
float
nanf_tm(const char* tagp)
{
    return nanf(tagp);
}

PICOTM_EXPORT
long double
nanl_tm(const char* tagp)
{
    return nanl(tagp);
}

PICOTM_EXPORT
double
remquo_tm(double x, double y, int* quo)
{
    MATHERR(double, res, remquo(x, y, quo), FE_INVALID);
    return res;
}

PICOTM_EXPORT
float
remquof_tm(float x, float y, int* quo)
{
    MATHERR(float, res, remquof(x, y, quo), FE_INVALID);
    return res;
}

PICOTM_EXPORT
long double
remquol_tm(long double x, long double y, int* quo)
{
    MATHERR(long double, res, remquol(x, y, quo), FE_INVALID);
    return res;
}
