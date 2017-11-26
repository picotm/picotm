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

AC_DEFUN([CONFIG_LIBC], [

    #
    # libc compile-time constants
    #

    AC_DEFINE([MAXNUMFD], [1024], [Maximum number of file descriptors])
    AC_DEFINE([RECBITS], [5], [Bits per file record])
])
