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

#pragma once

#include <complex.h>
#include <picotm/compiler.h>
#include <picotm/picotm-tm.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libm
 * \file
 *
 * \brief Transactional wrappers for interfaces of <complex.h>.
 */

PICOTM_TM_LOAD_TX(cdouble, double _Complex);
PICOTM_TM_LOAD_TX(cfloat, float _Complex);
PICOTM_TM_LOAD_TX(cldouble, long double _Complex);

PICOTM_TM_STORE_TX(cdouble, double _Complex);
PICOTM_TM_STORE_TX(cfloat, float _Complex);
PICOTM_TM_STORE_TX(cldouble, long double _Complex);

PICOTM_TM_PRIVATIZE_TX(cdouble, double _Complex);
PICOTM_TM_PRIVATIZE_TX(cfloat, float _Complex);
PICOTM_TM_PRIVATIZE_TX(cldouble, long double _Complex);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cabs()][posix::cabs].
 *
 * [posix::cabs]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cabs.html
 */
double
cabs_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cabsf()][posix::cabsf].
 *
 * [posix::cabsf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cabsf.html
 */
float
cabsf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cabsl()][posix::cabsl].
 *
 * [posix::cabsl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cabsl.html
 */
long double
cabsl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cacos()][posix::cacos].
 *
 * [posix::cacos]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cacos.html
 */
double complex
cacos_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cacosf()][posix::cacosf].
 *
 * [posix::cacosf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cacosf.html
 */
float complex
cacosf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cacosh()][posix::cacosh].
 *
 * [posix::cacosh]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cacosh.html
 */
double complex
cacosh_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cacoshf()][posix::cacoshf].
 *
 * [posix::cacoshf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cacoshf.html
 */
float complex
cacoshf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cacoshl()][posix::cacoshl].
 *
 * [posix::cacoshl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cacoshl.html
 */
long double complex
cacoshl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cacosl()][posix::cacosl].
 *
 * [posix::cacosl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cacosl.html
 */
long double complex
cacosl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [carg()][posix::carg].
 *
 * [posix::carg]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/carg.html
 */
double
carg_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cargf()][posix::cargf].
 *
 * [posix::cargf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cargf.html
 */
float
cargf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cargl()][posix::cargl].
 *
 * [posix::cargl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cargl.html
 */
long double
cargl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [casin()][posix::casin].
 *
 * [posix::casin]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/casin.html
 */
double complex
casin_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [casinf()][posix::casinf].
 *
 * [posix::casinf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/casinf.html
 */
float complex
casinf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [casinh()][posix::casinh].
 *
 * [posix::casinh]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/casinh.html
 */
double complex
casinh_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [casinhf()][posix::casinhf].
 *
 * [posix::casinhf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/casinhf.html
 */
float complex
casinhf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [casinhl()][posix::casinhl].
 *
 * [posix::casinhl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/casinhl.html
 */
long double complex
casinhl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [casinl()][posix::casinl].
 *
 * [posix::casinl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/casinl.html
 */
long double complex
casinl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [catan()][posix::catan].
 *
 * [posix::catan]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/catan.html
 */
double complex
catan_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [catanf()][posix::catanf].
 *
 * [posix::catanf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/catanf.html
 */
float complex
catanf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [catanh()][posix::catanh].
 *
 * [posix::catanh]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/catanh.html
 */
double complex
catanh_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [catanhf()][posix::catanhf].
 *
 * [posix::catanhf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/catanhf.html
 */
float complex
catanhf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [catanhl()][posix::catanhl].
 *
 * [posix::catanhl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/catanhl.html
 */
long double complex
catanhl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [catanl()][posix::catanl].
 *
 * [posix::catanl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/catanl.html
 */
long double complex
catanl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ccos()][posix::ccos].
 *
 * [posix::ccos]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ccos.html
 */
double complex
ccos_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ccosf()][posix::ccosf].
 *
 * [posix::ccosf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ccosf.html
 */
float complex
ccosf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ccosh()][posix::ccosh].
 *
 * [posix::ccosh]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ccosh.html
 */
double complex
ccosh_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ccoshf()][posix::ccoshf].
 *
 * [posix::ccoshf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ccoshf.html
 */
float complex
ccoshf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ccoshl()][posix::ccoshl].
 *
 * [posix::ccoshl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ccoshl.html
 */
long double complex
ccoshl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ccosl()][posix::ccosl].
 *
 * [posix::ccosl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ccosl.html
 */
long double complex
ccosl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cexp()][posix::cexp].
 *
 * [posix::cexp]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cexp.html
 */
double complex
cexp_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cexpf()][posix::cexpf].
 *
 * [posix::cexpf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cexpf.html
 */
float complex
cexpf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cexpl()][posix::cexpl].
 *
 * [posix::cexpl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cexpl.html
 */
long double complex
cexpl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cimag()][posix::cimag].
 *
 * [posix::cimag]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cimag.html
 */
double
cimag_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cimagf()][posix::cimagf].
 *
 * [posix::cimagf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cimagf.html
 */
float
cimagf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cimagl()][posix::cimagl].
 *
 * [posix::cimagl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cimagl.html
 */
long double
cimagl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [clog()][posix::clog].
 *
 * [posix::clog]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/clog.html
 */
double complex
clog_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [clogf()][posix::clogf].
 *
 * [posix::clogf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/clogf.html
 */
float complex
clogf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [clogl()][posix::clogl].
 *
 * [posix::clogl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/clogl.html
 */
long double complex
clogl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [conj()][posix::conj].
 *
 * [posix::conj]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/conj.html
 */
double complex
conj_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [conjf()][posix::conjf].
 *
 * [posix::conjf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/conjf.html
 */
float complex
conjf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [conjl()][posix::conjl].
 *
 * [posix::conjl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/conjl.html
 */
long double complex
conjl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cpow()][posix::cpow].
 *
 * [posix::cpow]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cpow.html
 */
double complex
cpow_tx(double complex x, double complex y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cpowf()][posix::cpowf].
 *
 * [posix::cpowf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cpowf.html
 */
float complex
cpowf_tx(float complex x, float complex y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cpowl()][posix::cpowl].
 *
 * [posix::cpowl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cpowl.html
 */
long double complex
cpowl_tx(long double complex x, long double complex y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cproj()][posix::cproj].
 *
 * [posix::cproj]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cproj.html
 */
double complex
cproj_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cprojf()][posix::cprojf].
 *
 * [posix::cprojf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cprojf.html
 */
float complex
cprojf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cprojl()][posix::cprojl].
 *
 * [posix::cprojl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cprojl.html
 */
long double complex
cprojl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [creal()][posix::creal].
 *
 * [posix::creal]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/creal.html
 */
double
creal_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [crealf()][posix::crealf].
 *
 * [posix::crealf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/crealf.html
 */
float
crealf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [creall()][posix::creall].
 *
 * [posix::creall]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/creall.html
 */
long double
creall_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [csin()][posix::csin].
 *
 * [posix::csin]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/csin.html
 */
double complex
csin_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [csinf()][posix::csinf].
 *
 * [posix::csinf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/csinf.html
 */
float complex
csinf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [csinh()][posix::csinh].
 *
 * [posix::csinh]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/csinh.html
 */
double complex
csinh_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [csinhf()][posix::csinhf].
 *
 * [posix::csinhf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/csinhf.html
 */
float complex
csinhf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [csinhl()][posix::csinhl].
 *
 * [posix::csinhl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/csinhl.html
 */
long double complex
csinhl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [csinl()][posix::csinl].
 *
 * [posix::csinl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/csinl.html
 */
long double complex
csinl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [csqrt()][posix::csqrt].
 *
 * [posix::csqrt]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/csqrt.html
 */
double complex
csqrt_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [csqrtf()][posix::csqrtf].
 *
 * [posix::csqrtf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/csqrtf.html
 */
float complex
csqrtf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [csqrtl()][posix::csqrtl].
 *
 * [posix::csqrtl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/csqrtl.html
 */
long double complex
csqrtl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ctan()][posix::ctan].
 *
 * [posix::ctan]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ctan.html
 */
double complex
ctan_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ctanf()][posix::ctanf].
 *
 * [posix::ctanf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ctanf.html
 */
float complex
ctanf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ctanh()][posix::ctanh].
 *
 * [posix::ctanh]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ctanh.html
 */
double complex
ctanh_tx(double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ctanhf()][posix::ctanhf].
 *
 * [posix::ctanhf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ctanhf.html
 */
float complex
ctanhf_tx(float complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ctanhl()][posix::ctanhl].
 *
 * [posix::ctanhl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ctanhl.html
 */
long double complex
ctanhl_tx(long double complex z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ctanl()][posix::ctanl].
 *
 * [posix::ctanl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ctanl.html
 */
long double complex
ctanl_tx(long double complex z);

PICOTM_END_DECLS
