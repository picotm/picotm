# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

lib_LTLIBRARIES = libsystx-tm.la

libsystx_tm_la_SOURCES = frame.c \
                         frame.h \
                         tm.c \
                         tm.h \
                         vmem.c \
                         vmem.h \
                         vmem_tx.c \
                         vmem_tx.h

CC = $(PTHREAD_CC)

AM_CPPFLAGS = -I$(top_srcdir)/include

AM_CFLAGS = @VISIBILITY_CFLAGS@ \
            $(PTHREAD_CFLAGS)

AM_LDFLAGS = $(PTHREAD_LDFLAGS)
