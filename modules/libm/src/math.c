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

#include "picotm/math.h"
#include <fenv.h>
#include <math.h>
#include <picotm/picotm-tm.h>
#include <picotm/picotm-libc.h>
#include "matherr.h"
#include "picotm/math-tm.h"

#if defined(PICOTM_LIBM_HAVE_ACOS) && PICOTM_LIBM_HAVE_ACOS
PICOTM_EXPORT
double
acos_tx(double x)
{
    MATHERR(double, res, acos(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ACOSF) && PICOTM_LIBM_HAVE_ACOSF
PICOTM_EXPORT
float
acosf_tx(float x)
{
    MATHERR(float, res, acosf(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ACOSH) && PICOTM_LIBM_HAVE_ACOSH
PICOTM_EXPORT
double
acosh_tx(double x)
{
    MATHERR(double, res, acosh(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ACOSHF) && PICOTM_LIBM_HAVE_ACOSHF
PICOTM_EXPORT
float
acoshf_tx(float x)
{
    MATHERR(float, res, acoshf(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ACOSHL) && PICOTM_LIBM_HAVE_ACOSHL
PICOTM_EXPORT
long double
acoshl_tx(long double x)
{
    MATHERR(long double, res, acoshl(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ACOSL) && PICOTM_LIBM_HAVE_ACOSL
PICOTM_EXPORT
long double
acosl_tx(long double x)
{
    MATHERR(long double, res, acosl(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ASIN) && PICOTM_LIBM_HAVE_ASIN
PICOTM_EXPORT
double
asin_tx(double x)
{
    MATHERR(double, res, asin(x), FE_INVALID | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ASINF) && PICOTM_LIBM_HAVE_ASINF
PICOTM_EXPORT
float
asinf_tx(float x)
{
    MATHERR(float, res, asinf(x), FE_INVALID | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ASINH) && PICOTM_LIBM_HAVE_ASINH
PICOTM_EXPORT
double
asinh_tx(double x)
{
    MATHERR(double, res, asinh(x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ASINHF) && PICOTM_LIBM_HAVE_ASINHF
PICOTM_EXPORT
float
asinhf_tx(float x)
{
    MATHERR(float, res, asinhf(x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ASINHL) && PICOTM_LIBM_HAVE_ASINHL
PICOTM_EXPORT
long double
asinhl_tx(long double x)
{
    MATHERR(long double, res, asinhl(x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ASINL) && PICOTM_LIBM_HAVE_ASINL
PICOTM_EXPORT
long double
asinl_tx(long double x)
{
    MATHERR(long double, res, asinl(x), FE_INVALID | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ATAN) && PICOTM_LIBM_HAVE_ATAN
PICOTM_EXPORT
double
atan_tx(double x)
{
    MATHERR(double, res, atan(x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ATAN2) && PICOTM_LIBM_HAVE_ATAN2
PICOTM_EXPORT
double
atan2_tx(double y, double x)
{
    MATHERR(double, res, atan2(y, x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ATAN2F) && PICOTM_LIBM_HAVE_ATAN2F
PICOTM_EXPORT
float
atan2f_tx(float y, float x)
{
    MATHERR(float, res, atan2f(y, x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ATAN2L) && PICOTM_LIBM_HAVE_ATAN2L
PICOTM_EXPORT
long double
atan2l_tx(long double y, long double x)
{
    MATHERR(long double, res, atan2l(y, x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ATANF) && PICOTM_LIBM_HAVE_ATANF
PICOTM_EXPORT
float
atanf_tx(float x)
{
    MATHERR(float, res, atanf(x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ATANH) && PICOTM_LIBM_HAVE_ATANH
PICOTM_EXPORT
double
atanh_tx(double x)
{
    MATHERR(double, res, atanh(x), FE_INVALID | FE_DIVBYZERO | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ATANHF) && PICOTM_LIBM_HAVE_ATANHF
PICOTM_EXPORT
float
atanhf_tx(float x)
{
    MATHERR(float, res, atanhf(x), FE_INVALID | FE_DIVBYZERO | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ATANHL) && PICOTM_LIBM_HAVE_ATANHL
PICOTM_EXPORT
long double
atanhl_tx(long double x)
{
    MATHERR(long double, res, atanhl(x),
            FE_INVALID | FE_DIVBYZERO | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ATANL) && PICOTM_LIBM_HAVE_ATANL
PICOTM_EXPORT
long double
atanl_tx(long double x)
{
    MATHERR(long, res, atanl(x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_CBRT) && PICOTM_LIBM_HAVE_CBRT
PICOTM_EXPORT
double
cbrt_tx(double x)
{
    return cbrt(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CBRTF) && PICOTM_LIBM_HAVE_CBRTF
PICOTM_EXPORT
float
cbrtf_tx(float x)
{
    return cbrtf(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CBRTL) && PICOTM_LIBM_HAVE_CBRTL
PICOTM_EXPORT
long double
cbrtl_tx(long double x)
{
    return cbrtl(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CEIL) && PICOTM_LIBM_HAVE_CEIL
PICOTM_EXPORT
double
ceil_tx(double x)
{
    return ceil(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CEILF) && PICOTM_LIBM_HAVE_CEILF
PICOTM_EXPORT
float
ceilf_tx(float x)
{
    return ceilf(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_CEILL) && PICOTM_LIBM_HAVE_CEILL
PICOTM_EXPORT
long double
ceill_tx(long double x)
{
    return ceill(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_COPYSIGN) && PICOTM_LIBM_HAVE_COPYSIGN
PICOTM_EXPORT
double
copysign_tx(double x, double y)
{
    return copysign(x, y);
}
#endif

#if defined(PICOTM_LIBM_HAVE_COPYSIGNF) && PICOTM_LIBM_HAVE_COPYSIGNF
PICOTM_EXPORT
float
copysignf_tx(float x, float y)
{
    return copysignf(x, y);
}
#endif

#if defined(PICOTM_LIBM_HAVE_COPYSIGNL) && PICOTM_LIBM_HAVE_COPYSIGNL
PICOTM_EXPORT
long double
copysignl_tx(long double x, long double y)
{
    return copysignl(x, y);
}
#endif

#if defined(PICOTM_LIBM_HAVE_COS) && PICOTM_LIBM_HAVE_COS
PICOTM_EXPORT
double
cos_tx(double x)
{
    MATHERR(double, res, cos(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_COSF) && PICOTM_LIBM_HAVE_COSF
PICOTM_EXPORT
float
cosf_tx(float x)
{
    MATHERR(float, res, cosf(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_COSH) && PICOTM_LIBM_HAVE_COSH
PICOTM_EXPORT
double
cosh_tx(double x)
{
    MATHERR(double, res, cosh(x), FE_OVERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_COSHF) && PICOTM_LIBM_HAVE_COSHF
PICOTM_EXPORT
float
coshf_tx(float x)
{
    MATHERR(float, res, coshf(x), FE_OVERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_COSHL) && PICOTM_LIBM_HAVE_COSHL
PICOTM_EXPORT
long double
coshl_tx(long double x)
{
    MATHERR(long double, res, coshl(x), FE_OVERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_COSL) && PICOTM_LIBM_HAVE_COSL
PICOTM_EXPORT
long double
cosl_tx(long double x)
{
    MATHERR(long double, res, cosl(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ERF) && PICOTM_LIBM_HAVE_ERF
PICOTM_EXPORT
double
erf_tx(double x)
{
    MATHERR(double, res, erf(x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ERFC) && PICOTM_LIBM_HAVE_ERFC
PICOTM_EXPORT
double
erfc_tx(double x)
{
    MATHERR(double, res, erfc(x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ERFCF) && PICOTM_LIBM_HAVE_ERFCF
PICOTM_EXPORT
float
erfcf_tx(float x)
{
    MATHERR(float, res, erfcf(x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ERFCL) && PICOTM_LIBM_HAVE_ERFCL
PICOTM_EXPORT
long double
erfcl_tx(long double x)
{
    MATHERR(long double, res, erfcl(x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ERFF) && PICOTM_LIBM_HAVE_ERFF
PICOTM_EXPORT
float
erff_tx(float x)
{
    MATHERR(float, res, erff(x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ERFL) && PICOTM_LIBM_HAVE_ERFL
PICOTM_EXPORT
long double
erfl_tx(long double x)
{
    MATHERR(long double, res, erfl(x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_EXP) && PICOTM_LIBM_HAVE_EXP
PICOTM_EXPORT
double
exp_tx(double x)
{
    MATHERR(double, res, exp(x), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_EXP2) && PICOTM_LIBM_HAVE_EXP2
PICOTM_EXPORT
double
exp2_tx(double x)
{
    MATHERR(double, res, exp2(x), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_EXP2F) && PICOTM_LIBM_HAVE_EXP2F
PICOTM_EXPORT
float
exp2f_tx(float x)
{
    MATHERR(float, res, exp2f(x), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_EXP2L) && PICOTM_LIBM_HAVE_EXP2L
PICOTM_EXPORT
long double
exp2l_tx(long double x)
{
    MATHERR(long double, res, exp2l(x), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_EXPF) && PICOTM_LIBM_HAVE_EXPF
PICOTM_EXPORT
float
expf_tx(float x)
{
    MATHERR(float, res, expf(x), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_EXPL) && PICOTM_LIBM_HAVE_EXPL
PICOTM_EXPORT
long double
expl_tx(long double x)
{
    MATHERR(long double, res, expl(x), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_EXPM1) && PICOTM_LIBM_HAVE_EXPM1
PICOTM_EXPORT
double
expm1_tx(double x)
{
    MATHERR(double, res, expm1(x), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_EXPM1F) && PICOTM_LIBM_HAVE_EXPM1F
PICOTM_EXPORT
float
expm1f_tx(float x)
{
    MATHERR(float, res, expm1f(x), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_EXPM1L) && PICOTM_LIBM_HAVE_EXPM1L
PICOTM_EXPORT
long double
expm1l_tx(long double x)
{
    MATHERR(long double, res, expm1l(x), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_FABS) && PICOTM_LIBM_HAVE_FABS
PICOTM_EXPORT
double
fabs_tx(double x)
{
    return fabs(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_FABSF) && PICOTM_LIBM_HAVE_FABSF
PICOTM_EXPORT
float
fabsf_tx(float x)
{
    return fabsf(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_FABSL) && PICOTM_LIBM_HAVE_FABSL
PICOTM_EXPORT
long double
fabsl_tx(long double x)
{
    return fabsl(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_FDIM) && PICOTM_LIBM_HAVE_FDIM
PICOTM_EXPORT
double
fdim_tx(double x, double y)
{
    MATHERR(double, res, fdim(x, y), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_FDIMF) && PICOTM_LIBM_HAVE_FDIMF
PICOTM_EXPORT
float
fdimf_tx(float x, float y)
{
    MATHERR(float, res, fdimf(x, y), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_FDIML) && PICOTM_LIBM_HAVE_FDIML
PICOTM_EXPORT
long double
fdiml_tx(long double x, long double y)
{
    MATHERR(long double, res, fdiml(x, y), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_FLOOR) && PICOTM_LIBM_HAVE_FLOOR
PICOTM_EXPORT
double
floor_tx(double x)
{
    return floor(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_FLOORF) && PICOTM_LIBM_HAVE_FLOORF
PICOTM_EXPORT
float
floorf_tx(float x)
{
    return floorf(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_FLOORL) && PICOTM_LIBM_HAVE_FLOORL
PICOTM_EXPORT
long double
floorl_tx(long double x)
{
    return floorl(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_FMA) && PICOTM_LIBM_HAVE_FMA
PICOTM_EXPORT
double
fma_tx(double x, double y, double z)
{
    MATHERR(double, res, fma(x, y, z),
            FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_FMAF) && PICOTM_LIBM_HAVE_FMAF
PICOTM_EXPORT
float
fmaf_tx(float x, float y, float z)
{
    MATHERR(float, res, fmaf(x, y, z),
            FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_FMAL) && PICOTM_LIBM_HAVE_FMAL
PICOTM_EXPORT
long double
fmal_tx(long double x, long double y, long double z)
{
    MATHERR(long double, res, fmal(x, y, z),
            FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_FMAX) && PICOTM_LIBM_HAVE_FMAX
PICOTM_EXPORT
double
fmax_tx(double x, double y)
{
    return fmax(x, y);
}
#endif

#if defined(PICOTM_LIBM_HAVE_FMAXF) && PICOTM_LIBM_HAVE_FMAXF
PICOTM_EXPORT
float
fmaxf_tx(float x, float y)
{
    return fmaxf(x, y);
}
#endif

#if defined(PICOTM_LIBM_HAVE_FMAXL) && PICOTM_LIBM_HAVE_FMAXL
PICOTM_EXPORT
long double
fmaxl_tx(long double x, long double y)
{
    return fmaxl(x, y);
}
#endif

#if defined(PICOTM_LIBM_HAVE_FMIN) && PICOTM_LIBM_HAVE_FMIN
PICOTM_EXPORT
double
fmin_tx(double x, double y)
{
    return fmin(x, y);
}
#endif

#if defined(PICOTM_LIBM_HAVE_FMINF) && PICOTM_LIBM_HAVE_FMINF
PICOTM_EXPORT
float
fminf_tx(float x, float y)
{
    return fminf(x, y);
}
#endif

#if defined(PICOTM_LIBM_HAVE_FMINL) && PICOTM_LIBM_HAVE_FMINL
PICOTM_EXPORT
long double
fminl_tx(long double x, long double y)
{
    return fminl(x, y);
}
#endif

#if defined(PICOTM_LIBM_HAVE_FMOD) && PICOTM_LIBM_HAVE_FMOD
PICOTM_EXPORT
double
fmod_tx(double x, double y)
{
    MATHERR(double, res, fmod(x, y), FE_INVALID | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_FMODF) && PICOTM_LIBM_HAVE_FMODF
PICOTM_EXPORT
float
fmodf_tx(float x, float y)
{
    MATHERR(float, res, fmodf(x, y), FE_INVALID | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_FMODL) && PICOTM_LIBM_HAVE_FMODL
PICOTM_EXPORT
long double
fmodl_tx(long double x, long double y)
{
    MATHERR(long double, res, fmodl(x, y), FE_INVALID | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_FREXP) && PICOTM_LIBM_HAVE_FREXP
PICOTM_EXPORT
double
frexp_tx(double num, int* exp)
{
    privatize_int_tx(exp, PICOTM_TM_PRIVATIZE_STORE);
    return frexp_tm(num, exp);
}
#endif

#if defined(PICOTM_LIBM_HAVE_FREXPF) && PICOTM_LIBM_HAVE_FREXPF
PICOTM_EXPORT
float
frexpf_tx(float num, int* exp)
{
    privatize_int_tx(exp, PICOTM_TM_PRIVATIZE_STORE);
    return frexpf_tm(num, exp);
}
#endif

#if defined(PICOTM_LIBM_HAVE_FREXPL) && PICOTM_LIBM_HAVE_FREXPL
PICOTM_EXPORT
long double
frexpl_tx(long double num, int* exp)
{
    privatize_int_tx(exp, PICOTM_TM_PRIVATIZE_STORE);
    return frexpl_tm(num, exp);
}
#endif

#if defined(PICOTM_LIBM_HAVE_HYPOT) && PICOTM_LIBM_HAVE_HYPOT
PICOTM_EXPORT
double
hypot_tx(double x, double y)
{
    MATHERR(double, res, hypot(x, y), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_HYPOTF) && PICOTM_LIBM_HAVE_HYPOTF
PICOTM_EXPORT
float
hypotf_tx(float x, float y)
{
    MATHERR(float, res, hypotf(x, y), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_HYPOTL) && PICOTM_LIBM_HAVE_HYPOTL
PICOTM_EXPORT
long double
hypotl_tx(long double x, long double y)
{
    MATHERR(long double, res, hypotl(x, y), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ILOGB) && PICOTM_LIBM_HAVE_ILOGB
PICOTM_EXPORT
double
ilogb_tx(double x)
{
    MATHERR(double, res, ilogb(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ILOGBF) && PICOTM_LIBM_HAVE_ILOGBF
PICOTM_EXPORT
float
ilogbf_tx(float x)
{
    MATHERR(float, res, ilogbf(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_ILOGBL) && PICOTM_LIBM_HAVE_ILOGBL
PICOTM_EXPORT
long double
ilogbl_tx(long double x)
{
    MATHERR(long double, res, ilogbl(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_J0) && PICOTM_LIBM_HAVE_J0
PICOTM_EXPORT
double
j0_tx(double x)
{
    MATHERR(double, res, j0(x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_J1) && PICOTM_LIBM_HAVE_J1
PICOTM_EXPORT
double
j1_tx(double x)
{
    MATHERR(double, res, j1(x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_JN) && PICOTM_LIBM_HAVE_JN
PICOTM_EXPORT
double
jn_tx(int n, double x)
{
    MATHERR(double, res, jn(n, x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LDEXP) && PICOTM_LIBM_HAVE_LDEXP
PICOTM_EXPORT
double
ldexp_tx(double x, int exp)
{
    MATHERR(double, res, ldexp(x, exp), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LDEXPF) && PICOTM_LIBM_HAVE_LDEXPF
PICOTM_EXPORT
float
ldexpf_tx(float x, int exp)
{
    MATHERR(float, res, ldexpf(x, exp), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LDEXPL) && PICOTM_LIBM_HAVE_LDEXPL
PICOTM_EXPORT
long double
ldexpl_tx(long double x, int exp)
{
    MATHERR(long double, res, ldexpl(x, exp), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LGAMMA) && PICOTM_LIBM_HAVE_LGAMMA
PICOTM_EXPORT
double
lgamma_tx(double x)
{
    privatize_int_tx(&signgam, PICOTM_TM_PRIVATIZE_STORE);
    MATHERR(double, res, lgamma(x), FE_DIVBYZERO | FE_OVERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LGAMMAF) && PICOTM_LIBM_HAVE_LGAMMAF
PICOTM_EXPORT
float
lgammaf_tx(float x)
{
    privatize_int_tx(&signgam, PICOTM_TM_PRIVATIZE_STORE);
    MATHERR(float, res, lgammaf(x), FE_DIVBYZERO | FE_OVERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LGAMMAL) && PICOTM_LIBM_HAVE_LGAMMAL
PICOTM_EXPORT
long double
lgammal_tx(long double x)
{
    privatize_int_tx(&signgam, PICOTM_TM_PRIVATIZE_STORE);
    MATHERR(long double, res, lgammal(x), FE_DIVBYZERO | FE_OVERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LLRINT) && PICOTM_LIBM_HAVE_LLRINT
PICOTM_EXPORT
long long
llrint_tx(double x)
{
    MATHERR(long long, res, llrint(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LLRINTF) && PICOTM_LIBM_HAVE_LLRINTF
PICOTM_EXPORT
long long
llrintf_tx(float x)
{
    MATHERR(long long, res, llrintf(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LLRINTL) && PICOTM_LIBM_HAVE_LLRINTL
PICOTM_EXPORT
long long
llrintl_tx(long double x)
{
    MATHERR(long long, res, llrintl(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LLROUND) && PICOTM_LIBM_HAVE_LLROUND
PICOTM_EXPORT
long long
llround_tx(double x)
{
    MATHERR(long long, res, llround(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LLROUNDF) && PICOTM_LIBM_HAVE_LLROUNDF
PICOTM_EXPORT
long long
llroundf_tx(float x)
{
    MATHERR(long long, res, llroundf(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LLROUNDL) && PICOTM_LIBM_HAVE_LLROUNDL
PICOTM_EXPORT
long long
llroundl_tx(long double x)
{
    MATHERR(long long, res, llroundl(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LOG) && PICOTM_LIBM_HAVE_LOG
PICOTM_EXPORT
double
log_tx(double x)
{
    MATHERR(double, res, log(x), FE_INVALID | FE_DIVBYZERO);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LOG10) && PICOTM_LIBM_HAVE_LOG10
PICOTM_EXPORT
double
log10_tx(double x)
{
    MATHERR(double, res, log10(x), FE_INVALID | FE_DIVBYZERO);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LOG10F) && PICOTM_LIBM_HAVE_LOG10F
PICOTM_EXPORT
float
log10f_tx(float x)
{
    MATHERR(float, res, log10f(x), FE_INVALID | FE_DIVBYZERO);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LOG10L) && PICOTM_LIBM_HAVE_LOG10L
PICOTM_EXPORT
long double
log10l_tx(long double x)
{
    MATHERR(long double, res, log10l(x), FE_INVALID | FE_DIVBYZERO);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LOG1P) && PICOTM_LIBM_HAVE_LOG1P
PICOTM_EXPORT
double
log1p_tx(double x)
{
    MATHERR(double, res, log1p(x), FE_INVALID | FE_DIVBYZERO | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LOG1PF) && PICOTM_LIBM_HAVE_LOG1PF
PICOTM_EXPORT
float
log1pf_tx(float x)
{
    MATHERR(float, res, log1pf(x), FE_INVALID | FE_DIVBYZERO | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LOG1PL) && PICOTM_LIBM_HAVE_LOG1PL
PICOTM_EXPORT
long double
log1pl_tx(long double x)
{
    MATHERR(long double, res, log1pl(x),
            FE_INVALID | FE_DIVBYZERO | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LOG2) && PICOTM_LIBM_HAVE_LOG2
PICOTM_EXPORT
double
log2_tx(double x)
{
    MATHERR(double, res, log2(x), FE_INVALID | FE_DIVBYZERO);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LOG2F) && PICOTM_LIBM_HAVE_LOG2F
PICOTM_EXPORT
float
log2f_tx(float x)
{
    MATHERR(float, res, log2f(x), FE_INVALID | FE_DIVBYZERO);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LOG2L) && PICOTM_LIBM_HAVE_LOG2L
PICOTM_EXPORT
long double
log2l_tx(long double x)
{
    MATHERR(long double, res, log2l(x), FE_INVALID | FE_DIVBYZERO);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LOGB) && PICOTM_LIBM_HAVE_LOGB
PICOTM_EXPORT
double
logb_tx(double x)
{
    MATHERR(double, res, logb(x), FE_DIVBYZERO);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LOGBF) && PICOTM_LIBM_HAVE_LOGBF
PICOTM_EXPORT
float
logbf_tx(float x)
{
    MATHERR(float, res, logbf(x), FE_DIVBYZERO);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LOGBL) && PICOTM_LIBM_HAVE_LOGBL
PICOTM_EXPORT
long double
logbl_tx(long double x)
{
    MATHERR(long double, res, logbl(x), FE_DIVBYZERO);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LOGF) && PICOTM_LIBM_HAVE_LOGF
PICOTM_EXPORT
float
logf_tx(float x)
{
    MATHERR(float, res, logf(x), FE_INVALID | FE_DIVBYZERO);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LOGL) && PICOTM_LIBM_HAVE_LOGL
PICOTM_EXPORT
long double
logl_tx(long double x)
{
    MATHERR(long double, res, logl(x), FE_INVALID | FE_DIVBYZERO);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LRINT) && PICOTM_LIBM_HAVE_LRINT
PICOTM_EXPORT
long
lrint_tx(double x)
{
    MATHERR(long, res, lrint(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LRINTF) && PICOTM_LIBM_HAVE_LRINTF
PICOTM_EXPORT
long
lrintf_tx(float x)
{
    MATHERR(long, res, lrintf(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LRINTL) && PICOTM_LIBM_HAVE_LRINTL
PICOTM_EXPORT
long
lrintl_tx(long double x)
{
    MATHERR(long, res, lrintl(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LROUND) && PICOTM_LIBM_HAVE_LROUND
PICOTM_EXPORT
long
lround_tx(double x)
{
    MATHERR(long, res, lround(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LROUNDF) && PICOTM_LIBM_HAVE_LROUNDF
PICOTM_EXPORT
long
lroundf_tx(float x)
{
    MATHERR(long, res, lroundf(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_LROUNDL) && PICOTM_LIBM_HAVE_LROUNDL
PICOTM_EXPORT
long
lroundl_tx(long double x)
{
    MATHERR(long, res, lroundl(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_MODF) && PICOTM_LIBM_HAVE_MODF
PICOTM_EXPORT
double
modf_tx(double x, double* iptr)
{
    privatize_double_tx(iptr, PICOTM_TM_PRIVATIZE_STORE);
    return modf_tm(x, iptr);
}
#endif

#if defined(PICOTM_LIBM_HAVE_MODFF) && PICOTM_LIBM_HAVE_MODFF
PICOTM_EXPORT
float
modff_tx(float x, float* iptr)
{
    privatize_float_tx(iptr, PICOTM_TM_PRIVATIZE_STORE);
    return modff_tm(x, iptr);
}
#endif

#if defined(PICOTM_LIBM_HAVE_MODFL) && PICOTM_LIBM_HAVE_MODFL
PICOTM_EXPORT
long double
modfl_tx(long double x, long double* iptr)
{
    privatize_ldouble_tx(iptr, PICOTM_TM_PRIVATIZE_STORE);
    return modfl_tm(x, iptr);
}
#endif

#if defined(PICOTM_LIBM_HAVE_NAN) && PICOTM_LIBM_HAVE_NAN
PICOTM_EXPORT
double
nan_tx(const char* tagp)
{
    privatize_c_tx(tagp, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return nan_tm(tagp);
}
#endif

#if defined(PICOTM_LIBM_HAVE_NANF) && PICOTM_LIBM_HAVE_NANF
PICOTM_EXPORT
float
nanf_tx(const char* tagp)
{
    privatize_c_tx(tagp, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return nanf_tm(tagp);
}
#endif

#if defined(PICOTM_LIBM_HAVE_NANL) && PICOTM_LIBM_HAVE_NANL
PICOTM_EXPORT
long double
nanl_tx(const char* tagp)
{
    privatize_c_tx(tagp, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return nanl_tm(tagp);
}
#endif

#if defined(PICOTM_LIBM_HAVE_NEARBYINT) && PICOTM_LIBM_HAVE_NEARBYINT
PICOTM_EXPORT
double
nearbyint_tx(double x)
{
    return nearbyint(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_NEARBYINTF) && PICOTM_LIBM_HAVE_NEARBYINTF
PICOTM_EXPORT
float
nearbyintf_tx(float x)
{
    return nearbyintf(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_NEARBYINTL) && PICOTM_LIBM_HAVE_NEARBYINTL
PICOTM_EXPORT
long double
nearbyintl_tx(long double x)
{
    return nearbyintl(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_NEXTAFTER) && PICOTM_LIBM_HAVE_NEXTAFTER
PICOTM_EXPORT
double
nextafter_tx(double x, double y)
{
    MATHERR(double, res, nextafter(x, y), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_NEXTAFTERF) && PICOTM_LIBM_HAVE_NEXTAFTERF
PICOTM_EXPORT
float
nextafterf_tx(float x, float y)
{
    MATHERR(float, res, nextafterf(x, y), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_NEXTAFTERL) && PICOTM_LIBM_HAVE_NEXTAFTERL
PICOTM_EXPORT
long double
nextafterl_tx(long double x, long double y)
{
    MATHERR(long double, res, nextafterl(x, y), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_NEXTTOWARD) && PICOTM_LIBM_HAVE_NEXTTOWARD
PICOTM_EXPORT
double
nexttoward_tx(double x, long double y)
{
    MATHERR(double, res, nexttoward(x, y), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_NEXTTOWARDF) && PICOTM_LIBM_HAVE_NEXTTOWARDF
PICOTM_EXPORT
float
nexttowardf_tx(float x, long double y)
{
    MATHERR(float, res, nexttowardf(x, y), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_NEXTTOWARDL) && PICOTM_LIBM_HAVE_NEXTTOWARDL
PICOTM_EXPORT
long double
nexttowardl_tx(long double x, long double y)
{
    MATHERR(long double, res, nexttowardl(x, y), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_POW) && PICOTM_LIBM_HAVE_POW
PICOTM_EXPORT
double
pow_tx(double x, double y)
{
    MATHERR(double, res, pow(x, y),
            FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_POWF) && PICOTM_LIBM_HAVE_POWF
PICOTM_EXPORT
float
powf_tx(float x, float y)
{
    MATHERR(float, res, powf(x, y),
            FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_POWL) && PICOTM_LIBM_HAVE_POWL
PICOTM_EXPORT
long double
powl_tx(long double x, long double y)
{
    MATHERR(long double, res, powl(x, y),
            FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_REMAINDER) && PICOTM_LIBM_HAVE_REMAINDER
PICOTM_EXPORT
double
remainder_tx(double x, double y)
{
    MATHERR(double, res, remainder(x, y), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_REMAINDERF) && PICOTM_LIBM_HAVE_REMAINDERF
PICOTM_EXPORT
float
remainderf_tx(float x, float y)
{
    MATHERR(float, res, remainderl(x, y), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_REMAINDERL) && PICOTM_LIBM_HAVE_REMAINDERL
PICOTM_EXPORT
long double
remainderl_tx(long double x, long double y)
{
    MATHERR(long double, res, remainderl(x, y), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_REMQUO) && PICOTM_LIBM_HAVE_REMQUO
PICOTM_EXPORT
double
remquo_tx(double x, double y, int* quo)
{
    privatize_int_tx(quo, PICOTM_TM_PRIVATIZE_STORE);
    return remquo_tm(x, y, quo);
}
#endif

#if defined(PICOTM_LIBM_HAVE_REMQUOF) && PICOTM_LIBM_HAVE_REMQUOF
PICOTM_EXPORT
float
remquof_tx(float x, float y, int* quo)
{
    privatize_int_tx(quo, PICOTM_TM_PRIVATIZE_STORE);
    return remquof_tm(x, y, quo);
}
#endif

#if defined(PICOTM_LIBM_HAVE_REMQUOL) && PICOTM_LIBM_HAVE_REMQUOL
PICOTM_EXPORT
long double
remquol_tx(long double x, long double y, int* quo)
{
    privatize_int_tx(quo, PICOTM_TM_PRIVATIZE_STORE);
    return remquol_tm(x, y, quo);
}
#endif

#if defined(PICOTM_LIBM_HAVE_RINT) && PICOTM_LIBM_HAVE_RINT
PICOTM_EXPORT
double
rint_tx(double x)
{
    return rint(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_RINTF) && PICOTM_LIBM_HAVE_RINTF
PICOTM_EXPORT
float
rintf_tx(float x)
{
    return rintf(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_RINTL) && PICOTM_LIBM_HAVE_RINTL
PICOTM_EXPORT
long double
rintl_tx(long double x)
{
    return rintl(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_ROUND) && PICOTM_LIBM_HAVE_ROUND
PICOTM_EXPORT
double
round_tx(double x)
{
    return round(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_ROUNDF) && PICOTM_LIBM_HAVE_ROUNDF
PICOTM_EXPORT
float
roundf_tx(float x)
{
    return roundf(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_ROUNDL) && PICOTM_LIBM_HAVE_ROUNDL
PICOTM_EXPORT
long double
roundl_tx(long double x)
{
    return roundl(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_SCALBLN) && PICOTM_LIBM_HAVE_SCALBLN
PICOTM_EXPORT
double
scalbln_tx(double x, long n)
{
    MATHERR(double, res, scalbln(x, n), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_SCALBLNF) && PICOTM_LIBM_HAVE_SCALBLNF
PICOTM_EXPORT
float
scalblnf_tx(float x, long n)
{
    MATHERR(float, res, scalblnf(x, n), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_SCALBLNL) && PICOTM_LIBM_HAVE_SCALBLNL
PICOTM_EXPORT
long double
scalblnl_tx(long double x, long n)
{
    MATHERR(long double, res, scalblnl(x, n), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_SCALBN) && PICOTM_LIBM_HAVE_SCALBN
PICOTM_EXPORT
double
scalbn_tx(double x, int n)
{
    MATHERR(double, res, scalbn(x, n), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_SCALBNF) && PICOTM_LIBM_HAVE_SCALBNF
PICOTM_EXPORT
float
scalbnf_tx(float x, int n)
{
    MATHERR(float, res, scalbnf(x, n), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_SCALBNL) && PICOTM_LIBM_HAVE_SCALBNL
PICOTM_EXPORT
long double
scalbnl_tx(long double x, int n)
{
    MATHERR(long double, res, scalbnl(x, n), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_SIN) && PICOTM_LIBM_HAVE_SIN
PICOTM_EXPORT
double
sin_tx(double x)
{
    MATHERR(double, res, sin(x), FE_INVALID | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_SINF) && PICOTM_LIBM_HAVE_SINF
PICOTM_EXPORT
float
sinf_tx(float x)
{
    MATHERR(float, res, sinf(x), FE_INVALID | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_SINH) && PICOTM_LIBM_HAVE_SINH
PICOTM_EXPORT
double
sinh_tx(double x)
{
    MATHERR(double, res, sinh(x), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_SINHF) && PICOTM_LIBM_HAVE_SINHF
PICOTM_EXPORT
float
sinhf_tx(float x)
{
    MATHERR(float, res, sinhf(x), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_SINHL) && PICOTM_LIBM_HAVE_SINHL
PICOTM_EXPORT
long double
sinhl_tx(long double x)
{
    MATHERR(long double, res, sinhl(x), FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_SINL) && PICOTM_LIBM_HAVE_SINL
PICOTM_EXPORT
long double
sinl_tx(long double x)
{
    MATHERR(long double, res, sinl(x), FE_INVALID | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_SQRT) && PICOTM_LIBM_HAVE_SQRT
PICOTM_EXPORT
double
sqrt_tx(double x)
{
    MATHERR(double, res, sqrt(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_SQRTF) && PICOTM_LIBM_HAVE_SQRTF
PICOTM_EXPORT
float
sqrtf_tx(float x)
{
    MATHERR(float, res, sqrtf(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_SQRTL) && PICOTM_LIBM_HAVE_SQRTL
PICOTM_EXPORT
long double
sqrtl_tx(long double x)
{
    MATHERR(long double, res, sqrtl(x), FE_INVALID);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_TAN) && PICOTM_LIBM_HAVE_TAN
PICOTM_EXPORT
double
tan_tx(double x)
{
    MATHERR(double, res, tan(x), FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_TANF) && PICOTM_LIBM_HAVE_TANF
PICOTM_EXPORT
float
tanf_tx(float x)
{
    MATHERR(float, res, tanf(x), FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_TANH) && PICOTM_LIBM_HAVE_TANH
PICOTM_EXPORT
double
tanh_tx(double x)
{
    MATHERR(double, res, tanh(x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_TANHF) && PICOTM_LIBM_HAVE_TANHF
PICOTM_EXPORT
float
tanhf_tx(float x)
{
    MATHERR(float, res, tanhf(x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_TANHL) && PICOTM_LIBM_HAVE_TANHL
PICOTM_EXPORT
long double
tanhl_tx(long double x)
{
    MATHERR(long double, res, tanhl(x), FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_TANL) && PICOTM_LIBM_HAVE_TANL
PICOTM_EXPORT
long double
tanl_tx(long double x)
{
    MATHERR(long double, res, tanl(x), FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_TGAMMA) && PICOTM_LIBM_HAVE_TGAMMA
PICOTM_EXPORT
double
tgamma_tx(double x)
{
    MATHERR(double, res, tgamma(x),
            FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_TGAMMAF) && PICOTM_LIBM_HAVE_TGAMMAF
PICOTM_EXPORT
float
tgammaf_tx(float x)
{
    MATHERR(float, res, tgammaf(x),
            FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_TGAMMAL) && PICOTM_LIBM_HAVE_TGAMMAL
PICOTM_EXPORT
long double
tgammal_tx(long double x)
{
    MATHERR(long double, res, tgammal(x),
            FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_TRUNC) && PICOTM_LIBM_HAVE_TRUNC
PICOTM_EXPORT
double
trunc_tx(double x)
{
    return trunc(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_TRUNCF) && PICOTM_LIBM_HAVE_TRUNCF
PICOTM_EXPORT
float
truncf_tx(float x)
{
    return truncf(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_TRUNCL) && PICOTM_LIBM_HAVE_TRUNCL
PICOTM_EXPORT
long double
truncl_tx(long double x)
{
    return truncl(x);
}
#endif

#if defined(PICOTM_LIBM_HAVE_Y0) && PICOTM_LIBM_HAVE_Y0
PICOTM_EXPORT
double
y0_tx(double x)
{
    MATHERR(double, res, y0(x),
            FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_Y1) && PICOTM_LIBM_HAVE_Y1
PICOTM_EXPORT
double
y1_tx(double x)
{
    MATHERR(double, res, y1(x),
            FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif

#if defined(PICOTM_LIBM_HAVE_YN) && PICOTM_LIBM_HAVE_YN
PICOTM_EXPORT
double
yn_tx(int n, double x)
{
    MATHERR(double, res, yn(n, x),
            FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);
    return res;
}
#endif
