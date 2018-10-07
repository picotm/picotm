#
# SYNOPSIS
#
#   CONFIG_TM
#
# LICENSE
#
#   Copyright (c) 2017-2018 Thomas Zimmermann <contact@tzimmermann.org>
#
#   Copying and distribution of this file, with or without modification,
#   are permitted in any medium without royalty provided the copyright
#   notice and this notice are preserved.  This file is offered as-is,
#   without any warranty.

AC_DEFUN([_CONFIG_TM], [

    #
    # Types
    #

    _CHECK_MODULE_TYPE([tm], [_Bool],              [[]])
    _CHECK_MODULE_TYPE([tm], [char],               [[]])
    _CHECK_MODULE_TYPE([tm], [double],             [[]])
    _CHECK_MODULE_TYPE([tm], [float],              [[]])
    _CHECK_MODULE_TYPE([tm], [int],                [[]])
    _CHECK_MODULE_TYPE([tm], [long],               [[]])
    _CHECK_MODULE_TYPE([tm], [long double],        [[]])
    _CHECK_MODULE_TYPE([tm], [long long],          [[]])
    _CHECK_MODULE_TYPE([tm], [short],              [[]])
    _CHECK_MODULE_TYPE([tm], [signed char],        [[]])
    _CHECK_MODULE_TYPE([tm], [unsigned char],      [[]])
    _CHECK_MODULE_TYPE([tm], [unsigned int],       [[]])
    _CHECK_MODULE_TYPE([tm], [unsigned long],      [[]])
    _CHECK_MODULE_TYPE([tm], [unsigned long long], [[]])
    _CHECK_MODULE_TYPE([tm], [unsigned short],     [[]])
])

AC_DEFUN([CONFIG_TM], [
    AC_ARG_ENABLE([module-tm],
                  [AS_HELP_STRING([--enable-module-tm],
                                  [enable Transactional Memory module @<:@default=yes@:>@])],
                  [enable_module_tm=$enableval],
                  [enable_module_tm=yes])
    AM_CONDITIONAL([ENABLE_MODULE_TM],
                   [test "x$enable_module_tm" = "xyes"])
    AS_VAR_IF([enable_module_tm], [yes], [
        _CONFIG_TM
    ])
])
