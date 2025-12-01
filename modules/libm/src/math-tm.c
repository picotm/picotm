/*
 * picotm - A system-level transaction manager
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
