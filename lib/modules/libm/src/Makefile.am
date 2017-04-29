# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

lib_LTLIBRARIES = libpicotm-m.la

libpicotm_m_la_SOURCES = complex.c \
                         fpu_tx.c \
                         fpu_tx.h \
                         libm.c \
                         math.c \
                         matherr.c \
                         matherr.h \
                         math-tm.c

AM_CPPFLAGS = -I$(top_srcdir)/modules/libm/include \
              -I$(top_srcdir)/modules/libc/include \
              -I$(top_srcdir)/modules/tm/include \
              -I$(top_srcdir)/include

AM_CFLAGS = @VISIBILITY_CFLAGS@
