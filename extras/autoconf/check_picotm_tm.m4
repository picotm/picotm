#
# SYNOPSIS
#
#   CHECK_PICOTM_TM
#
# LICENSE
#
#   Copyright (c) 2017 Thomas Zimmermann <contact@tzimmermann.org>
#
#   Copying and distribution of this file, with or without modification,
#   are permitted in any medium without royalty provided the copyright
#   notice and this notice are preserved.  This file is offered as-is,
#   without any warranty.

AC_DEFUN([CHECK_PICOTM_TM], [
  AC_CHECK_LIB([picotm-tm], [__picotm_tm_load])
])
