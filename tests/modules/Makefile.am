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

TESTS = module-tm-t1.test \
        module-tm-t4.test \
        module-malloc-t1.test \
        module-malloc-t4.test \
        module-fildes-t1.test \
        module-fildes-t4.test \
        module-vfs-t1.test \
        module-vfs-t4.test

EXTRA_DIST = $(TESTS)

TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
                  $(top_srcdir)/build-aux/tap-driver.sh

check_PROGRAMS = picotm-test

picotm_test_SOURCES = fildes_test.c \
                      fildes_test.h \
                      main.c \
                      malloc_test.c \
                      malloc_test.h \
                      module.c \
                      module.h \
                      opts.c \
                      opts.h \
                      ptr.h \
                      safe_fcntl.c \
                      safe_fcntl.h \
                      safe_pthread.c \
                      safe_pthread.h \
                      safe_stdio.c \
                      safe_stdio.h \
                      safe_stdlib.c \
                      safe_stdlib.h \
                      safe_sys_stat.c \
                      safe_sys_stat.h \
                      safe_sys_time.c \
                      safe_sys_time.h \
                      safe_unistd.c \
                      safe_unistd.h \
                      tap.c \
                      tap.h \
                      taputils.c \
                      taputils.h \
                      tempfile.c \
                      tempfile.h \
                      test.c \
                      test.h \
                      testhlp.c \
                      testhlp.h \
                      test_state.c \
                      test_state.h \
                      tm_test.c \
                      tm_test.h \
                      vfs_test.c \
                      vfs_test.h

picotm_test_LDADD = $(top_builddir)/modules/libpthread/src/libpicotm-pthread.la \
                    $(top_builddir)/modules/libm/src/libpicotm-m.la \
                    $(top_builddir)/modules/libc/src/libpicotm-c.la \
                    $(top_builddir)/modules/tm/src/libpicotm-tm.la \
                    $(top_builddir)/src/libpicotm.la

AM_CPPFLAGS = -I$(top_srcdir)/modules/libpthread/include \
              -I$(top_srcdir)/modules/libm/include \
              -I$(top_srcdir)/modules/libc/include \
              -I$(top_srcdir)/modules/tm/include \
              -I$(top_srcdir)/include
