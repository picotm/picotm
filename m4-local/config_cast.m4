#
# SYNOPSIS
#
#   CONFIG_CAST
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
