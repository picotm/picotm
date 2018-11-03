#!/bin/sh
#
# picotm - A system-level transaction manager
# Copyright (c) 2017-2018   Thomas Zimmermann <contact@tzimmermann.org>
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

# Fail immediately on errors
set -e

test_aclocal_paths_for_filename () {

    filename=$1

    paths=`aclocal --print-ac-dir`
    paths="${paths}:${ACLOCAL_PATH}"

    i="1"
    while :
    do
        path=`echo "${paths}" | cut -s -f${i} -d: -`
        if [ -z "${path}" ]; then
            # An empty string signals the end of the paths
            # variable. Return failure.
            return 1;
        elif [ -f "${path}/${filename}" ]; then
            # We've found the required file in one of the
            # paths. Return success.
            return 0
        fi
        i=$((i + 1))
    done
}

test -f configure.ac ||
    (echo "Error: configure.ac could not be found. Run this script"\
          "in the repository's top-level directory to generate the"\
          "build infrastructure."
     exit 1)

test -d .git ||
    (echo "Warning: Git repository could not be found. You should"\
          "only have to bootstrap the build infrastructure if you"\
          "intend to do development on picotm. This requires a copy"\
          "of picotm's Git repository. With Git installed, you can"\
          "get your copy by entering"\
          "\`git clone https://github.com/picotm/picotm.git'. See the"\
          "README for more information.")

test_aclocal_paths_for_filename "ax_pthread.m4" ||
    (echo "Warning: Autoconf macro archive could not be found. The"\
          "macros are provided in your Unix distribution's package"\
          "repository. Add the path name of the directory containing"\
          "ax_pthread.m4 to the environment variable ACLOCAL_PATH; it's"\
          "usually something like /usr/share/aclocal or"\
          "/usr/local/share/aclocal. See \`info automake' for more"\
          "information.")

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
