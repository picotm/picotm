#
# picotm - A system-level transaction manager
# Copyright (c) 2017-2018   Thomas Zimmermann
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
#

current = 1
revision = 3
age = 1

lib_LTLIBRARIES =
if ENABLE_MODULE_TXLIB
lib_LTLIBRARIES += libpicotm-txlib.la
endif

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
                             txmultiset.c \
                             txmultiset.h \
                             txmultiset_entry.c \
                             txmultiset_entry.h \
                             txmultiset_state.c \
                             txmultiset_state.h \
                             txmultiset_tx.c \
                             txmultiset_tx.h \
                             txqueue.c \
                             txqueue.h \
                             txqueue_entry.c \
                             txqueue_entry.h \
                             txqueue_state.c \
                             txqueue_state.h \
                             txqueue_tx.c \
                             txqueue_tx.h \
                             txstack.c \
                             txstack.h \
                             txstack_entry.c \
                             txstack_entry.h \
                             txstack_state.c \
                             txstack_state.h \
                             txstack_tx.c \
                             txstack_tx.h

libpicotm_txlib_la_LDFLAGS = -version-info $(current):$(revision):$(age)

AM_CPPFLAGS = -iquote $(top_builddir)/modules/txlib/include \
              -iquote $(top_srcdir)/modules/txlib/include \
              -iquote $(top_builddir)/include \
              -iquote $(top_srcdir)/include \
              -include config.h
