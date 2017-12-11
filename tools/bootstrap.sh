#!/bin/sh

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

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
