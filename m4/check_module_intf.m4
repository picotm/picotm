#
# SYNOPSIS
#
#   _CHECK_MODULE_INTF
#
# LICENSE
#
#   Copyright (c) 2017 Thomas Zimmermann <tdz@users.sourceforge.net>
#
#   Copying and distribution of this file, with or without modification,
#   are permitted in any medium without royalty provided the copyright
#   notice and this notice are preserved.  This file is offered as-is,
#   without any warranty.

AC_DEFUN([_CHECK_MODULE_INTF], [
    AC_CHECK_DECL([$2],
                  [AC_DEFINE(AS_TR_CPP([PICOTM_$1_HAVE_$2]),
                             [1],
                             [Define to 1 if you have $2.])],
                  [],
                  [$3])
])
