#
# picotm - A system-level transaction manager
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
revision = 1
age = 0

lib_LTLIBRARIES =
if ENABLE_MODULE_LIBPTHREAD
lib_LTLIBRARIES += libpicotm-pthread.la
endif

libpicotm_pthread_la_SOURCES = pthread.c

libpicotm_pthread_la_LDFLAGS = -version-info $(current):$(revision):$(age)

AM_CPPFLAGS = -iquote $(top_builddir)/modules/libpthread/include \
              -iquote $(top_srcdir)/modules/libpthread/include \
              -iquote $(top_builddir)/modules/tm/include \
              -iquote $(top_srcdir)/modules/tm/include \
              -iquote $(top_builddir)/include \
              -iquote $(top_srcdir)/include \
              -include config.h
