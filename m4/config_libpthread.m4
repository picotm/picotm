#
# SYNOPSIS
#
#   CONFIG_LIBPTHREAD
#
# LICENSE
#
#   Copyright (c) 2017-2018 Thomas Zimmermann <contact@tzimmermann.org>
#
#   Copying and distribution of this file, with or without modification,
#   are permitted in any medium without royalty provided the copyright
#   notice and this notice are preserved.  This file is offered as-is,
#   without any warranty.

AC_DEFUN([_CHECK_LIBPTHREAD_PTHREAD_H], [
    AC_CHECK_HEADERS([pthread.h])
    AS_VAR_IF([ac_cv_header_pthread_h], [yes], [

        #
        # Types
        #

        _CHECK_MODULE_TYPE([libpthread], [pthread_t], [[@%:@include <pthread.h>]])

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libpthread], [pthread_equal], [[@%:@include <pthread.h>]])
        _CHECK_MODULE_INTF([libpthread], [pthread_self],  [[@%:@include <pthread.h>]])
    ])
])

AC_DEFUN([_CONFIG_LIBPTHREAD], [
    AC_REQUIRE([AX_PTHREAD])
    AS_VAR_IF([ax_pthread_ok], [yes], [
        _CHECK_LIBPTHREAD_PTHREAD_H
    ])
])

AC_DEFUN([CONFIG_LIBPTHREAD], [
    AC_ARG_ENABLE([module-libpthread],
                  [AS_HELP_STRING([--enable-module-libpthread],
                                  [enable POSIX Threads module @<:@default=yes@:>@])],
                  [enable_module_libpthread=$enableval],
                  [enable_module_libpthread=yes])
    AM_CONDITIONAL([ENABLE_MODULE_LIBPTHREAD],
                   [test "x$enable_module_libpthread" = "xyes"])
    AS_VAR_IF([enable_module_libpthread], [yes], [
        _CONFIG_LIBPTHREAD
    ])
])
