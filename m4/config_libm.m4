#
# SYNOPSIS
#
#   CONFIG_LIBM
#
# LICENSE
#
#   Copyright (c) 2017 Thomas Zimmermann <tdz@users.sourceforge.net>
#
#   Copying and distribution of this file, with or without modification,
#   are permitted in any medium without royalty provided the copyright
#   notice and this notice are preserved.  This file is offered as-is,
#   without any warranty.

AC_DEFUN([_CHECK_LIBM_COMPLEX_H], [

    AC_CHECK_HEADERS([complex.h])

    if test "x$ac_cv_header_complex_h" != "xno"; then

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libm], [cabs],    [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cabsf],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cabsl],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cacos],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cacosf],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cacosh],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cacoshf], [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cacoshl], [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cacosl],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [carg],    [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cargf],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cargl],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [casin],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [casinf],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [casinh],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [casinhf], [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [casinhl], [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [casinl],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [catan],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [catanf],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [catanh],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [catanhf], [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [catanhl], [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [catanl],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [ccos],    [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [ccosf],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [ccosh],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [ccoshf],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [ccoshl],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [ccosl],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cexp],    [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cexpf],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cexpl],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cimag],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cimagf],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cimagl],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [clog],    [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [clogf],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [clogl],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [conj],    [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [conjf],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [conjl],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cpow],    [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cpowf],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cpowl],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cproj],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cprojf],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [cprojl],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [creal],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [crealf],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [creall],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [csin],    [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [csinf],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [csinh],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [csinhf],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [csinhl],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [csinl],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [csqrt],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [csqrtf],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [csqrtl],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [ctan],    [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [ctanf],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [ctanh],   [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [ctanhf],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [ctanhl],  [[#include <complex.h>]])
        _CHECK_MODULE_INTF([libm], [ctanl],   [[#include <complex.h>]])
    fi
])

AC_DEFUN([CONFIG_LIBM], [
    AC_CHECK_LIB([m], [signgam])
    if test "x$ac_cv_lib_m_signam" != "xno"; then
        _CHECK_LIBM_COMPLEX_H
    fi
])
