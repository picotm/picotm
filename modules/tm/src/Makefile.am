#
# picotm - A system-level transaction manager
# Copyright (c) 2017-2018   Thomas Zimmermann <contact@tzimmermann.org>
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
age = 0

lib_LTLIBRARIES =
if ENABLE_MODULE_TM
lib_LTLIBRARIES += libpicotm-tm.la
endif

libpicotm_tm_la_SOURCES = block.h \
                          frame.c \
                          frame.h \
                          framemap.c \
                          framemap.h \
                          module.c \
                          module.h \
                          page.c \
                          page.h \
                          tm.c \
                          tm.h \
                          vmem.c \
                          vmem.h \
                          vmem_tx.c \
                          vmem_tx.h

libpicotm_tm_la_LDFLAGS = -version-info $(current):$(revision):$(age)

AM_CPPFLAGS = -iquote $(top_builddir)/modules/tm/include \
              -iquote $(top_srcdir)/modules/tm/include \
              -iquote $(top_builddir)/include \
              -iquote $(top_srcdir)/include \
              -include config.h
