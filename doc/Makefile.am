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

EXTRA_DIST = Doxyfile-html \
             Doxyfile.in

DOXYGEN_FLAGS ?=
OPTIPNG_FLAGS ?= -silent -o0

html-local: doxygen-html-optimized

install-html-local:
	@MKDIR_P@ ${htmldir} && \
		cp -fr html/ ${htmldir}

uninstall-local:
	rm -fr ${htmldir}/html/

#
# Internal rules
#

.PHONY: doxygen-html doxygen-html-optimized

# Runs doxygen to generate HTML documentation. Good for testing
# documentation changes.
doxygen-html:
if HAVE_DOXYGEN
	$(DOXYGEN) $(DOXYGEN_FLAGS) @abs_srcdir@/Doxyfile-html
endif

# Runs optipng to reduce the size of generated PNG file. This
# should only be necessary when generating the distribution.
doxygen-html-optimized: doxygen-html
if HAVE_OPTIPNG
if HAVE_FIND
	$(OPTIPNG) $(OPTIPNG_FLAGS) `$(FIND) html/ -type f -name "*.png"`
endif
endif
