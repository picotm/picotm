#
# SYNOPSIS
#
#   CONFIG_ARITHMETIC
#
# LICENSE
#
#   Copyright (c) 2018  Thomas Zimmermann <tdz@users.sourceforge.net>
#
#   Copying and distribution of this file, with or without modification,
#   are permitted in any medium without royalty provided the copyright
#   notice and this notice are preserved.  This file is offered as-is,
#   without any warranty.

AC_DEFUN([CONFIG_ARITHMETIC], [

    #
    # Types
    #

    _CHECK_MODULE_TYPE([arithmetic], [_Bool],              [[]])
    _CHECK_MODULE_TYPE([arithmetic], [char],               [[]])
    _CHECK_MODULE_TYPE([arithmetic], [double],             [[]])
    _CHECK_MODULE_TYPE([arithmetic], [float],              [[]])
    _CHECK_MODULE_TYPE([arithmetic], [int],                [[]])
    _CHECK_MODULE_TYPE([arithmetic], [long],               [[]])
    _CHECK_MODULE_TYPE([arithmetic], [long double],        [[]])
    _CHECK_MODULE_TYPE([arithmetic], [long long],          [[]])
    _CHECK_MODULE_TYPE([arithmetic], [short],              [[]])
    _CHECK_MODULE_TYPE([arithmetic], [signed char],        [[]])
    _CHECK_MODULE_TYPE([arithmetic], [unsigned char],      [[]])
    _CHECK_MODULE_TYPE([arithmetic], [unsigned int],       [[]])
    _CHECK_MODULE_TYPE([arithmetic], [unsigned long],      [[]])
    _CHECK_MODULE_TYPE([arithmetic], [unsigned long long], [[]])
    _CHECK_MODULE_TYPE([arithmetic], [unsigned short],     [[]])

    CONFIG_TEST([modules/arithmetic/tests/pubapi/arithmetic-pubapi-valgrind-t1.test])
])
