dnl
dnl picotm - A system-level transaction manager
dnl Copyright (c) 2017-2018 Thomas Zimmermann <contact@tzimmermann.org>
dnl
dnl This program is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU Lesser General Public License as published by
dnl the Free Software Foundation, either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU Lesser General Public License for more details.
dnl
dnl You should have received a copy of the GNU Lesser General Public License
dnl along with this program.  If not, see <https://www.gnu.org/licenses/>.
dnl
dnl SPDX-License-Identifier: LGPL-3.0-or-later
dnl

AC_PREREQ([2.69])
AC_INIT([picotm], [0.12.0], [bugs@picotm.org], [], [http://picotm.org])

dnl Internal configuration header
AH_TOP([/* SPDX-License-Identifier: LGPL-3.0-or-later */])
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

AC_CONFIG_FILES([tests/env/tests-env.sh])


dnl
dnl Core transaction manager
dnl

AC_CONFIG_HEADERS(
    [include/picotm/config/picotm-config.h])

AC_CHECK_HEADERS([signal.h],
                 [AC_DEFINE([PICOTM_HAVE_SIGNAL_H],
                            [1],
                            [Define to 1 if you have the <signal.h> header file.])]
                 [AC_CHECK_TYPES([siginfo_t],
                                 [AC_DEFINE([PICOTM_HAVE_TYPE_SIGINFO_T],
                                 [1],
                                 [Define to 1 if the system has the type 'siginfo_t'.])],,
                                 [[@%:@include <signal.h>]])])

AC_CHECK_HEADERS([setjmp.h],
                 [AC_DEFINE([PICOTM_HAVE_SETJMP_H],
                            [1],
                            [Define to 1 if you have the <setjmp.h> header file.])]
                 [AC_CHECK_TYPES([sigjmp_buf],
                                 [AC_DEFINE([PICOTM_HAVE_TYPE_SIGJMP_BUF],
                                 [1],
                                 [Define to 1 if the system has the type 'sigjmp_buf'.])],,
                                 [[@%:@include <setjmp.h>]])])

AC_CHECK_TYPES([kern_return_t],
               [AC_DEFINE([PICOTM_HAVE_TYPE_KERN_RETURN_T],
                          [1],
                          [Define to 1 if the system has the type 'kern_return_t'.])],,
               [[@%:@include <mach/mach.h>]])


dnl
dnl Modules
dnl

AC_CONFIG_HEADERS(
    [modules/ptrdata/include/picotm/config/picotm-ptrdata-config.h])
CONFIG_PTRDATA

AC_CONFIG_HEADERS([modules/cast/include/picotm/config/picotm-cast-config.h])
CONFIG_CAST

AC_CONFIG_HEADERS(
    [modules/arithmetic/include/picotm/config/picotm-arithmetic-config.h])
CONFIG_ARITHMETIC

AC_CONFIG_HEADERS([modules/tm/include/picotm/config/picotm-tm-config.h])
CONFIG_TM

AC_CONFIG_HEADERS([modules/libc/include/picotm/config/picotm-libc-config.h])
CONFIG_LIBC

AC_CONFIG_HEADERS(
    [modules/libpthread/include/picotm/config/picotm-libpthread-config.h])
CONFIG_LIBPTHREAD

AC_CONFIG_HEADERS([modules/libm/include/picotm/config/picotm-libm-config.h])
CONFIG_LIBM

AC_CONFIG_HEADERS([modules/txlib/include/picotm/config/picotm-txlib-config.h])
CONFIG_TXLIB


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
dnl Error checking
dnl
dnl Some autoconf macros don't work well with -Wall and especially
dnl -Werror. We set both options at the end of the configure script
dnl to avoid problems during the earlier tests.
dnl
dnl Exceptions from error checking should be listed here and must
dnl be documented. If you only need an exception for a very specfic
dnl code location, consider using a GCC pragma instead.
dnl

AX_CHECK_COMPILE_FLAG([-Wall],
                      [AS_VAR_APPEND([CFLAGS], [" -Wall"])])
AX_CHECK_COMPILE_FLAG([-Werror],
                      [AS_VAR_APPEND([CFLAGS], [" -Werror"])])


dnl
dnl Output
dnl

AC_CONFIG_FILES([Makefile
                 doc/Doxyfile
                 doc/Makefile
                 extras/Makefile
                 extras/aclocal/Makefile
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
                 modules/ptrdata/Makefile
                 modules/ptrdata/include/Makefile
                 modules/ptrdata/src/Makefile
                 modules/ptrdata/tests/Makefile
                 modules/ptrdata/tests/modapi/Makefile
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
                 tests/env/Makefile
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
