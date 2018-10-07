#
# SYNOPSIS
#
#   CONFIG_LIBPTHREAD
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

AC_DEFUN([_CHECK_LIBPTHREAD_PTHREAD_H], [
    AC_CHECK_HEADERS([pthread.h])
    AS_VAR_IF([ac_cv_header_pthread_h], [yes], [

        #
        # Types
        #

        _CHECK_MODULE_TYPE([libpthread], [pthread_t], [[@%:@include <pthread.h>]])

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libpthread], [pthread_equal], [[@%:@include <pthread.h>]])
        _CHECK_MODULE_INTF([libpthread], [pthread_self],  [[@%:@include <pthread.h>]])
    ])
])

AC_DEFUN([_CONFIG_LIBPTHREAD], [
    AC_REQUIRE([AX_PTHREAD])
    AS_VAR_IF([ax_pthread_ok], [yes], [
        _CHECK_LIBPTHREAD_PTHREAD_H
    ])
])

AC_DEFUN([CONFIG_LIBPTHREAD], [
    AC_ARG_ENABLE([module-libpthread],
                  [AS_HELP_STRING([--enable-module-libpthread],
                                  [enable POSIX Threads module @<:@default=yes@:>@])],
                  [enable_module_libpthread=$enableval],
                  [enable_module_libpthread=yes])
    AM_CONDITIONAL([ENABLE_MODULE_LIBPTHREAD],
                   [test "x$enable_module_libpthread" = "xyes"])
    AS_VAR_IF([enable_module_libpthread], [yes], [
        _CONFIG_LIBPTHREAD
    ])
])
