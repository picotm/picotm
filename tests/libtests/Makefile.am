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

check_LTLIBRARIES = libpicotm_tests.la

libpicotm_tests_la_SOURCES = opts.c \
                             opts.h \
                             ptr.h \
                             safeblk.c \
                             safeblk.h \
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
                             tempfile.c \
                             tempfile.h \
                             test.c \
                             test.h \
                             testhlp.c \
                             testhlp.h \
                             thread.c \
                             thread.h

AM_CPPFLAGS = -I$(top_builddir)/tests/libtap \
              -I$(top_srcdir)/tests/libtap \
              -I$(top_builddir)/include \
              -I$(top_srcdir)/include \
              -include picotm/config.h
