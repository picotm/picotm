#
# MIT License
# Copyright (c) 2017-2018   Thomas Zimmermann <tdz@users.sourceforge.net>
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

PUBAPI_TESTS = allocator-pubapi-t1.test \
               allocator-pubapi-t1-valgrind.test \
               allocator-pubapi-t4.test \
               cwd-pubapi-t1.test \
               cwd-pubapi-t1-valgrind.test \
               cwd-pubapi-t4.test \
               fildes-pubapi-t1.test \
               fildes-pubapi-t1-valgrind.test \
               fildes-pubapi-t4.test

TESTS =
if ENABLE_MODULE_LIBC
TESTS += $(PUBAPI_TESTS)
endif

EXTRA_DIST = $(PUBAPI_TESTS)

# Prepare environment for test scripts.
AM_TESTS_ENVIRONMENT = \
    PATH="$(top_srcdir)/tests/env:${PATH}"; \
    . $(top_builddir)/tests/env/tests-env.sh;

TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
                  $(top_srcdir)/build-aux/tap-driver.sh

if ENABLE_MODULE_LIBC
check_PROGRAMS = allocator-pubapi \
                 cwd-pubapi \
                 fildes-pubapi
endif

allocator_pubapi_SOURCES = allocator_pubapi.c

cwd_pubapi_SOURCES = cwd_pubapi.c

fildes_pubapi_SOURCES = fildes_pubapi.c

AM_LDFLAGS = -static

LDADD = $(top_builddir)/tests/libtests/libpicotm_tests.la \
        $(top_builddir)/tests/libsafeblk/libpicotm_safeblk.la \
        $(top_builddir)/tests/libtap/libpicotm_tap.la \
        $(top_builddir)/modules/libc/src/libpicotm-c.la \
        $(top_builddir)/modules/tm/src/libpicotm-tm.la \
        $(top_builddir)/src/libpicotm.la

AM_CPPFLAGS = -iquote $(top_builddir)/tests/libtests \
              -iquote $(top_srcdir)/tests/libtests \
              -iquote $(top_builddir)/tests/libsafeblk \
              -iquote $(top_srcdir)/tests/libsafeblk \
              -iquote $(top_builddir)/tests/libtap \
              -iquote $(top_srcdir)/tests/libtap \
              -iquote $(top_builddir)/modules/libc/include \
              -iquote $(top_srcdir)/modules/libc/include \
              -iquote $(top_builddir)/modules/tm/include \
              -iquote $(top_srcdir)/modules/tm/include \
              -iquote $(top_builddir)/include \
              -iquote $(top_srcdir)/include \
              -include config.h
