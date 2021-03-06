#
# SYNOPSIS
#
#   CONFIG_LIBC
#
# LICENSE
#
#   picotm - A system-level transaction manager
#   Copyright (c) 2017-2018 Thomas Zimmermann <contact@tzimmermann.org>
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Lesser General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
#   SPDX-License-Identifier: LGPL-3.0-or-later
#

AC_DEFUN([_CHECK_LIBC_ERRNO_H], [
    AC_CHECK_HEADERS([errno.h])
    AS_VAR_IF([ac_cv_header_errno_h], [yes], [

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [errno], [[@%:@include <errno.h>]])
    ])
])

AC_DEFUN([_CHECK_LIBC_FCNTL_H], [
    AC_CHECK_HEADERS([fcntl.h])
    AS_VAR_IF([ac_cv_header_fcntl_h], [yes], [

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [creat], [[@%:@include <fcntl.h>]])
        _CHECK_MODULE_INTF([libc], [fcntl], [[@%:@include <fcntl.h>]])
        _CHECK_MODULE_INTF([libc], [open],  [[@%:@include <fcntl.h>]])
    ])
])

AC_DEFUN([_CHECK_LIBC_LOCALE_H], [
    AC_CHECK_HEADERS([locale.h])
    AS_VAR_IF([ac_cv_header_locale_h], [yes], [

        #
        # Types
        #

        _CHECK_MODULE_TYPE([libc], [locale_t],     [[@%:@include <locale.h>]])
        _CHECK_MODULE_TYPE([libc], [struct lconv], [[@%:@include <locale.h>]])

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [duplocale],  [[@%:@include <locale.h>]])
        _CHECK_MODULE_INTF([libc], [freelocale], [[@%:@include <locale.h>]])
        _CHECK_MODULE_INTF([libc], [localeconv], [[@%:@include <locale.h>]])
        _CHECK_MODULE_INTF([libc], [newlocale],  [[@%:@include <locale.h>]])
        _CHECK_MODULE_INTF([libc], [setlocale],  [[@%:@include <locale.h>]])
        _CHECK_MODULE_INTF([libc], [uselocale],  [[@%:@include <locale.h>]])
    ])
])

AC_DEFUN([_CHECK_LIBC_MALLOC_H], [
    AC_CHECK_HEADERS([malloc.h])
    AS_VAR_IF([ac_cv_header_malloc_h], [yes], [

        #
        # System interfaces
        #

        AC_CHECK_DECLS([malloc_usable_size],,, [[@%:@include <malloc.h>]])
    ])
])

AC_DEFUN([_CHECK_LIBC_MALLOC_MALLOC_H], [
    AC_CHECK_HEADERS([malloc/malloc.h])
    AS_VAR_IF([ac_cv_header_malloc_malloc_h], [yes], [

        #
        # System interfaces
        #

        AC_CHECK_DECLS([malloc_size],,, [[@%:@include <malloc/malloc.h>]])
    ])
])

AC_DEFUN([_CHECK_LIBC_MALLOC_NP_H], [
    AC_CHECK_HEADERS([malloc_np.h])
    AS_VAR_IF([ac_cv_header_malloc_np_h], [yes], [

        #
        # System interfaces
        #

        AC_CHECK_DECLS([malloc_usable_size],,, [[@%:@include <malloc_np.h>]])
    ])
])

AC_DEFUN([_CHECK_LIBC_SCHED_H], [
    AC_CHECK_HEADERS([sched.h])
    AS_VAR_IF([ac_cv_header_sched_h], [yes], [

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [sched_yield], [[@%:@include <sched.h>]])
    ])
])

AC_DEFUN([_CHECK_LIBC_STDBOOL_H], [
    AC_CHECK_HEADERS([stdbool.h])
    AS_VAR_IF([ac_cv_header_stdbool_h], [yes], [

        #
        # Types
        #

        _CHECK_MODULE_TYPE([libc], [bool], [[@%:@include <stdbool.h>]])
    ])
])

AC_DEFUN([_CHECK_LIBC_STDDEF_H], [
    AC_CHECK_HEADERS([stddef.h])
    AS_VAR_IF([ac_cv_header_stddef_h], [yes], [

        #
        # Types
        #

        _CHECK_MODULE_TYPE([libc], [ptrdiff_t], [[@%:@include <stddef.h>]])
        _CHECK_MODULE_TYPE([libc], [size_t],    [[@%:@include <stddef.h>]])
        _CHECK_MODULE_TYPE([libc], [wchar_t],   [[@%:@include <stddef.h>]])
    ])
])

AC_DEFUN([_CHECK_LIBC_STDINT_H], [
    AC_CHECK_HEADERS([stdint.h])
    AS_VAR_IF([ac_cv_header_stdint_h], [yes], [

        #
        # Types
        #

        _CHECK_MODULE_TYPE([libc], [int_fast8_t],    [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [int_fast16_t],   [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [int_fast32_t],   [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [int_fast64_t],   [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [int_least8_t],   [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [int_least16_t],  [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [int_least32_t],  [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [int_least64_t],  [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [int8_t],         [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [int16_t],        [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [int32_t],        [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [int64_t],        [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [intmax_t],       [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [intptr_t],       [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [uint_fast8_t],   [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [uint_fast16_t],  [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [uint_fast32_t],  [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [uint_fast64_t],  [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [uint_least8_t],  [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [uint_least16_t], [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [uint_least32_t], [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [uint_least64_t], [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [uint8_t],        [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [uint16_t],       [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [uint32_t],       [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [uint64_t],       [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [uintmax_t],      [[@%:@include <stdint.h>]])
        _CHECK_MODULE_TYPE([libc], [uintptr_t],      [[@%:@include <stdint.h>]])
    ])
])

AC_DEFUN([_CHECK_LIBC_STDIO_H], [
    AC_CHECK_HEADERS([stdio.h])
    AS_VAR_IF([ac_cv_header_stdio_h], [yes], [

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [snprintf],  [[@%:@include <stdio.h>]])
        _CHECK_MODULE_INTF([libc], [sscanf],    [[@%:@include <stdio.h>]])
        _CHECK_MODULE_INTF([libc], [vsnprintf], [[@%:@include <stdio.h>]])
        _CHECK_MODULE_INTF([libc], [vsscanf],   [[@%:@include <stdio.h>]])
    ])
])

AC_DEFUN([_CHECK_LIBC_STDLIB_H], [
    AC_CHECK_HEADERS([stdlib.h])
    AS_VAR_IF([ac_cv_header_stdlib_h], [yes], [

        #
        # Types
        #

        _CHECK_MODULE_TYPE([libc], [div_t],   [[@%:@include <stdlib.h>]])
        _CHECK_MODULE_TYPE([libc], [ldiv_t],  [[@%:@include <stdlib.h>]])
        _CHECK_MODULE_TYPE([libc], [lldiv_t], [[@%:@include <stdlib.h>]])
        _CHECK_MODULE_TYPE([libc], [size_t],  [[@%:@include <stdlib.h>]])
        _CHECK_MODULE_TYPE([libc], [wchar_t], [[@%:@include <stdlib.h>]])

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [_Exit],  [[@%:@include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [abort],  [[@%:@include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [calloc], [[@%:@include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [exit],   [[@%:@include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [free],   [[@%:@include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [malloc], [[@%:@include <stdlib.h>]])
        # POSIX requires mkdtemp() to be declared in <stdlib.h>. Darwin
        # systems declare it in <unistd.h> instead.
        AS_CASE([$host_os],
                [*darwin*], [],
                [_CHECK_MODULE_INTF([libc],
                                    [mkdtemp],
                                    [[@%:@include <stdlib.h>]])])
        _CHECK_MODULE_INTF([libc], [mkstemp],        [[@%:@include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [posix_memalign], [[@%:@include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [qsort],          [[@%:@include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [realloc],        [[@%:@include <stdlib.h>]])
        _CHECK_MODULE_INTF([libc], [rand_r],         [[@%:@include <stdlib.h>]])

        #
        # Internal interfaces
        #

        AC_CHECK_FUNCS([posix_memalign],,, [[@%:@include <stdlib.h>]])
    ])
])

AC_DEFUN([_CHECK_LIBC_STRING_H], [
    AC_CHECK_HEADERS([string.h])
    AS_VAR_IF([ac_cv_header_string_h], [yes], [

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [memccpy],    [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [memchr],     [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [memcmp],     [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [memcpy],     [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [memmove],    [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [memset],     [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [memrchr],    [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [rawmemchr],  [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [stpcpy],     [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [stpncpy],    [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strcat],     [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strchr],     [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strcmp],     [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strcoll_l],  [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strcpy],     [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strcspn],    [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strdup],     [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strerror_r], [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strlen],     [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strncat],    [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strncmp],    [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strncpy],    [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strndup],    [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strnlen],    [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strpbrk],    [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strrchr],    [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strspn],     [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strstr],     [[@%:@include <string.h>]])
        _CHECK_MODULE_INTF([libc], [strtok_r],   [[@%:@include <string.h>]])
    ])
])

AC_DEFUN([_CHECK_LIBC_SYS_SOCKET_H], [
    AC_CHECK_HEADERS([sys/socket.h])
    AS_VAR_IF([ac_cv_header_sys_socket_h], [yes], [

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [accept],   [[@%:@include <sys/socket.h>]])
        _CHECK_MODULE_INTF([libc], [bind],     [[@%:@include <sys/socket.h>]])
        _CHECK_MODULE_INTF([libc], [connect],  [[@%:@include <sys/socket.h>]])
        _CHECK_MODULE_INTF([libc], [listen],   [[@%:@include <sys/socket.h>]])
        _CHECK_MODULE_INTF([libc], [recv],     [[@%:@include <sys/socket.h>]])
        _CHECK_MODULE_INTF([libc], [send],     [[@%:@include <sys/socket.h>]])
        _CHECK_MODULE_INTF([libc], [shutdown], [[@%:@include <sys/socket.h>]])
        _CHECK_MODULE_INTF([libc], [socket],   [[@%:@include <sys/socket.h>]])
    ])
])

AC_DEFUN([_CHECK_LIBC_SYS_STAT_H], [
    AC_CHECK_HEADERS([sys/stat.h])
    AS_VAR_IF([ac_cv_header_sys_stat_h], [yes], [

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [chmod],  [[@%:@include <sys/stat.h>]])
        _CHECK_MODULE_INTF([libc], [fchmod], [[@%:@include <sys/stat.h>]])
        _CHECK_MODULE_INTF([libc], [fstat],  [[@%:@include <sys/stat.h>]])
        _CHECK_MODULE_INTF([libc], [lstat],  [[@%:@include <sys/stat.h>]])
        _CHECK_MODULE_INTF([libc], [mkdir],  [[@%:@include <sys/stat.h>]])
        _CHECK_MODULE_INTF([libc], [mkfifo], [[@%:@include <sys/stat.h>]])
        _CHECK_MODULE_INTF([libc], [mknod],  [[@%:@include <sys/stat.h>]])
        _CHECK_MODULE_INTF([libc], [stat],   [[@%:@include <sys/stat.h>]])
    ])
])

AC_DEFUN([_CHECK_LIBC_SYS_TYPES_H], [
    AC_CHECK_HEADERS([sys/types.h])
    AS_VAR_IF([ac_cv_header_sys_types_h], [yes], [

        #
        # Types
        #

        _CHECK_MODULE_TYPE([libc], [blkcnt_t],    [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [blksize_t],   [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [clock_t],     [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [clockid_t],   [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [dev_t],       [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [fsblkcnt_t],  [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [fsfilcnt_t],  [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [gid_t],       [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [id_t],        [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [ino_t],       [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [key_t],       [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [mode_t],      [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [nlink_t],     [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [off_t],       [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [pid_t],       [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [ssize_t],     [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [suseconds_t], [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [time_t],      [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [timer_t],     [[@%:@include <sys/types.h>]])
        _CHECK_MODULE_TYPE([libc], [uid_t],       [[@%:@include <sys/types.h>]])
    ])
])

AC_DEFUN([_CHECK_LIBC_TIME_H], [
    AC_CHECK_HEADERS([time.h])
    AS_VAR_IF([ac_cv_header_time_h], [yes], [

        #
        # Types
        #

        _CHECK_MODULE_TYPE([libc], [struct itimerspec], [[@%:@include <time.h>]])
        _CHECK_MODULE_TYPE([libc], [struct timespec],   [[@%:@include <time.h>]])
        _CHECK_MODULE_TYPE([libc], [struct tm],         [[@%:@include <time.h>]])

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [strftime],   [[@%:@include <time.h>]])
        _CHECK_MODULE_INTF([libc], [strftime_l], [[@%:@include <time.h>]])
        _CHECK_MODULE_INTF([libc], [strptime],   [[@%:@include <time.h>]])
    ])
])

AC_DEFUN([_CHECK_LIBC_UNISTD_H], [
    AC_CHECK_HEADERS([unistd.h])
    AS_VAR_IF([ac_cv_header_unistd_h], [yes], [

        #
        # Public interfaces
        #

        _CHECK_MODULE_INTF([libc], [_exit],  [[@%:@include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [chdir],  [[@%:@include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [close],  [[@%:@include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [dup],    [[@%:@include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [dup2],   [[@%:@include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [fchdir], [[@%:@include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [fsync],  [[@%:@include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [getcwd], [[@%:@include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [link],   [[@%:@include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [lseek],  [[@%:@include <unistd.h>]])
        # Darwin systems declare mkdtemp() in <unistd.h> instead
        # of <stdlib.h>.
        AS_CASE([$host_os],
                [*darwin*], [_CHECK_MODULE_INTF([libc],
                                                [mkdtemp],
                                                [[@%:@include <unistd.h>]])])
        _CHECK_MODULE_INTF([libc], [pipe],   [[@%:@include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [pread],  [[@%:@include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [pwrite], [[@%:@include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [read],   [[@%:@include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [sleep],  [[@%:@include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [sync],   [[@%:@include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [unlink], [[@%:@include <unistd.h>]])
        _CHECK_MODULE_INTF([libc], [write],  [[@%:@include <unistd.h>]])

        #
        # System interfaces
        #

        AC_CHECK_DECLS([get_current_dir_name],,, [[@%:@include <unistd.h>]])
    ])
])

AC_DEFUN([_CONFIG_LIBC], [
    dnl Functions from the C standard library are often provided as
    dnl built-ins by the compiler. This breaks the Autoconf test. We
    dnl disable built-ins while testing for longjmp().
    AS_VAR_COPY([saved_LDFLAGS], [LDFLAGS])
    AS_VAR_APPEND([LDFLAGS], [" -fno-builtin"])
    AC_CHECK_LIB([c], [longjmp])
    AS_VAR_COPY([LDFLAGS], [saved_LDFLAGS])

    AS_VAR_IF([ac_cv_lib_c_longjmp], [yes], [

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
        _CHECK_LIBC_LOCALE_H
        _CHECK_LIBC_MALLOC_H
        _CHECK_LIBC_MALLOC_MALLOC_H
        _CHECK_LIBC_MALLOC_NP_H
        _CHECK_LIBC_SCHED_H
        _CHECK_LIBC_STDBOOL_H
        _CHECK_LIBC_STDDEF_H
        _CHECK_LIBC_STDINT_H
        _CHECK_LIBC_STDIO_H
        _CHECK_LIBC_STDLIB_H
        _CHECK_LIBC_STRING_H
        _CHECK_LIBC_SYS_SOCKET_H
        _CHECK_LIBC_SYS_STAT_H
        _CHECK_LIBC_SYS_TYPES_H
        _CHECK_LIBC_TIME_H
        _CHECK_LIBC_UNISTD_H
    ])
])

AC_DEFUN([CONFIG_LIBC], [
    AC_ARG_ENABLE([module-libc],
                  [AS_HELP_STRING([--enable-module-libc],
                                  [enable C Standard Library module @<:@default=yes@:>@])],
                  [enable_module_libc=$enableval],
                  [enable_module_libc=yes])
    AM_CONDITIONAL([ENABLE_MODULE_LIBC],
                   [test "x$enable_module_libc" = "xyes"])
    AS_VAR_IF([enable_module_libc], [yes], [
        _CONFIG_LIBC
    ])
])
