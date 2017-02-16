/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

ceuta_hdrl(#ifndef TANGER_STM_STD_COMPLEX_H);
ceuta_hdrl(#define TANGER_STM_STD_COMPLEX_H);
ceuta_hdrl(#include <complex.h>);

#include <complex.h>

/* Trigonometric functions
 */

ceuta_pure(complex double,      csin,  csin,  complex double x);
ceuta_pure(complex double,      csinf, csinf, complex float x);
ceuta_pure(complex long double, csinl, csinl, complex long double x);
ceuta_pure(complex double,      ccos,  ccos,  complex double x);
ceuta_pure(complex double,      ccosf, ccosf, complex float x);
ceuta_pure(complex long double, ccosl, ccosl, complex long double x);
ceuta_pure(complex double,      ctan,  ctan,  complex double x);
ceuta_pure(complex double,      ctanf, ctanf, complex float x);
ceuta_pure(complex long double, ctanl, ctanl, complex long double x);

/* Inverse trigonometric functions
 */

ceuta_pure(complex double,      casin,  casin,  complex double x);
ceuta_pure(complex double,      cacos,  cacos,  complex double x);
ceuta_pure(complex double,      catan,  catan,  complex double x);
ceuta_pure(complex float,       casinf, casinf, complex float x);
ceuta_pure(complex float,       cacosf, cacosf, complex float x);
ceuta_pure(complex float,       catanf, catanf, complex float x);
ceuta_pure(complex long double, casinl, casinl, complex long double x);
ceuta_pure(complex long double, cacosl, cacosl, complex long double x);
ceuta_pure(complex long double, catanl, catanl, complex long double x);

/* Exponents and logarithms
 */

ceuta_pure(complex double,      cexp,   cexp,   complex double x);
ceuta_pure(complex float,       cexpf,  cexpf,  complex float x);
ceuta_pure(complex long double, cexpl,  cexpl,  complex long double x);
ceuta_pure(complex double,      clog,   clog,   complex double x);
ceuta_pure(complex float,       clogf,  clogf,  complex float x);
ceuta_pure(complex long double, clogl,  clogl,  complex long double x);
ceuta_pure(complex double,      csqrt,  csqrt,  complex double x);
ceuta_pure(complex float,       csqrtf, csqrtf, complex float x);
ceuta_pure(complex long double, csqrtl, csqrtl, complex long double x);
ceuta_pure(complex double,      cpow,   cpow,   complex double x,      complex double y);
ceuta_pure(complex float,       cpowf,  cpowf,  complex float x,       complex float y);
ceuta_pure(complex long double, cpowl,  cpowl,  complex long double x, complex long double y);

/* Hyperbolic functions
 */

ceuta_pure(complex double,      csinh,   csinh,   complex double x);
ceuta_pure(complex float,       csinhf,  csinhf,  complex float x);
ceuta_pure(complex long double, csinhl,  csinhl,  complex long double x);
ceuta_pure(complex double,      ccosh,   ccosh,   complex double x);
ceuta_pure(complex float,       ccoshf,  ccoshf,  complex float x);
ceuta_pure(complex long double, ccoshl,  ccoshl,  complex long double x);
ceuta_pure(complex double,      ctanh,   ctanh,   complex double x);
ceuta_pure(complex float,       ctanhf,  ctanhf,  complex float x);
ceuta_pure(complex long double, ctanhl,  ctanhl,  complex long double x);
ceuta_pure(complex double,      casinh,  casinh,  complex double x);
ceuta_pure(complex float,       casinhf, casinhf, complex float x);
ceuta_pure(complex long double, casinhl, casinhl, complex long double x);
ceuta_pure(complex double,      cacosh,  cacosh,  complex double x);
ceuta_pure(complex float,       cacoshf, cacoshf, complex float x);
ceuta_pure(complex long double, cacoshl, cacoshl, complex long double x);
ceuta_pure(complex double,      catanh,  catanh,  complex double x);
ceuta_pure(complex float,       catanhf, catanhf, complex float x);
ceuta_pure(complex long double, catanhl, catanhl, complex long double x);

ceuta_hdrl(#endif);

