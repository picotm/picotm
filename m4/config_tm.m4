#
# SYNOPSIS
#
#   CONFIG_TM
#
# LICENSE
#
#   picotm - A system-level transaction manager
#   Copyright (c) 2017-2018 Thomas Zimmermann <contact@tzimmermann.org>
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
