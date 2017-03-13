dnl This Source Code Form is subject to the terms of the Mozilla Public
dnl License, v. 2.0. If a copy of the MPL was not distributed with this
dnl file, You can obtain one at http://mozilla.org/MPL/2.0/.

AC_PREREQ([2.69])
AC_INIT([systx], [0.0.1])

AC_CONFIG_SRCDIR([src/Makefile.am])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AC_USE_SYSTEM_EXTENSIONS

AC_PROG_CC_C99
AM_PROG_AR
AC_PROG_LIBTOOL

AX_PTHREAD

AC_CHECK_HEADERS([sys/cdefs.h])

AX_CHECK_COMPILE_FLAG([-fvisibility=hidden],
                      [VISIBILITY_CFLAGS="-fvisibility=hidden"],
                      [VISIBILITY_CFLAGS=""],
                      [$PTHREAD_CFLAGS])

AC_SUBST(VISIBILITY_CFLAGS, [$VISIBILITY_CFLAGS])

AC_DEFINE([MAXNUMFD], [1024])
AC_DEFINE([RECSIZE],  [32])

AC_CONFIG_FILES([Makefile
                 include/Makefile
                 src/Makefile
                 src/tm/Makefile])

AC_OUTPUT
