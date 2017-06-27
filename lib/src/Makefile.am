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

lib_LTLIBRARIES = libpicotm.la

libpicotm_la_SOURCES = log.c \
                       log.h \
                       module.c \
                       module.h \
                       picotm.c \
                       picotm.h \
                       picotm-error.c \
                       picotm-lib-rwlock.c \
                       picotm-lib-tab.c \
                       table.c \
                       table.h \
                       tx.c \
                       tx.h \
                       tx_shared.c \
                       tx_shared.h

AM_CPPFLAGS = -I$(srcdir)/../include

AM_CFLAGS = @VISIBILITY_CFLAGS@
