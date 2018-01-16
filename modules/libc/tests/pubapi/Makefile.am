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

TESTS = allocator-pubapi-t1.test \
        allocator-pubapi-t4.test \
        cwd-pubapi-t1.test \
        cwd-pubapi-t4.test \
        fildes-pubapi-t1.test \
        fildes-pubapi-t4.test

VALGRIND_TESTS = allocator-pubapi-valgrind-t1.test \
                 cwd-pubapi-valgrind-t1.test \
                 fildes-pubapi-valgrind-t1.test

if HAVE_VALGRIND
TESTS += $(VALGRIND_TESTS)
endif

EXTRA_DIST = $(TESTS)

TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
                  $(top_srcdir)/build-aux/tap-driver.sh

check_PROGRAMS = allocator-pubapi \
                 cwd-pubapi \
                 fildes-pubapi

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

AM_CPPFLAGS = -I$(top_builddir)/tests/libtests \
              -I$(top_srcdir)/tests/libtests \
              -I$(top_builddir)/tests/libsafeblk \
              -I$(top_srcdir)/tests/libsafeblk \
              -I$(top_builddir)/tests/libtap \
              -I$(top_srcdir)/tests/libtap \
              -I$(top_builddir)/modules/libc/include \
              -I$(top_srcdir)/modules/libc/include \
              -I$(top_builddir)/modules/tm/include \
              -I$(top_srcdir)/modules/tm/include \
              -I$(top_builddir)/include \
              -I$(top_srcdir)/include \
              -include config.h
