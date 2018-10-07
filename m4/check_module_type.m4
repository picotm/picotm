#
# SYNOPSIS
#
#   _CHECK_MODULE_TYPE
#
# LICENSE
#
#   Copyright (c) 2017 Thomas Zimmermann <contact@tzimmermann.org>
#
#   Copying and distribution of this file, with or without modification,
#   are permitted in any medium without royalty provided the copyright
#   notice and this notice are preserved.  This file is offered as-is,
#   without any warranty.

AC_DEFUN([_CHECK_MODULE_TYPE], [
    AC_CHECK_TYPE([$2],
                  [AC_DEFINE(AS_TR_CPP([PICOTM_$1_HAVE_TYPE_$2]),
                             [1],
                             [Define to 1 if the system has the type `$2'.])],
                  [],
                  [$3])
])
