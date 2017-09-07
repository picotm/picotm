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

#include <math.h>
#include <picotm/compiler.h>
#include <picotm/picotm-tm.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libm
 * \file
 *
 * \brief Transactional wrappers for interfaces of <math.h>.
 */

PICOTM_TM_LOAD_TX(float_t, float_t);
PICOTM_TM_LOAD_TX(double_t, double_t);

PICOTM_TM_STORE_TX(float_t, float_t);
PICOTM_TM_STORE_TX(double_t, double_t);

PICOTM_TM_PRIVATIZE_TX(float_t, float_t);
PICOTM_TM_PRIVATIZE_TX(double_t, double_t);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [acos()][posix::acos].
 *
 * [posix::acos]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/acos.html
 */
double
acos_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [acosf()][posix::acosf].
 *
 * [posix::acosf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/acosf.html
 */
float
acosf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [acosh()][posix::acosh].
 *
 * [posix::acosh]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/acosh.html
 */
double
acosh_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [acoshf()][posix::acoshf].
 *
 * [posix::acoshf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/acoshf.html
 */
float
acoshf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [acoshl()][posix::acoshl].
 *
 * [posix::acoshl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/acoshl.html
 */
long double
acoshl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [acosl()][posix::acosl].
 *
 * [posix::acosl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/acosl.html
 */
long double
acosl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [asin()][posix::asin].
 *
 * [posix::asin]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/asin.html
 */
double
asin_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [asinf()][posix::asinf].
 *
 * [posix::asinf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/asinf.html
 */
float
asinf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [asinh()][posix::asinh].
 *
 * [posix::asinh]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/asinh.html
 */
double
asinh_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [asinhf()][posix::asinhf].
 *
 * [posix::asinhf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/asinhf.html
 */
float
asinhf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [asinhl()][posix::asinhl].
 *
 * [posix::asinhl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/asinhl.html
 */
long double
asinhl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [asinl()][posix::asinl].
 *
 * [posix::asinl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/asinl.html
 */
long double
asinl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [atan()][posix::atan].
 *
 * [posix::atan]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/atan.html
 */
double
atan_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [atan2()][posix::atan2].
 *
 * [posix::atan2]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/atan2.html
 */
double
atan2_tx(double y, double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [atan2f()][posix::atan2f].
 *
 * [posix::atan2f]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/atan2f.html
 */
float
atan2f_tx(float y, float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [atan2l()][posix::atan2l].
 *
 * [posix::atan2l]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/atan2l.html
 */
long double
atan2l_tx(long double y, long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [atanf()][posix::atanf].
 *
 * [posix::atanf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/atanf.html
 */
float
atanf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [atanh()][posix::atanh].
 *
 * [posix::atanh]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/atanh.html
 */
double
atanh_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [atanhf()][posix::atanhf].
 *
 * [posix::atanhf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/atanhf.html
 */
float
atanhf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [atanhl()][posix::atanhl].
 *
 * [posix::atanhl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/atanhl.html
 */
long double
atanhl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [atanl()][posix::atanl].
 *
 * [posix::atanl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/atanl.html
 */
long double
atanl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cbrt()][posix::cbrt].
 *
 * [posix::cbrt]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cbrt.html
 */
double
cbrt_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cbrtf()][posix::cbrtf].
 *
 * [posix::cbrtf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cbrtf.html
 */
float
cbrtf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cbrtl()][posix::cbrtl].
 *
 * [posix::cbrtl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cbrtl.html
 */
long double
cbrtl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ceil()][posix::ceil].
 *
 * [posix::ceil]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ceil.html
 */
double
ceil_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ceilf()][posix::ceilf].
 *
 * [posix::ceilf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ceilf.html
 */
float
ceilf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ceill()][posix::ceill].
 *
 * [posix::ceill]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ceill.html
 */
long double
ceill_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [copysign()][posix::copysign].
 *
 * [posix::copysign]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/copysign.html
 */
double
copysign_tx(double x, double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [copysignf()][posix::copysignf].
 *
 * [posix::copysignf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/copysignf.html
 */
float
copysignf_tx(float x, float y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [copysignl()][posix::copysignl].
 *
 * [posix::copysignl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/copysignl.html
 */
long double
copysignl_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cos()][posix::cos].
 *
 * [posix::cos]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cos.html
 */
double
cos_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cosf()][posix::cosf].
 *
 * [posix::cosf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cosf.html
 */
float
cosf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cosh()][posix::cosh].
 *
 * [posix::cosh]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cosh.html
 */
double
cosh_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [coshf()][posix::coshf].
 *
 * [posix::coshf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/coshf.html
 */
float
coshf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [coshl()][posix::coshl].
 *
 * [posix::coshl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/coshl.html
 */
long double
coshl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [cosl()][posix::cosl].
 *
 * [posix::cosl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/cosl.html
 */
long double
cosl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [erf()][posix::erf].
 *
 * [posix::erf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/erf.html
 */
double
erf_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [erfc()][posix::erfc].
 *
 * [posix::erfc]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/erfc.html
 */
double
erfc_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [erfcf()][posix::erfcf].
 *
 * [posix::erfcf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/erfcf.html
 */
float
erfcf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [erfcl()][posix::erfcl].
 *
 * [posix::erfcl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/erfcl.html
 */
long double
erfcl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [erff()][posix::erff].
 *
 * [posix::erff]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/erff.html
 */
float
erff_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [erfl()][posix::erfl].
 *
 * [posix::erfl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/erfl.html
 */
long double
erfl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [exp()][posix::exp].
 *
 * [posix::exp]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/exp.html
 */
double
exp_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [exp2()][posix::exp2].
 *
 * [posix::exp2]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/exp2.html
 */
double
exp2_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [exp2f()][posix::exp2f].
 *
 * [posix::exp2f]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/exp2f.html
 */
float
exp2f_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [exp2l()][posix::exp2l].
 *
 * [posix::exp2l]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/exp2l.html
 */
long double
exp2l_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [expf()][posix::expf].
 *
 * [posix::expf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/expf.html
 */
float
expf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [expl()][posix::expl].
 *
 * [posix::expl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/expl.html
 */
long double
expl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [expm1()][posix::expm1].
 *
 * [posix::expm1]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/expm1.html
 */
double
expm1_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [expm1f()][posix::expm1f].
 *
 * [posix::expm1f]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/expm1f.html
 */
float
expm1f_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [expm1l()][posix::expm1l].
 *
 * [posix::expm1l]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/expm1l.html
 */
long double
expm1l_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fabs()][posix::fabs].
 *
 * [posix::fabs]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fabs.html
 */
double
fabs_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fabsf()][posix::fabsf].
 *
 * [posix::fabsf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fabsf.html
 */
float
fabsf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fabsl()][posix::fabsl].
 *
 * [posix::fabsl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fabsl.html
 */
long double
fabsl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fdim()][posix::fdim].
 *
 * [posix::fdim]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fdim.html
 */
double
fdim_tx(double x, double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fdimf()][posix::fdimf].
 *
 * [posix::fdimf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fdimf.html
 */
float
fdimf_tx(float x, float y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fdiml()][posix::fdiml].
 *
 * [posix::fdiml]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fdiml.html
 */
long double
fdiml_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [floor()][posix::floor].
 *
 * [posix::floor]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/floor.html
 */
double
floor_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [floorf()][posix::floorf].
 *
 * [posix::floorf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/floorf.html
 */
float
floorf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [floorl()][posix::floorl].
 *
 * [posix::floorl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/floorl.html
 */
long double
floorl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fma()][posix::fma].
 *
 * [posix::fma]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fma.html
 */
double
fma_tx(double x, double y, double z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fmaf()][posix::fmaf].
 *
 * [posix::fmaf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fmaf.html
 */
float
fmaf_tx(float x, float y, float z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fmal()][posix::fmal].
 *
 * [posix::fmal]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fmal.html
 */
long double
fmal_tx(long double x, long double y, long double z);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fmax()][posix::fmax].
 *
 * [posix::fmax]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fmax.html
 */
double
fmax_tx(double x, double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fmaxf()][posix::fmaxf].
 *
 * [posix::fmaxf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fmaxf.html
 */
float
fmaxf_tx(float x, float y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fmaxl()][posix::fmaxl].
 *
 * [posix::fmaxl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fmaxl.html
 */
long double
fmaxl_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fmin()][posix::fmin].
 *
 * [posix::fmin]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fmin.html
 */
double
fmin_tx(double x, double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fminf()][posix::fminf].
 *
 * [posix::fminf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fminf.html
 */
float
fminf_tx(float x, float y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fminl()][posix::fminl].
 *
 * [posix::fminl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fminl.html
 */
long double
fminl_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fmodl()][posix::fmodl].
 *
 * [posix::fmodl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fmodl.html
 */
double
fmod_tx(double x, double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fmodf()][posix::fmodf].
 *
 * [posix::fmodf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fmodf.html
 */
float
fmodf_tx(float x, float y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fmodl()][posix::fmodl].
 *
 * [posix::fmodl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fmodl.html
 */
long double
fmodl_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [frexp()][posix::frexp].
 *
 * [posix::frexp]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/frexp.html
 */
double
frexp_tx(double num, int* exp);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [frexpf()][posix::frexpf].
 *
 * [posix::frexpf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/frexpf.html
 */
float
frexpf_tx(float num, int* exp);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [frexpl()][posix::frexpl].
 *
 * [posix::frexpl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/frexpl.html
 */
long double
frexpl_tx(long double num, int* exp);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [hypot()][posix::hypot].
 *
 * [posix::hypot]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/hypot.html
 */
double
hypot_tx(double x, double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [hypotf()][posix::hypotf].
 *
 * [posix::hypotf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/hypotf.html
 */
float
hypotf_tx(float x, float y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [hypotl()][posix::hypotl].
 *
 * [posix::hypotl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/hypotl.html
 */
long double
hypotl_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ilogb()][posix::ilogb].
 *
 * [posix::ilogb]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ilogb.html
 */
double
ilogb_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ilogbf()][posix::ilogbf].
 *
 * [posix::ilogbf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ilogbf.html
 */
float
ilogbf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ilogbl()][posix::ilogbl].
 *
 * [posix::ilogbl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ilogbl.html
 */
long double
ilogbl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [j0()][posix::j0].
 *
 * [posix::j0]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/j0.html
 */
double
j0_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [j1()][posix::j1].
 *
 * [posix::j1]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/j1.html
 */
double
j1_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [jn()][posix::jn].
 *
 * [posix::jn]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/jn.html
 */
double
jn_tx(int n, double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ldexp()][posix::ldexp].
 *
 * [posix::ldexp]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ldexp.html
 */
double
ldexp_tx(double x, int exp);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ldexpf()][posix::ldexpf].
 *
 * [posix::ldexpf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ldexpf.html
 */
float
ldexpf_tx(float x, int exp);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [ldexpl()][posix::ldexpl].
 *
 * [posix::ldexpl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/ldexpl.html
 */
long double
ldexpl_tx(long double x, int exp);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [lgamma()][posix::lgamma].
 *
 * [posix::lgamma]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/lgamma.html
 */
double
lgamma_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [lgammaf()][posix::lgammaf].
 *
 * [posix::lgammaf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/lgammaf.html
 */
float
lgammaf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [lgammal()][posix::lgammal].
 *
 * [posix::lgammal]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/lgammal.html
 */
long double
lgammal_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [llrint()][posix::llrint].
 *
 * [posix::llrint]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/llrint.html
 */
long long
llrint_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [llrintf()][posix::llrintf].
 *
 * [posix::llrintf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/llrintf.html
 */
long long
llrintf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [llrintl()][posix::llrintl].
 *
 * [posix::llrintl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/llrintl.html
 */
long long
llrintl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [llround()][posix::llround].
 *
 * [posix::llround]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/llround.html
 */
long long
llround_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [llroundf()][posix::llroundf].
 *
 * [posix::llroundf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/llroundf.html
 */
long long
llroundf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [llroundl()][posix::llroundl].
 *
 * [posix::llroundl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/llroundl.html
 */
long long
llroundl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [log()][posix::log].
 *
 * [posix::log]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/log.html
 */
double
log_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [log10()][posix::log10].
 *
 * [posix::log10]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/log10.html
 */
double
log10_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [log10f()][posix::log10f].
 *
 * [posix::log10f]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/log10f.html
 */
float
log10f_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [log10l()][posix::log10l].
 *
 * [posix::log10l]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/log10l.html
 */
long double
log10l_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [log1p()][posix::log1p].
 *
 * [posix::log1p]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/log1p.html
 */
double
log1p_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [log1pf()][posix::log1pf].
 *
 * [posix::log1pf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/log1pf.html
 */
float
log1pf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [log1pl()][posix::log1pl].
 *
 * [posix::log1pl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/log1pl.html
 */
long double
log1pl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [log2()][posix::log2].
 *
 * [posix::log2]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/log2.html
 */
double
log2_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [log2f()][posix::log2f].
 *
 * [posix::log2f]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/log2f.html
 */
float
log2f_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [log2l()][posix::log2l].
 *
 * [posix::log2l]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/log2l.html
 */
long double
log2l_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [logb()][posix::logb].
 *
 * [posix::logb]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/logb.html
 */
double
logb_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [logbf()][posix::logbf].
 *
 * [posix::logbf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/logbf.html
 */
float
logbf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [logbl()][posix::logbl].
 *
 * [posix::logbl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/logbl.html
 */
long double
logbl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [logf()][posix::logf].
 *
 * [posix::logf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/logf.html
 */
float
logf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [logl()][posix::logl].
 *
 * [posix::logl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/logl.html
 */
long double
logl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [lrint()][posix::lrint].
 *
 * [posix::lrint]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/lrint.html
 */
long
lrint_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [lrintf()][posix::lrintf].
 *
 * [posix::lrintf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/lrintf.html
 */
long
lrintf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [lrintl()][posix::lrintl].
 *
 * [posix::lrintl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/lrintl.html
 */
long
lrintl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [lround()][posix::lround].
 *
 * [posix::lround]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/lround.html
 */
long
lround_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [lroundf()][posix::lroundf].
 *
 * [posix::lroundf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/lroundf.html
 */
long
lroundf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [lroundl()][posix::lroundl].
 *
 * [posix::lroundl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/lroundl.html
 */
long
lroundl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [modf()][posix::modf].
 *
 * [posix::modf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/modf.html
 */
double
modf_tx(double x, double* iptr);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [modff()][posix::modff].
 *
 * [posix::modff]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/modff.html
 */
float
modff_tx(float x, float* iptr);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [modfl()][posix::modfl].
 *
 * [posix::modfl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/modfl.html
 */
long double
modfl_tx(long double x, long double* iptr);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [nan()][posix::nan].
 *
 * [posix::nan]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/nan.html
 */
double
nan_tx(const char* tagp);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [nanf()][posix::nanf].
 *
 * [posix::nanf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/nanf.html
 */
float
nanf_tx(const char* tagp);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [nanl()][posix::nanl].
 *
 * [posix::nanl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/nanl.html
 */
long double
nanl_tx(const char* tagp);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [nearbyint()][posix::nearbyint].
 *
 * [posix::nearbyint]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/nearbyint.html
 */
double
nearbyint_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [nearbyintf()][posix::nearbyintf].
 *
 * [posix::nearbyintf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/nearbyintf.html
 */
float
nearbyintf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [nearbyintl()][posix::nearbyintl].
 *
 * [posix::nearbyintl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/nearbyintl.html
 */
long double
nearbyintl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [nextafter()][posix::nextafter].
 *
 * [posix::nextafter]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/nextafter.html
 */
double
nextafter_tx(double x, double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [nearafterf()][posix::nearafterf].
 *
 * [posix::nearafterf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/nearafterf.html
 */
float
nextafterf_tx(float x, float y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [nearafterl()][posix::nearafterl].
 *
 * [posix::nearafterl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/nearafterl.html
 */
long double
nextafterl_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [nexttoward()][posix::nexttoward].
 *
 * [posix::nexttoward]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/nexttoward.html
 */
double
nexttoward_tx(double x, long double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [nexttowardf()][posix::nexttowardf].
 *
 * [posix::nexttowardf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/nexttowardf.html
 */
float
nexttowardf_tx(float x, long double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [nexttowardl()][posix::nexttowardl].
 *
 * [posix::nexttowardl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/nexttowardl.html
 */
long double
nexttowardl_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [pow()][posix::pow].
 *
 * [posix::pow]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/pow.html
 */
double
pow_tx(double x, double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [powf()][posix::powf].
 *
 * [posix::powf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/powf.html
 */
float
powf_tx(float x, float y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [powl()][posix::powl].
 *
 * [posix::powl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/powl.html
 */
long double
powl_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [remainder()][posix::remainder].
 *
 * [posix::remainder]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/remainder.html
 */
double
remainder_tx(double x, double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [remainderf()][posix::remainderf].
 *
 * [posix::remainderf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/remainderf.html
 */
float
remainderf_tx(float x, float y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [remainderl()][posix::remainderl].
 *
 * [posix::remainderl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/remainderl.html
 */
long double
remainderl_tx(long double x, long double y);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [remquo()][posix::remquo].
 *
 * [posix::remquo]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/remquo.html
 */
double
remquo_tx(double x, double y, int* quo);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [remquof()][posix::remquof].
 *
 * [posix::remquof]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/remquof.html
 */
float
remquof_tx(float x, float y, int* quo);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [remquol()][posix::remquol].
 *
 * [posix::remquol]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/remquol.html
 */
long double
remquol_tx(long double x, long double y, int* quo);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [rint()][posix::rint].
 *
 * [posix::rint]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/rint.html
 */
double
rint_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [rintl()][posix::rintl].
 *
 * [posix::rintl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/rintl.html
 */
float
rintf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [rintl()][posix::rintl].
 *
 * [posix::rintl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/rintl.html
 */
long double
rintl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [round()][posix::round].
 *
 * [posix::round]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/round.html
 */
double
round_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [roundf()][posix::roundf].
 *
 * [posix::roundf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/roundf.html
 */
float
roundf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [roundl()][posix::roundl].
 *
 * [posix::roundl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/roundl.html
 */
long double
roundl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [scalbln()][posix::scalbln].
 *
 * [posix::scalbln]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/scalbln.html
 */
double
scalbln_tx(double x, long n);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [scalblnf()][posix::scalblnf].
 *
 * [posix::scalblnf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/scalblnf.html
 */
float
scalblnf_tx(float x, long n);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [scalblnl()][posix::scalblnl].
 *
 * [posix::scalblnl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/scalblnl.html
 */
long double
scalblnl_tx(long double x, long n);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [scalbn()][posix::scalbn].
 *
 * [posix::scalbn]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/scalbn.html
 */
double
scalbn_tx(double x, int n);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [scalbnf()][posix::scalbnf].
 *
 * [posix::scalbnf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/scalbnf.html
 */
float
scalbnf_tx(float x, int n);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [scalbnl()][posix::scalbnl].
 *
 * [posix::scalbnl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/scalbnl.html
 */
long double
scalbnl_tx(long double x, int n);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [sin()][posix::sin].
 *
 * [posix::sin]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/sin.html
 */
double
sin_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [sinf()][posix::sinf].
 *
 * [posix::sinf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/sinf.html
 */
float
sinf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [sinh()][posix::sinh].
 *
 * [posix::sinh]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/sinh.html
 */
double
sinh_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [sinhf()][posix::sinhf].
 *
 * [posix::sinhf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/sinhf.html
 */
float
sinhf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [sinhl()][posix::sinhl].
 *
 * [posix::sinhl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/sinhl.html
 */
long double
sinhl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [sinl()][posix::sinl].
 *
 * [posix::sinl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/sinl.html
 */
long double
sinl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [sqrt()][posix::sqrt].
 *
 * [posix::sqrt]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/sqrt.html
 */
double
sqrt_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [sqrtf()][posix::sqrtf].
 *
 * [posix::sqrtf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/sqrtf.html
 */
float
sqrtf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [sqrtl()][posix::sqrtl].
 *
 * [posix::sqrtl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/sqrtl.html
 */
long double
sqrtl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [tan()][posix::tan].
 *
 * [posix::tan]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/tan.html
 */
double
tan_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [tanf()][posix::tanf].
 *
 * [posix::tanf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/tanf.html
 */
float
tanf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [tanh()][posix::tanh].
 *
 * [posix::tanh]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/tanh.html
 */
double
tanh_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [tanhf()][posix::tanhf].
 *
 * [posix::tanhf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/tanhf.html
 */
float
tanhf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [tanhl()][posix::tanhl].
 *
 * [posix::tanhl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/tanhl.html
 */
long double
tanhl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [tanl()][posix::tanl].
 *
 * [posix::tanl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/tanl.html
 */
long double
tanl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [tgamma()][posix::tgamma].
 *
 * [posix::tgamma]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/tgamma.html
 */
double
tgamma_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [tgammaf()][posix::tgammaf].
 *
 * [posix::tgammaf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/tgammaf.html
 */
float
tgammaf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [tgammal()][posix::tgammal].
 *
 * [posix::tgammal]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/tgammal.html
 */
long double
tgammal_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [trunc()][posix::trunc].
 *
 * [posix::trunc]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/trunc.html
 */
double
trunc_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [truncf()][posix::truncf].
 *
 * [posix::truncf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/truncf.html
 */
float
truncf_tx(float x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [truncl()][posix::truncl].
 *
 * [posix::truncl]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/truncl.html
 */
long double
truncl_tx(long double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [y0()][posix::y0].
 *
 * [posix::y0]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/y0.html
 */
double
y0_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [y1()][posix::y1].
 *
 * [posix::y1]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/y1.html
 */
double
y1_tx(double x);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [yn()][posix::yn].
 *
 * [posix::yn]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/yn.html
 */
double
yn_tx(int n, double x);

PICOTM_END_DECLS
