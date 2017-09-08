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

TESTS = txlist-pubapi-t1.test \
        txlist-pubapi-t4.test \
        txmultiset-pubapi-t1.test \
        txmultiset-pubapi-t4.test \
        txqueue-pubapi-t1.test \
        txqueue-pubapi-t4.test \
        txstack-pubapi-t1.test \
        txstack-pubapi-t4.test

EXTRA_DIST = $(TESTS)

TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
                  $(top_srcdir)/build-aux/tap-driver.sh

check_PROGRAMS = txlist-pubapi.test \
                 txmultiset-pubapi.test \
                 txqueue-pubapi.test \
                 txstack-pubapi.test

txlist_pubapi_test_SOURCES = txlist_pubapi.c \
                             txlist_test.c \
                             txlist_test.h

txmultiset_pubapi_test_SOURCES = txmultiset_pubapi.c \
                                 txmultiset_test.c \
                                 txmultiset_test.h

txqueue_pubapi_test_SOURCES = txqueue_pubapi.c \
                              txqueue_test.c \
                              txqueue_test.h

txstack_pubapi_test_SOURCES = txstack_pubapi.c \
                              txstack_test.c \
                              txstack_test.h

AM_LDFLAGS = -static

LDADD = $(top_builddir)/tests/libtests/libpicotm_tests.la \
        $(top_builddir)/tests/libtap/libpicotm_tap.la \
        $(top_builddir)/modules/txlib/src/libpicotm-txlib.la \
        $(top_builddir)/src/libpicotm.la

AM_CPPFLAGS = -I$(top_builddir)/tests/libtests \
              -I$(top_srcdir)/tests/libtests \
              -I$(top_builddir)/tests/libtap \
              -I$(top_srcdir)/tests/libtap \
              -I$(top_builddir)/modules/txlib/include \
              -I$(top_srcdir)/modules/txlib/include \
              -I$(top_builddir)/include \
              -I$(top_srcdir)/include \
              -include picotm/config.h
