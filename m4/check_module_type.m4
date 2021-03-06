#
# SYNOPSIS
#
#   _CHECK_MODULE_TYPE
#
# LICENSE
#
#   picotm - A system-level transaction manager
#   Copyright (c) 2017  Thomas Zimmermann <contact@tzimmermann.org>
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

AC_DEFUN([_CHECK_MODULE_TYPE], [
    AC_CHECK_TYPE([$2],
                  [AC_DEFINE(AS_TR_CPP([PICOTM_$1_HAVE_TYPE_$2]),
                             [1],
                             [Define to 1 if the system has the type `$2'.])],
                  [],
                  [$3])
])
