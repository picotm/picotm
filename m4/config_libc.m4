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
    fi
])
