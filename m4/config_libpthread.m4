#
# SYNOPSIS
#
#   CONFIG_LIBPTHREAD
#
# LICENSE
#
#   Copyright (c) 2017 Thomas Zimmermann <tdz@users.sourceforge.net>
#
#   Copying and distribution of this file, with or without modification,
#   are permitted in any medium without royalty provided the copyright
#   notice and this notice are preserved.  This file is offered as-is,
#   without any warranty.

AC_DEFUN([_CHECK_LIBPTHREAD_PTHREAD_H], [

    AC_CHECK_HEADERS([pthread.h])

    if test "x$ac_cv_header_pthread_h" != "xno"; then

        #
        # Types
        #

        _CHECK_MODULE_TYPE([libpthread], [pthread_t], [[@%:@include <pthread.h>]])

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libpthread], [pthread_equal], [[#include <pthread.h>]])
        _CHECK_MODULE_INTF([libpthread], [pthread_self],  [[#include <pthread.h>]])
    fi
])

AC_DEFUN([CONFIG_LIBPTHREAD], [
    AC_REQUIRE([AX_PTHREAD])
    if test "x$ax_pthread_ok" != "xno"; then
        _CHECK_LIBPTHREAD_PTHREAD_H
    fi
])
