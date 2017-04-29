# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

lib_LTLIBRARIES = libpicotm.la

libpicotm_la_SOURCES = log.c \
                       log.h \
                       module.c \
                       module.h \
                       picotm.c \
                       picotm.h \
                       ptr.h \
                       table.c \
                       table.h \
                       tx.c \
                       tx.h \
                       tx_shared.c \
                       tx_shared.h

AM_CPPFLAGS = -I$(srcdir)/../include

AM_CFLAGS = @VISIBILITY_CFLAGS@
