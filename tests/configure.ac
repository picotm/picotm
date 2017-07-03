dnl Permission is hereby granted, free of charge, to any person obtaining a
dnl copy of this software and associated documentation files (the "Software"),
dnl to deal in the Software without restriction, including without limitation
dnl the rights to use, copy, modify, merge, publish, distribute, sublicense,
dnl and/or sell copies of the Software, and to permit persons to whom the
dnl Software is furnished to do so, subject to the following conditions:
dnl
dnl The above copyright notice and this permission notice shall be included in
dnl all copies or substantial portions of the Software.
dnl
dnl THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
dnl IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
dnl FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
dnl THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
dnl LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
dnl FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
dnl DEALINGS IN THE SOFTWARE.

AC_PREREQ([2.69])
AC_INIT([picotm-tests], [0.3.0], [bugs@picotm.org], [], [http://picotm.org])

AC_CONFIG_SRCDIR([src/Makefile.am])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])


dnl
dnl Compiler support
dnl

AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC

AX_PTHREAD
CC="$PTHREAD_CC"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
LDFLAGS="$LDFLAGS $PTHREAD_LDFLAGS"
LIBS="$PTHREAD_LIBS $LIBS"

dnl Request C11
AX_CHECK_COMPILE_FLAG([-std=c11], [CFLAGS="$CFLAGS -std=c11"])


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
