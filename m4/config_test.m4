#
# SYNOPSIS
#
#   CONFIG_TEST(file)
#
# LICENSE
#
#   Copyright (c) 2018  Thomas Zimmermann <tdz@users.sourceforge.net>
#
#   Copying and distribution of this file, with or without modification,
#   are permitted in any medium without royalty provided the copyright
#   notice and this notice are preserved.  This file is offered as-is,
#   without any warranty.

AC_DEFUN([CONFIG_TEST], [
    AC_CONFIG_FILES([$1], [chmod +x $1])
])
