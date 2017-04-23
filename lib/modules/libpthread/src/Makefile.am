# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

lib_LTLIBRARIES = libpicotm-pthread.la

libpicotm_pthread_la_SOURCES = pthread.c

CC = $(PTHREAD_CC)

AM_CPPFLAGS = -I$(top_srcdir)/modules/libpthread/include \
              -I$(top_srcdir)/modules/tm/include \
              -I$(top_srcdir)/include

AM_CFLAGS = @VISIBILITY_CFLAGS@ \
            $(PTHREAD_CFLAGS)

AM_LDFLAGS = $(PTHREAD_LDFLAGS)
