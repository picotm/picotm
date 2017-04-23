/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
