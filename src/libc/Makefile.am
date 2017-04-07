# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

lib_LTLIBRARIES = libsystx-c.la

libsystx_c_la_SOURCES = alloc/comalloc.c \
                        alloc/comalloc.h \
                        alloc/comalloc_free.c \
                        alloc/comalloc_posix_memalign.c \
                        alloc/comalloctx.c \
                        alloc/comalloctx.h \
                        alloc/rnd2wb.c \
                        alloc/rnd2wb.h \
                        counter.c \
                        counter.h \
                        mutex.c \
                        mutex.h \
                        spinlock.c \
                        spinlock.h \
                        stdlib-tm.c \
                        stdlib.c \
                        string-tm.c \
                        string.c \
                        types.h \
                        unistd.c

CC = $(PTHREAD_CC)

AM_CPPFLAGS = -I$(top_srcdir)/include \
              -I$(top_srcdir)/src \
              -I$(top_srcdir)/src/libc/txproto

AM_CFLAGS = @VISIBILITY_CFLAGS@ \
            $(PTHREAD_CFLAGS)

AM_LDFLAGS = $(PTHREAD_LDFLAGS)
