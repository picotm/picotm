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

AC_DEFUN([_CHECK_LIBC_MALLOC_H], [

    AC_CHECK_HEADERS([malloc.h])

    if test "x$ac_cv_header_malloc_h" != "xno"; then

        #
        # System interfaces
        #

        AC_CHECK_DECLS([malloc_usable_size],,,[[#include <malloc.h>]])
    fi
])

AC_DEFUN([_CHECK_LIBC_MALLOC_NP_H], [

    AC_CHECK_HEADERS([malloc_np.h])

    if test "x$ac_cv_header_malloc_np_h" != "xno"; then

        #
        # System interfaces
        #

        AC_CHECK_DECLS([malloc_usable_size],,,[[#include <malloc_np.h>]])
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

AC_DEFUN([_CHECK_LIBC_STRING_H], [

    AC_CHECK_HEADERS([string.h])

    if test "x$ac_cv_header_string_h" != "xno"; then

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [memccpy],    [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [memchr],     [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [memcmp],     [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [memcpy],     [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [memmove],    [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [memset],     [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [memrchr],    [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [rawmemchr],  [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [stpcpy],     [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [stpncpy],    [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strcat],     [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strchr],     [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strcmp],     [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strcoll_l],  [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strcpy],     [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strcspn],    [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strdup],     [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strerror_r], [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strlen],     [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strncat],    [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strncmp],    [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strncpy],    [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strndup],    [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strnlen],    [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strpbrk],    [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strrchr],    [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strspn],     [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strstr],     [[#include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strtok_r],   [[#include <string.h>]])
    fi
])

AC_DEFUN([_CHECK_LIBC_SYS_SOCKET_H], [

    AC_CHECK_HEADERS([sys/socket.h])

    if test "x$ac_cv_header_sys_socket_h" != "xno"; then

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [accept],   [[#include <sys/socket.h>]])
        _CHECK_MODULE_INTF([libc], [bind],     [[#include <sys/socket.h>]])
        _CHECK_MODULE_INTF([libc], [connect],  [[#include <sys/socket.h>]])
        _CHECK_MODULE_INTF([libc], [listen],   [[#include <sys/socket.h>]])
        _CHECK_MODULE_INTF([libc], [recv],     [[#include <sys/socket.h>]])
        _CHECK_MODULE_INTF([libc], [send],     [[#include <sys/socket.h>]])
        _CHECK_MODULE_INTF([libc], [shutdown], [[#include <sys/socket.h>]])
        _CHECK_MODULE_INTF([libc], [socket],   [[#include <sys/socket.h>]])
    fi
])

AC_DEFUN([_CHECK_LIBC_SYS_STAT_H], [

    AC_CHECK_HEADERS([sys/stat.h])

    if test "x$ac_cv_header_sys_stat_h" != "xno"; then

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [chmod],  [[#include <sys/stat.h>]])
        _CHECK_MODULE_INTF([libc], [fchmod], [[#include <sys/stat.h>]])
        _CHECK_MODULE_INTF([libc], [fstat],  [[#include <sys/stat.h>]])
        _CHECK_MODULE_INTF([libc], [lstat],  [[#include <sys/stat.h>]])
        _CHECK_MODULE_INTF([libc], [mkdir],  [[#include <sys/stat.h>]])
        _CHECK_MODULE_INTF([libc], [mkfifo], [[#include <sys/stat.h>]])
        _CHECK_MODULE_INTF([libc], [mknod],  [[#include <sys/stat.h>]])
        _CHECK_MODULE_INTF([libc], [stat],   [[#include <sys/stat.h>]])
    fi
])

AC_DEFUN([_CHECK_LIBC_UNISTD_H], [

    AC_CHECK_HEADERS([unistd.h])

    if test "x$ac_cv_header_unistd_h" != "xno"; then

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [_exit],  [[#include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [chdir],  [[#include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [close],  [[#include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [dup],    [[#include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [dup2],   [[#include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [fchdir], [[#include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [fsync],  [[#include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [getcwd], [[#include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [link],   [[#include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [lseek],  [[#include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [pipe],   [[#include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [pread],  [[#include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [pwrite], [[#include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [read],   [[#include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [sleep],  [[#include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [sync],   [[#include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [unlink], [[#include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [write],  [[#include <unistd.h>]])

        #
        # System interfaces
        #

        AC_CHECK_DECLS([get_current_dir_name],,,[[#include <unistd.h>]])
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
        _CHECK_LIBC_MALLOC_H
        _CHECK_LIBC_MALLOC_NP_H
        _CHECK_LIBC_SCHED_H
        _CHECK_LIBC_STDIO_H
        _CHECK_LIBC_STDLIB_H
        _CHECK_LIBC_STRING_H
        _CHECK_LIBC_SYS_SOCKET_H
        _CHECK_LIBC_SYS_STAT_H
        _CHECK_LIBC_UNISTD_H
    fi
])
