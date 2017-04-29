dnl This Source Code Form is subject to the terms of the Mozilla Public
dnl License, v. 2.0. If a copy of the MPL was not distributed with this
dnl file, You can obtain one at http://mozilla.org/MPL/2.0/.

AC_PREREQ([2.69])
AC_INIT([picotm], [0.1.0])

AC_CONFIG_SRCDIR([src/Makefile.am])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])


dnl
dnl Compiler support
dnl

AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC_C99

AX_PTHREAD
CC="$PTHREAD_CC"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
LDFLAGS="$LDFLAGS $PTHREAD_LDFLAGS"
LIBS="$PTHREAD_LIBS $LIBS"

AX_CHECK_COMPILE_FLAG([-fvisibility=hidden],
                      [VISIBILITY_CFLAGS="-fvisibility=hidden"],
                      [VISIBILITY_CFLAGS=""])
AC_SUBST(VISIBILITY_CFLAGS, [$VISIBILITY_CFLAGS])


dnl
dnl Build toolchain
dnl

AM_PROG_AR
AC_PROG_LIBTOOL


dnl
dnl System headers and libraries
dnl

AC_CHECK_HEADERS([sys/cdefs.h])


dnl
dnl Picotm compile-time variables
dnl

AC_DEFINE([MAXNUMFD], [1024])
AC_DEFINE([RECSIZE],  [32])


dnl
dnl Documentation
dnl

AC_PATH_PROG([DOXYGEN], [doxygen])
AC_PATH_PROG([DOT], [dot])

if test "x$ac_cv_path_DOT" = "x"; then
    doxygen_have_dot="NO"
else
    doxygen_have_dot="YES"
fi
AC_SUBST([DOXYGEN_HAVE_DOT], [$doxygen_have_dot])

doxygen_input="`realpath $srcdir/include`  \
               `realpath $srcdir/modules`  \
               `realpath $srcdir/src`"
AC_SUBST([DOXYGEN_INPUT], [$doxygen_input])

doxygen_strip_from_path="`realpath $srcdir`"
AC_SUBST([DOXYGEN_STRIP_FROM_PATH], [$doxygen_strip_from_path])


dnl
dnl Output
dnl

AC_CONFIG_FILES([Makefile
                 doc/Doxyfile
                 doc/Makefile
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

AC_MSG_RESULT([])
AC_MSG_RESULT([  Configuration finished.])
AC_MSG_RESULT([])
AC_MSG_RESULT([  Continue installation with:])
AC_MSG_RESULT([    make])
AC_MSG_RESULT([    make install])
AC_MSG_RESULT([])
