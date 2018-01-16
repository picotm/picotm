#
# SYNOPSIS
#
#   CONFIG_TXLIB
#
# LICENSE
#
#   Copyright (c) 2017-2018 Thomas Zimmermann <tdz@users.sourceforge.net>
#
#   Copying and distribution of this file, with or without modification,
#   are permitted in any medium without royalty provided the copyright
#   notice and this notice are preserved.  This file is offered as-is,
#   without any warranty.

AC_DEFUN([CONFIG_TXLIB], [
    CONFIG_TEST([modules/txlib/tests/pubapi/txlist-pubapi-valgrind-t1.test])
    CONFIG_TEST([modules/txlib/tests/pubapi/txmultiset-pubapi-valgrind-t1.test])
    CONFIG_TEST([modules/txlib/tests/pubapi/txqueue-pubapi-valgrind-t1.test])
    CONFIG_TEST([modules/txlib/tests/pubapi/txstack-pubapi-valgrind-t1.test])
])
