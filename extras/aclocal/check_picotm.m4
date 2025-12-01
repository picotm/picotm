#
# SYNOPSIS
#
#   CHECK_PICOTM
#
# LICENSE
#
#   Copying and distribution of this file, with or without modification,
#   are permitted in any medium without royalty provided the copyright
#   notice and this notice are preserved.  This file is offered as-is,
#   without any warranty.
#
#   SPDX-License-Identifier: FSFAP
#

#serial 1

AC_DEFUN([CHECK_PICOTM], [
  AC_CHECK_LIB([picotm], [__picotm_commit])
])
