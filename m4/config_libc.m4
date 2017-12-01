#
# SYNOPSIS
#
#   CONFIG_LIBC
#
# LICENSE
#
#   Copyright (c) 2017 Thomas Zimmermann <tdz@users.sourceforge.net>
#
#   Copying and distribution of this file, with or without modification,
#   are permitted in any medium without royalty provided the copyright
#   notice and this notice are preserved.  This file is offered as-is,
#   without any warranty.

AC_DEFUN([_CHECK_LIBC_ERRNO_H], [

    AC_CHECK_HEADERS([errno.h])

    if test "x$ac_cv_header_errno_h" != "xno"; then

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [errno], [[#include <errno.h>]])
    fi
])

AC_DEFUN([_CHECK_LIBC_FCNTL_H], [

    AC_CHECK_HEADERS([fcntl.h])

    if test "x$ac_cv_header_fcntl_h" != "xno"; then

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [creat], [[#include <fcntl.h>]])
        _CHECK_MODULE_INTF([libc], [fcntl], [[#include <fcntl.h>]])
        _CHECK_MODULE_INTF([libc], [open],  [[#include <fcntl.h>]])
    fi
])

AC_DEFUN([_CHECK_LIBC_SCHED_H], [

    AC_CHECK_HEADERS([sched.h])

    if test "x$ac_cv_header_sched_h" != "xno"; then

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [sched_yield], [[#include <sched.h>]])
    fi
])

AC_DEFUN([_CHECK_LIBC_STDIO_H], [

    AC_CHECK_HEADERS([stdio.h])

    if test "x$ac_cv_header_stdio_h" != "xno"; then

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [snprintf],  [[#include <stdio.h>]])
        _CHECK_MODULE_INTF([libc], [sscanf],    [[#include <stdio.h>]])
        _CHECK_MODULE_INTF([libc], [vsnprintf], [[#include <stdio.h>]])
        _CHECK_MODULE_INTF([libc], [vsscanf],   [[#include <stdio.h>]])
    fi
])

AC_DEFUN([_CHECK_LIBC_STDLIB_H], [

    AC_CHECK_HEADERS([stdlib.h])

    if test "x$ac_cv_header_stdlib_h" != "xno"; then

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [_Exit],          [[#include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [abort],          [[#include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [calloc],         [[#include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [exit],           [[#include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [free],           [[#include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [malloc],         [[#include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [mkdtemp],        [[#include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [mkstemp],        [[#include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [posix_memalign], [[#include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [qsort],          [[#include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [realloc],        [[#include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [rand_r],         [[#include <stdlib.h>]])
    fi
])

AC_DEFUN([CONFIG_LIBC], [
    AC_CHECK_LIB([c], [longjmp])
    if test "x$ac_cv_lib_c_memcpy" != "xno"; then

        #
        # libc compile-time constants
        #

        AC_DEFINE([MAXNUMFD], [1024], [Maximum number of file descriptors])
        AC_DEFINE([RECBITS], [5], [Bits per file record])

        #
        # System interfaces and functionality
        #

        _CHECK_LIBC_ERRNO_H
        _CHECK_LIBC_FCNTL_H
        _CHECK_LIBC_SCHED_H
        _CHECK_LIBC_STDIO_H
        _CHECK_LIBC_STDLIB_H
    fi
])
