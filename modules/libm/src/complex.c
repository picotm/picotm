/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann
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

#include "picotm/complex.h"

#if defined(PICOTM_LIBM_HAVE_CABS) && PICOTM_LIBM_HAVE_CABS
PICOTM_EXPORT
double
cabs_tx(double complex z)
{
    return cabs(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CABSF) && PICOTM_LIBM_HAVE_CABSF
PICOTM_EXPORT
float
cabsf_tx(float complex z)
{
    return cabsf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CABSL) && PICOTM_LIBM_HAVE_CABSL
PICOTM_EXPORT
long double
cabsl_tx(long double complex z)
{
    return cabsl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CACOS) && PICOTM_LIBM_HAVE_CACOS
PICOTM_EXPORT
double complex
cacos_tx(double complex z)
{
    return cacos(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CACOSF) && PICOTM_LIBM_HAVE_CACOSF
PICOTM_EXPORT
float complex
cacosf_tx(float complex z)
{
    return cacosf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CACOSH) && PICOTM_LIBM_HAVE_CACOSH
PICOTM_EXPORT
double complex
cacosh_tx(double complex z)
{
    return cacosh(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CACOSHF) && PICOTM_LIBM_HAVE_CACOSHF
PICOTM_EXPORT
float complex
cacoshf_tx(float complex z)
{
    return cacoshf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CACOSHL) && PICOTM_LIBM_HAVE_CACOSHL
PICOTM_EXPORT
long double complex
cacoshl_tx(long double complex z)
{
    return cacoshl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CACOSHL) && PICOTM_LIBM_HAVE_CACOSHL
PICOTM_EXPORT
long double complex
cacosl_tx(long double complex z)
{
    return cacosl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CARG) && PICOTM_LIBM_HAVE_CARG
PICOTM_EXPORT
double
carg_tx(double complex z)
{
    return carg(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CARGF) && PICOTM_LIBM_HAVE_CARGF
PICOTM_EXPORT
float
cargf_tx(float complex z)
{
    return cargf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CARGL) && PICOTM_LIBM_HAVE_CARGL
PICOTM_EXPORT
long double
cargl_tx(long double complex z)
{
    return cargl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CSIN) && PICOTM_LIBM_HAVE_CSIN
PICOTM_EXPORT
double complex
casin_tx(double complex z)
{
    return casin(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CSINF) && PICOTM_LIBM_HAVE_CSINF
PICOTM_EXPORT
float complex
casinf_tx(float complex z)
{
    return casinf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CSINH) && PICOTM_LIBM_HAVE_CSINH
PICOTM_EXPORT
double complex
casinh_tx(double complex z)
{
    return casinh(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CSINHF) && PICOTM_LIBM_HAVE_CSINHF
PICOTM_EXPORT
float complex
casinhf_tx(float complex z)
{
    return casinhf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CASINHL) && PICOTM_LIBM_HAVE_CASINHL
PICOTM_EXPORT
long double complex
casinhl_tx(long double complex z)
{
    return casinhl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CASINL) && PICOTM_LIBM_HAVE_CASINL
PICOTM_EXPORT
long double complex
casinl_tx(long double complex z)
{
    return casinl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CATAN) && PICOTM_LIBM_HAVE_CATAN
PICOTM_EXPORT
double complex
catan_tx(double complex z)
{
    return catan(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CATANF) && PICOTM_LIBM_HAVE_CATANF
PICOTM_EXPORT
float complex
catanf_tx(float complex z)
{
    return catanf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CATANH) && PICOTM_LIBM_HAVE_CATANH
PICOTM_EXPORT
double complex
catanh_tx(double complex z)
{
    return catanh(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CATANHF) && PICOTM_LIBM_HAVE_CATANHF
PICOTM_EXPORT
float complex
catanhf_tx(float complex z)
{
    return catanhf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CATANHL) && PICOTM_LIBM_HAVE_CATANHL
PICOTM_EXPORT
long double complex
catanhl_tx(long double complex z)
{
    return catanhl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CATANL) && PICOTM_LIBM_HAVE_CATANL
PICOTM_EXPORT
long double complex
catanl_tx(long double complex z)
{
    return catanl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CCOS) && PICOTM_LIBM_HAVE_CCOS
PICOTM_EXPORT
double complex
ccos_tx(double complex z)
{
    return ccos(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CCOSF) && PICOTM_LIBM_HAVE_CCOSF
PICOTM_EXPORT
float complex
ccosf_tx(float complex z)
{
    return ccosf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CCOSH) && PICOTM_LIBM_HAVE_CCOSH
PICOTM_EXPORT
double complex
ccosh_tx(double complex z)
{
    return ccosh(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CCOSHF) && PICOTM_LIBM_HAVE_CCOSHF
PICOTM_EXPORT
float complex
ccoshf_tx(float complex z)
{
    return ccoshf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CCOSHL) && PICOTM_LIBM_HAVE_CCOSHL
PICOTM_EXPORT
long double complex
ccoshl_tx(long double complex z)
{
    return ccoshl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CCOSL) && PICOTM_LIBM_HAVE_CCOSL
PICOTM_EXPORT
long double complex
ccosl_tx(long double complex z)
{
    return ccosl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CEXP) && PICOTM_LIBM_HAVE_CEXP
PICOTM_EXPORT
double complex
cexp_tx(double complex z)
{
    return cexp(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CEXPF) && PICOTM_LIBM_HAVE_CEXPF
PICOTM_EXPORT
float complex
cexpf_tx(float complex z)
{
    return cexpf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CEXPL) && PICOTM_LIBM_HAVE_CEXPL
PICOTM_EXPORT
long double complex
cexpl_tx(long double complex z)
{
    return cexpl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CIMAG) && PICOTM_LIBM_HAVE_CIMAG
PICOTM_EXPORT
double
cimag_tx(double complex z)
{
    return cimag(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CIMAGF) && PICOTM_LIBM_HAVE_CIMAGF
PICOTM_EXPORT
float
cimagf_tx(float complex z)
{
    return cimagf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CIMAGL) && PICOTM_LIBM_HAVE_CIMAGL
PICOTM_EXPORT
long double
cimagl_tx(long double complex z)
{
    return cimagl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CLOG) && PICOTM_LIBM_HAVE_CLOG
PICOTM_EXPORT
double complex
clog_tx(double complex z)
{
    return clog(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CLOGF) && PICOTM_LIBM_HAVE_CLOGF
PICOTM_EXPORT
float complex
clogf_tx(float complex z)
{
    return clogf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CLOGL) && PICOTM_LIBM_HAVE_CLOGL
PICOTM_EXPORT
long double complex
clogl_tx(long double complex z)
{
    return clogl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CONJ) && PICOTM_LIBM_HAVE_CONJ
PICOTM_EXPORT
double complex
conj_tx(double complex z)
{
    return conj(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CONJF) && PICOTM_LIBM_HAVE_CONJF
PICOTM_EXPORT
float complex
conjf_tx(float complex z)
{
    return conjf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CONJL) && PICOTM_LIBM_HAVE_CONJL
PICOTM_EXPORT
long double complex
conjl_tx(long double complex z)
{
    return conjl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CPOW) && PICOTM_LIBM_HAVE_CPOW
PICOTM_EXPORT
double complex
cpow_tx(double complex x, double complex y)
{
    return cpow(x, y);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CPOWF) && PICOTM_LIBM_HAVE_CPOWF
PICOTM_EXPORT
float complex
cpowf_tx(float complex x, float complex y)
{
    return cpowf(x, y);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CPOWL) && PICOTM_LIBM_HAVE_CPOWL
PICOTM_EXPORT
long double complex
cpowl_tx(long double complex x, long double complex y)
{
    return cpowl(x, y);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CPROJ) && PICOTM_LIBM_HAVE_CPROJ
PICOTM_EXPORT
double complex
cproj_tx(double complex z)
{
    return cproj(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CPROJF) && PICOTM_LIBM_HAVE_CPROJF
PICOTM_EXPORT
float complex
cprojf_tx(float complex z)
{
    return cprojf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CPROJL) && PICOTM_LIBM_HAVE_CPROJL
PICOTM_EXPORT
long double complex
cprojl_tx(long double complex z)
{
    return cprojl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CREAL) && PICOTM_LIBM_HAVE_CREAL
PICOTM_EXPORT
double
creal_tx(double complex z)
{
    return creal(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CREALF) && PICOTM_LIBM_HAVE_CREALF
PICOTM_EXPORT
float
crealf_tx(float complex z)
{
    return crealf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CREALL) && PICOTM_LIBM_HAVE_CREALL
PICOTM_EXPORT
long double
creall_tx(long double complex z)
{
    return creall(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CSIN) && PICOTM_LIBM_HAVE_CSIN
PICOTM_EXPORT
double complex
csin_tx(double complex z)
{
    return csin(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CSINF) && PICOTM_LIBM_HAVE_CSINF
PICOTM_EXPORT
float complex
csinf_tx(float complex z)
{
    return csinf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CSINH) && PICOTM_LIBM_HAVE_CSINH
PICOTM_EXPORT
double complex
csinh_tx(double complex z)
{
    return csinh(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CSINHF) && PICOTM_LIBM_HAVE_CSINHF
PICOTM_EXPORT
float complex
csinhf_tx(float complex z)
{
    return csinhf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CSINHL) && PICOTM_LIBM_HAVE_CSINHL
PICOTM_EXPORT
long double complex
csinhl_tx(long double complex z)
{
    return csinhl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CSINL) && PICOTM_LIBM_HAVE_CSINL
PICOTM_EXPORT
long double complex
csinl_tx(long double complex z)
{
    return csinl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CSQRT) && PICOTM_LIBM_HAVE_CSQRT
PICOTM_EXPORT
double complex
csqrt_tx(double complex z)
{
    return csqrt(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CSQRTF) && PICOTM_LIBM_HAVE_CSQRTF
PICOTM_EXPORT
float complex
csqrtf_tx(float complex z)
{
    return csqrtf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CSQRTL) && PICOTM_LIBM_HAVE_CSQRTL
PICOTM_EXPORT
long double complex
csqrtl_tx(long double complex z)
{
    return csqrtl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CTAN) && PICOTM_LIBM_HAVE_CTAN
PICOTM_EXPORT
double complex
ctan_tx(double complex z)
{
    return ctan(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CTANF) && PICOTM_LIBM_HAVE_CTANF
PICOTM_EXPORT
float complex
ctanf_tx(float complex z)
{
    return ctanf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CTANH) && PICOTM_LIBM_HAVE_CTANH
PICOTM_EXPORT
double complex
ctanh_tx(double complex z)
{
    return ctanh(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CTANHF) && PICOTM_LIBM_HAVE_CTANHF
PICOTM_EXPORT
float complex
ctanhf_tx(float complex z)
{
    return ctanhf(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CTANHL) && PICOTM_LIBM_HAVE_CTANHL
PICOTM_EXPORT
long double complex
ctanhl_tx(long double complex z)
{
    return ctanhl(z);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CTANL) && PICOTM_LIBM_HAVE_CTANL
PICOTM_EXPORT
long double complex
ctanl_tx(long double complex z)
{
    return ctanl(z);
}
#endif
