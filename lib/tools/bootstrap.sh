#!/bin/sh

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Run this script in the repository's top-level directory to generate
# build infrastructure.

test -d build-aux/ || mkdir -p build-aux/
test -d m4/ || mkdir -p m4/

test -e ChangeLog || touch ChangeLog

libtoolize -c
aclocal -I m4/
automake -ac
autoconf
