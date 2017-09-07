#
# SYNOPSIS
#
#   CHECK_LIBM
#
# LICENSE
#
#   Copyright (c) 2017 Thomas Zimmermann <tdz@users.sourceforge.net>
#
#   Copying and distribution of this file, with or without modification,
#   are permitted in any medium without royalty provided the copyright
#   notice and this notice are preserved.  This file is offered as-is,
#   without any warranty.

AC_DEFUN([CHECK_LIBM], [
  AC_CHECK_LIB([m], [signgam])
])
