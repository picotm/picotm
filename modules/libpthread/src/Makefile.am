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

current = 0
revision = 1
age = 0

lib_LTLIBRARIES =
if ENABLE_MODULE_LIBPTHREAD
lib_LTLIBRARIES += libpicotm-pthread.la
endif

libpicotm_pthread_la_SOURCES = pthread.c

libpicotm_pthread_la_LDFLAGS = -version-info $(current):$(revision):$(age)

AM_CPPFLAGS = -iquote $(top_builddir)/modules/libpthread/include \
              -iquote $(top_srcdir)/modules/libpthread/include \
              -iquote $(top_builddir)/modules/tm/include \
              -iquote $(top_srcdir)/modules/tm/include \
              -iquote $(top_builddir)/include \
              -iquote $(top_srcdir)/include \
              -include config.h
