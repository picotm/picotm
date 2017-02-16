# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SUBDIRS = libc \
          libm \
          libpthread

lib_LTLIBRARIES = libsystx.la

libsystx_la_SOURCES = comerror.c \
                      comerror.h \
                      comerror_failoncommit.c \
                      comerror_pop.c \
                      comerror_push.c \
                      comerrortx.c \
                      comerrortx.h \
                      component.c \
                      component.h \
                      ext-actions.c \
                      ext-actions.h \
                      indirectcalls.c \
                      log.c \
                      log.h \
                      table.c \
                      table.h

CC = $(PTHREAD_CC)

AM_CPPFLAGS = -I$(srcdir)/../include

AM_CFLAGS = $(PTHREAD_CFLAGS)

AM_LDFLAGS = $(PTHREAD_LDFLAGS)
