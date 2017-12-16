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
          extras

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = LICENSE
