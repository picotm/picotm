dnl This Source Code Form is subject to the terms of the Mozilla Public
dnl License, v. 2.0. If a copy of the MPL was not distributed with this
dnl file, You can obtain one at http://mozilla.org/MPL/2.0/.

AC_PREREQ([2.69])
AC_INIT([systx-tests], [0.0.1])

AC_CONFIG_SRCDIR([src/Makefile.am])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_USE_SYSTEM_EXTENSIONS

AC_PROG_CC_C99

AX_PTHREAD

AC_CHECK_HEADERS([sys/cdefs.h])

CHECK_SYSTX
CHECK_SYSTX_TM

AC_CONFIG_FILES([Makefile
                 src/Makefile])

AC_OUTPUT
