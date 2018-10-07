#
# SYNOPSIS
#
#   CONFIG_TXLIB
#
# LICENSE
#
#   Copyright (c) 2017-2018 Thomas Zimmermann <contact@tzimmermann.org>
#
#   Copying and distribution of this file, with or without modification,
#   are permitted in any medium without royalty provided the copyright
#   notice and this notice are preserved.  This file is offered as-is,
#   without any warranty.

AC_DEFUN([_CONFIG_TXLIB], [
])

AC_DEFUN([CONFIG_TXLIB], [
    AC_ARG_ENABLE([module-txlib],
                  [AS_HELP_STRING([--enable-module-txlib],
                                  [enable txlib module @<:@default=yes@:>@])],
                  [enable_module_txlib=$enableval],
                  [enable_module_txlib=yes])
    AM_CONDITIONAL([ENABLE_MODULE_TXLIB],
                   [test "x$enable_module_txlib" = "xyes"])
    AS_VAR_IF([enable_module_txlib], [yes], [
        _CONFIG_TXLIB
    ])
])
