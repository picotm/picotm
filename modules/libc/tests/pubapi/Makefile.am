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

TESTS = allocator-pubapi-t1.test \
        allocator-pubapi-t4.test \
        cwd-pubapi-t1.test \
        cwd-pubapi-t4.test \
        fildes-pubapi-t1.test \
        fildes-pubapi-t4.test

EXTRA_DIST = $(TESTS)

TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
                  $(top_srcdir)/build-aux/tap-driver.sh

check_PROGRAMS = allocator-pubapi.test \
                 cwd-pubapi.test \
                 fildes-pubapi.test

allocator_pubapi_test_SOURCES = delay.c \
                                delay.h \
                                allocator_pubapi.c \
                                allocator_test.c \
                                allocator_test.h

cwd_pubapi_test_SOURCES = cwd_pubapi.c \
                          cwd_test.c \
                          cwd_test.h \
                          delay.c \
                          delay.h

fildes_pubapi_test_SOURCES = delay.c \
                             delay.h \
                             fildes_pubapi.c \
                             fildes_test.c \
                             fildes_test.h

AM_LDFLAGS = -static

LDADD = $(top_builddir)/tests/libtests/libpicotm_tests.la \
        $(top_builddir)/tests/libtap/libpicotm_tap.la \
        $(top_builddir)/modules/libc/src/libpicotm-c.la \
        $(top_builddir)/modules/tm/src/libpicotm-tm.la \
        $(top_builddir)/src/libpicotm.la

AM_CPPFLAGS = -I$(top_builddir)/tests/libtests \
              -I$(top_srcdir)/tests/libtests \
              -I$(top_builddir)/tests/libtap \
              -I$(top_srcdir)/tests/libtap \
              -I$(top_builddir)/modules/libc/include \
              -I$(top_srcdir)/modules/libc/include \
              -I$(top_builddir)/modules/tm/include \
              -I$(top_srcdir)/modules/tm/include \
              -I$(top_builddir)/include \
              -I$(top_srcdir)/include \
              -include config.h
