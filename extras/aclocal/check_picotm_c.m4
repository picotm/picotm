#
# SYNOPSIS
#
#   CHECK_PICOTM_C
#
# LICENSE
#
#   Copyright (c) 2017-2018 Thomas Zimmermann <contact@tzimmermann.org>
#
#   Copying and distribution of this file, with or without modification,
#   are permitted in any medium without royalty provided the copyright
#   notice and this notice are preserved.  This file is offered as-is,
#   without any warranty.
#
#   SPDX-License-Identifier: FSFAP
#

#serial 1

AC_DEFUN([CHECK_PICOTM_C], [
  AC_CHECK_LIB([picotm-c], [abort_tx])
])
