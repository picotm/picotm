#!/bin/sh
#
# picotm - A system-level transaction manager
# Copyright (c) 2017    Thomas Zimmermann <contact@tzimmermann.org>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

# Run this script in the repository's top-level directory to generate
# build infrastructure.

# Fail immediately on errors
set -e

test -d build-aux/ || mkdir -p build-aux/
test -d config/ || mkdir -p config/
test -d m4/ || mkdir -p m4/

test -e ChangeLog || touch ChangeLog

# Mac OS has it's own libtoolize script, which is something different. GNU's
# libtoolize is usually called 'glibtoolize'. We use this name, if available.
LIBTOOLIZE=`which glibtoolize 2>/dev/null` || true
if test -z ${LIBTOOLIZE}; then
    LIBTOOLIZE=libtoolize
fi

# TODO: pass '-Wall -Werror' to libtool after upgrading CI to libtool >= 2.4.3
${LIBTOOLIZE} -c
aclocal -Wall -Werror -I m4/
autoheader -Wall -Werror
automake -Wall -Werror -ac
autoconf -Wall -Werror

exit 0
