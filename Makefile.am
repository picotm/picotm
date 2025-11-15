#
# picotm - A system-level transaction manager
# Copyright (c) 2017    Thomas Zimmermann
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
#

#
# The dependency chain between the directories is
#
#   include -> src -> tests -> modules -> doc
#
# We first create the core headers and library from `include' and `src'.
# The `tests' directory only contains helpers for `make check'. It does not
# contain artifacts for building the regular code, but probably depends on
# the core interfaces. Modules in `modules' depend on the core library, each
# other, and their tests depend on the test infrastructure. The documentation
# in `doc' can depend on any source or header files.
#
SUBDIRS = include \
          src \
          tests \
          modules \
          doc \
          extras \
          tools \
          LICENSES

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = COPYING \
             COPYING.LESSER
