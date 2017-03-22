# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SUBDIRS =

lib_LTLIBRARIES = libsystx.la

libsystx_la_SOURCES = component.c \
                      component.h \
                      indirectcalls.c \
                      log.c \
                      log.h \
                      systx.c \
                      systx.h \
                      table.c \
                      table.h \
                      tx.c \
                      tx.h

CC = $(PTHREAD_CC)

AM_CPPFLAGS = -I$(srcdir)/../include

AM_CFLAGS = @VISIBILITY_CFLAGS@ \
            $(PTHREAD_CFLAGS)

AM_LDFLAGS = $(PTHREAD_LDFLAGS)
