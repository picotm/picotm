#
# SYNOPSIS
#
#   CONFIG_CAST
#
# LICENSE
#
#   Copyright (c) 2018  Thomas Zimmermann <tdz@users.sourceforge.net>
#
#   Copying and distribution of this file, with or without modification,
#   are permitted in any medium without royalty provided the copyright
#   notice and this notice are preserved.  This file is offered as-is,
#   without any warranty.

AC_DEFUN([_CONFIG_CAST], [

    #
    # Types
    #

    _CHECK_MODULE_TYPE([cast], [_Bool],              [[]])
    _CHECK_MODULE_TYPE([cast], [char],               [[]])
    _CHECK_MODULE_TYPE([cast], [double],             [[]])
    _CHECK_MODULE_TYPE([cast], [float],              [[]])
    _CHECK_MODULE_TYPE([cast], [int],                [[]])
    _CHECK_MODULE_TYPE([cast], [long],               [[]])
    _CHECK_MODULE_TYPE([cast], [long double],        [[]])
    _CHECK_MODULE_TYPE([cast], [long long],          [[]])
    _CHECK_MODULE_TYPE([cast], [short],              [[]])
    _CHECK_MODULE_TYPE([cast], [signed char],        [[]])
    _CHECK_MODULE_TYPE([cast], [unsigned char],      [[]])
    _CHECK_MODULE_TYPE([cast], [unsigned int],       [[]])
    _CHECK_MODULE_TYPE([cast], [unsigned long],      [[]])
    _CHECK_MODULE_TYPE([cast], [unsigned long long], [[]])
    _CHECK_MODULE_TYPE([cast], [unsigned short],     [[]])

    CONFIG_TEST([modules/cast/tests/pubapi/cast-pubapi-valgrind-t1.test])
])

AC_DEFUN([CONFIG_CAST], [
    AC_ARG_ENABLE([module-cast],
                  [AS_HELP_STRING([--enable-module-cast],
                                  [enable type-casting module @<:@default=yes@:>@])],
                  [enable_module_cast=$enableval],
                  [enable_module_cast=yes])
    AM_CONDITIONAL([ENABLE_MODULE_CAST],
                   [test "x$enable_module_cast" = "xyes"])
    AS_VAR_IF([enable_module_cast], [yes], [
        _CONFIG_CAST
    ])
])
