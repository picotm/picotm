dnl This Source Code Form is subject to the terms of the Mozilla Public
dnl License, v. 2.0. If a copy of the MPL was not distributed with this
dnl file, You can obtain one at http://mozilla.org/MPL/2.0/.

AC_PREREQ([2.69])
AC_INIT([picotm], [0.0.1])

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
                 modules/Makefile
                 modules/libc/Makefile
                 modules/libc/include/Makefile
                 modules/libc/src/Makefile
                 modules/libm/Makefile
                 modules/libm/include/Makefile
                 modules/libm/src/Makefile
                 modules/libpthread/Makefile
                 modules/libpthread/include/Makefile
                 modules/libpthread/src/Makefile
                 modules/tm/Makefile
                 modules/tm/include/Makefile
                 modules/tm/src/Makefile
                 src/Makefile])

AC_OUTPUT
