#
# MIT License
# Copyright (c) 2017    Thomas Zimmermann <tdz@users.sourceforge.net>
#
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
#
# SPDX-License-Identifier: MIT

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
