dnl This Source Code Form is subject to the terms of the Mozilla Public
dnl License, v. 2.0. If a copy of the MPL was not distributed with this
dnl file, You can obtain one at http://mozilla.org/MPL/2.0/.

AC_PREREQ([2.69])
AC_INIT([picotm-tests], [0.1.0], [bugs@picotm.org], [], [http://picotm.org])

AC_CONFIG_SRCDIR([src/Makefile.am])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])


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


dnl
dnl System headers and libraries
dnl

AC_CHECK_HEADERS([sys/cdefs.h])


dnl
dnl Picotm
dnl

CHECK_PICOTM
CHECK_PICOTM_TM
CHECK_PICOTM_C


dnl
dnl Output
dnl

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT

AC_MSG_RESULT([])
AC_MSG_RESULT([  Configuration finished.])
AC_MSG_RESULT([])
AC_MSG_RESULT([  Continue installation with:])
AC_MSG_RESULT([    make])
AC_MSG_RESULT([    make install])
AC_MSG_RESULT([])
