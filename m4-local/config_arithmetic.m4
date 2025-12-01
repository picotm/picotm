#
# SYNOPSIS
#
#   CONFIG_ARITHMETIC
#
# LICENSE
#
#   picotm - A system-level transaction manager
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Lesser General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
#   SPDX-License-Identifier: LGPL-3.0-or-later
#

AC_DEFUN([_CONFIG_ARITHMETIC], [

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
])

AC_DEFUN([CONFIG_ARITHMETIC], [
    AC_ARG_ENABLE([module-arithmetic],
                  [AS_HELP_STRING([--enable-module-arithmetic],
                                  [enable arithmetic module @<:@default=yes@:>@])],
                  [enable_module_arithmetic=$enableval],
                  [enable_module_arithmetic=yes])
    AM_CONDITIONAL([ENABLE_MODULE_ARITHMETIC],
                   [test "x$enable_module_arithmetic" = "xyes"])
    AS_VAR_IF([enable_module_arithmetic], [yes], [
        _CONFIG_ARITHMETIC
    ])
])
