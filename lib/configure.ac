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
AC_INIT([picotm], [0.4.0], [bugs@picotm.org], [], [http://picotm.org])

AC_CONFIG_SRCDIR([src/Makefile.am])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])


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

dnl By default, all symbols are hidden. Public symbols have to
dnl exported explicitly.
AX_CHECK_COMPILE_FLAG([-fvisibility=hidden],
                      [CFLAGS="$CFLAGS -fvisibility=hidden"])


dnl
dnl Build toolchain
dnl

AM_PROG_AR
AC_PROG_LIBTOOL


dnl
dnl System headers and libraries
dnl

AC_HEADER_ASSERT

AC_CHECK_HEADERS([sys/cdefs.h])

AC_FUNC_ALLOCA


dnl
dnl Picotm compile-time variables
dnl

AC_DEFINE([MAXNUMFD], [1024])
AC_DEFINE([RECBITS],  [5])


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
