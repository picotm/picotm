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

current = 0
revision = 0
age = 0

lib_LTLIBRARIES = libpicotm-txlib.la

libpicotm_txlib_la_SOURCES = txlib_event.c \
                             txlib_event.h \
                             txlib_module.c \
                             txlib_module.h \
                             txlib_tx.c \
                             txlib_tx.h \
                             txlist.c \
                             txlist.h \
                             txlist_entry.c \
                             txlist_entry.h \
                             txlist_state.c \
                             txlist_state.h \
                             txlist_tx.c \
                             txlist_tx.h \
                             txqueue.c \
                             txqueue.h \
                             txqueue_entry.c \
                             txqueue_entry.h \
                             txqueue_state.c \
                             txqueue_state.h \
                             txqueue_tx.c \
                             txqueue_tx.h

libpicotm_txlib_la_LDFLAGS = -version-info $(current):$(revision):$(age)

AM_CPPFLAGS = -I$(top_srcdir)/modules/txlib/include \
              -I$(top_srcdir)/include
