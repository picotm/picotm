#
# SYNOPSIS
#
#   CONFIG_PTRDATA
#
# LICENSE
#
#   picotm - A system-level transaction manager
#   Copyright (c) 2018  Thomas Zimmermann <contact@tzimmermann.org>
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

AC_DEFUN([_CONFIG_PTRDATA], [
])

AC_DEFUN([CONFIG_PTRDATA], [
    AC_ARG_ENABLE([module-ptrdata],
                  [AS_HELP_STRING([--enable-module-ptrdata],
                                  [enable ptrdata module @<:@default=yes@:>@])],
                  [enable_module_ptrdata=$enableval],
                  [enable_module_ptrdata=yes])
    AM_CONDITIONAL([ENABLE_MODULE_PTRDATA],
                   [test "x$enable_module_ptrdata" = "xyes"])
    AS_VAR_IF([enable_module_ptrdata], [yes], [
        _CONFIG_PTRDATA
    ])
])
