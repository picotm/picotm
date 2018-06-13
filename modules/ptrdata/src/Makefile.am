#
# picotm - A system-level transaction manager
# Copyright (c) 2018    Thomas Zimmermann <contact@tzimmermann.org>
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

current = 0
revision = 0
age = 0

lib_LTLIBRARIES =
if ENABLE_MODULE_PTRDATA
lib_LTLIBRARIES += libpicotm-ptrdata.la
endif

libpicotm_ptrdata_la_SOURCES = module.c \
                               module.h \
                               picotm-ptrdata.c \
                               picotm-ptrdata.h \
                               ptrdata.c \
                               ptrdata.h \
                               ptrdata_tx.c \
                               ptrdata_tx.h

libpicotm_ptrdata_la_LDFLAGS = -version-info $(current):$(revision):$(age)

AM_CPPFLAGS = -iquote $(top_builddir)/modules/ptrdata/include \
              -iquote $(top_srcdir)/modules/ptrdata/include \
              -iquote $(top_builddir)/include \
              -iquote $(top_srcdir)/include \
              -include config.h
