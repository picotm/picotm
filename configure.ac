dnl
dnl MIT License
dnl Copyright (c) 2017-2018 Thomas Zimmermann <tdz@users.sourceforge.net>
dnl
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
dnl
dnl SPDX-License-Identifier: MIT

AC_PREREQ([2.69])
AC_INIT([picotm], [0.11.0], [bugs@picotm.org], [], [http://picotm.org])

dnl Internal configuration header
AH_TOP([/* SPDX-License-Identifier: MIT */])
AC_CONFIG_HEADERS([config/config.h])

AC_CONFIG_SRCDIR([src/Makefile.am])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AC_REQUIRE_AUX_FILE([tap-driver.sh])


dnl
dnl Compiler support
dnl

AC_USE_SYSTEM_EXTENSIONS
AC_CANONICAL_HOST
AS_CASE([$host_os],
        [*darwin*], [AC_DEFINE([_DARWIN_C_SOURCE],
                               [1],
                               [Enable extensions on Darwin.])])
AC_PROG_CC

AX_PTHREAD
AS_VAR_SET([CC], ["$PTHREAD_CC"])
AS_VAR_APPEND([CFLAGS], [" $PTHREAD_CFLAGS"])
AS_VAR_APPEND([LDFLAGS], [" $PTHREAD_LDFLAGS"])
AS_VAR_SET([LIBS], ["$PTHREAD_LIBS $LIBS"])

dnl Request C11
AX_CHECK_COMPILE_FLAG([-std=c11],
                      [AS_VAR_APPEND([CFLAGS], [" -std=c11"])])

dnl By default, all symbols are hidden. Public symbols have to
dnl exported explicitly.
AX_CHECK_COMPILE_FLAG([-fvisibility=hidden],
                      [AS_VAR_APPEND([CFLAGS], [" -fvisibility=hidden"])])

dnl Garbage-collect unused functions and data. This is mostly
dnl useful for reducing the size of statically linked binaries.
AX_CHECK_LINK_FLAG([-Wl,--gc-sections],
                   [AS_VAR_APPEND([LDFLAGS], [" -Wl,--gc-sections"])]
                   [AX_CHECK_COMPILE_FLAG(
                        [-ffunction-sections],
                        [AS_VAR_APPEND([CFLAGS], [" -ffunction-sections"])])]
                   [AX_CHECK_COMPILE_FLAG(
                        [-fdata-sections],
                        [AS_VAR_APPEND([CFLAGS], [" -fdata-sections"])])])


dnl
dnl Build toolchain
dnl

AM_PROG_AR

dnl 1) All options to LT_INIT are evaluated statically by autoconf
dnl    when it generates the configure script. So none can depend on
dnl    run-time parameters. Same is true for libtool's AC_ macros.
dnl
dnl 2) Building shared DLLs on Windows is currently not supported. A
dnl    shared DLL has to resolve all its dependencies at link time, but
dnl    the internal dependencies cannot be resolved as DLLs of picotm
dnl    do not exist until installation. Libtool will warn during the
dnl    link process and disabled shared DLLs automatically.
LT_INIT([win32-dll])


dnl
dnl System headers and libraries
dnl

AC_HEADER_ASSERT

AC_CHECK_HEADERS([sys/cdefs.h])

AC_FUNC_ALLOCA


dnl
dnl Test infrastructure
dnl

AC_CHECK_TYPES([pthread_barrierattr_t],,, [[@%:@include <pthread.h>]])
AC_CHECK_TYPES([pthread_barrier_t],,,     [[@%:@include <pthread.h>]])

AC_ARG_VAR([VALGRIND], [Path to Valgrind])
AS_VAR_SET_IF([VALGRIND],,[AS_VAR_SET([VALGRIND], [valgrind])])
AC_PATH_PROG([VALGRIND], [$VALGRIND])
AM_CONDITIONAL([HAVE_VALGRIND], [test "x$VALGRIND" != "x"])


dnl
dnl Core transaction manager
dnl

AC_CONFIG_HEADERS(
    [include/picotm/config/picotm-config.h])

AC_CHECK_TYPES([kern_return_t],
               [AC_DEFINE([PICOTM_HAVE_TYPE_KERN_RETURN_T],
                          [1],
                          [Define to 1 if the system has the type 'kern_return_t'.])],,
               [[@%:@include <mach/mach.h>]])

CONFIG_TEST([tests/pubapi/core-pubapi-valgrind-t1.test])


dnl
dnl Modules
dnl

AC_CONFIG_HEADERS([modules/cast/include/picotm/config/picotm-cast-config.h])
CONFIG_CAST
CONFIG_TEST([modules/cast/tests/pubapi/cast-pubapi-valgrind-t1.test])

AC_CONFIG_HEADERS(
    [modules/arithmetic/include/picotm/config/picotm-arithmetic-config.h])
CONFIG_ARITHMETIC
CONFIG_TEST([modules/arithmetic/tests/pubapi/arithmetic-pubapi-valgrind-t1.test])

AC_CONFIG_HEADERS([modules/tm/include/picotm/config/picotm-tm-config.h])
CONFIG_TM
CONFIG_TEST([modules/tm/tests/pubapi/tm-pubapi-valgrind-t1.test])

AC_CONFIG_HEADERS([modules/libc/include/picotm/config/picotm-libc-config.h])
CONFIG_LIBC
CONFIG_TEST([modules/libc/tests/pubapi/allocator-pubapi-valgrind-t1.test])
CONFIG_TEST([modules/libc/tests/pubapi/cwd-pubapi-valgrind-t1.test])
CONFIG_TEST([modules/libc/tests/pubapi/fildes-pubapi-valgrind-t1.test])

AC_CONFIG_HEADERS(
    [modules/libpthread/include/picotm/config/picotm-libpthread-config.h])
CONFIG_LIBPTHREAD

AC_CONFIG_HEADERS([modules/libm/include/picotm/config/picotm-libm-config.h])
CONFIG_LIBM
CONFIG_TEST([modules/libm/tests/pubapi/complex-pubapi-valgrind-t1.test])
CONFIG_TEST([modules/libm/tests/pubapi/math-pubapi-valgrind-t1.test])

AC_CONFIG_HEADERS([modules/txlib/include/picotm/config/picotm-txlib-config.h])
CONFIG_TXLIB
CONFIG_TEST([modules/txlib/tests/pubapi/txlist-pubapi-valgrind-t1.test])
CONFIG_TEST([modules/txlib/tests/pubapi/txmultiset-pubapi-valgrind-t1.test])
CONFIG_TEST([modules/txlib/tests/pubapi/txqueue-pubapi-valgrind-t1.test])
CONFIG_TEST([modules/txlib/tests/pubapi/txstack-pubapi-valgrind-t1.test])


dnl
dnl Documentation
dnl

AC_ARG_VAR([DOXYGEN], [Path to Doxygen])
AS_VAR_SET_IF([DOXYGEN],,[AS_VAR_SET([DOXYGEN], [doxygen])])
AC_PATH_PROG([DOXYGEN], [$DOXYGEN])
AM_CONDITIONAL([HAVE_DOXYGEN], [test "x$DOXYGEN" != "x"])

AC_ARG_VAR([DOT], [Path to dot])
AS_VAR_SET_IF([DOT],,[AS_VAR_SET([DOT], [dot])])
AC_PATH_PROG([DOT], [$DOT])
AS_VAR_IF([ac_cv_path_DOT], [],
          [AS_VAR_SET([doxygen_have_dot], ["NO"])],
          [AS_VAR_SET([doxygen_have_dot], ["YES"])])
AC_SUBST([DOXYGEN_HAVE_DOT], [$doxygen_have_dot])

AC_ARG_VAR([FIND], [Path to find])
AS_VAR_SET_IF([FIND],,[AS_VAR_SET([FIND], [find])])
AC_PATH_PROG([FIND], [$FIND])
AM_CONDITIONAL([HAVE_FIND], [test "x$FIND" != "x"])

AC_ARG_VAR([OPTIPNG], [Path to OptiPNG])
AS_VAR_SET_IF([OPTIPNG],,[AS_VAR_SET([OPTIPNG], [optipng])])
AC_PATH_PROG([OPTIPNG], [$OPTIPNG])
AM_CONDITIONAL([HAVE_OPTIPNG], [test "x$OPTIPNG" != "x"])


dnl
dnl Output
dnl

AC_CONFIG_FILES([Makefile
                 doc/Doxyfile
                 doc/Makefile
                 extras/Makefile
                 extras/autoconf/Makefile
                 include/Makefile
                 LICENSES/Makefile
                 modules/Makefile
                 modules/arithmetic/Makefile
                 modules/arithmetic/include/Makefile
                 modules/arithmetic/src/Makefile
                 modules/arithmetic/tests/Makefile
                 modules/arithmetic/tests/pubapi/Makefile
                 modules/cast/Makefile
                 modules/cast/include/Makefile
                 modules/cast/src/Makefile
                 modules/cast/tests/Makefile
                 modules/cast/tests/pubapi/Makefile
                 modules/libc/Makefile
                 modules/libc/include/Makefile
                 modules/libc/src/Makefile
                 modules/libc/tests/Makefile
                 modules/libc/tests/pubapi/Makefile
                 modules/libm/Makefile
                 modules/libm/include/Makefile
                 modules/libm/src/Makefile
                 modules/libm/tests/Makefile
                 modules/libm/tests/pubapi/Makefile
                 modules/libpthread/Makefile
                 modules/libpthread/include/Makefile
                 modules/libpthread/src/Makefile
                 modules/tm/Makefile
                 modules/tm/include/Makefile
                 modules/tm/src/Makefile
                 modules/tm/tests/Makefile
                 modules/tm/tests/pubapi/Makefile
                 modules/txlib/Makefile
                 modules/txlib/include/Makefile
                 modules/txlib/src/Makefile
                 modules/txlib/tests/Makefile
                 modules/txlib/tests/pubapi/Makefile
                 src/Makefile
                 tests/Makefile
                 tests/libsafeblk/Makefile
                 tests/libtap/Makefile
                 tests/libtests/Makefile
                 tests/pubapi/Makefile
                 tools/Makefile])
AC_OUTPUT

AC_MSG_RESULT([])
AC_MSG_RESULT([  Configuration finished.])
AC_MSG_RESULT([])
AC_MSG_RESULT([  Continue installation with:])
AC_MSG_RESULT([    make])
AC_MSG_RESULT([    make install])
AC_MSG_RESULT([])
AC_MSG_RESULT([  Further steps:])
AC_MSG_RESULT([])
AC_MSG_RESULT([      Generate documentation (requires Doxygen) with:])
AC_MSG_RESULT([        make html])
AC_MSG_RESULT([        make install-html])
AC_MSG_RESULT([])
AC_MSG_RESULT([      Usage examples are available in the documentation, or the test cases under:])
AC_MSG_RESULT([        modules/<module name>/tests/pubapi/])
AC_MSG_RESULT([])
AC_MSG_RESULT([      Run test cases with:])
AC_MSG_RESULT([        make -j1 check])
AC_MSG_RESULT([])
AC_MSG_RESULT([      For questions, suggestions or general discussion, please reach out via])
AC_MSG_RESULT([        Mailing list:    picotm-devel@picotm.org])
AC_MSG_RESULT([        IRC:             @%:@picotm on freenode.net])
AC_MSG_RESULT([        Web:             picotm.org])
AC_MSG_RESULT([])
