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

check_LTLIBRARIES = libpicotm_tests.la

libpicotm_tests_la_SOURCES = opts.c \
                             opts.h \
                             ptr.h \
                             pubapi.c \
                             pubapi.h \
                             sysenv.c \
                             sysenv.h \
                             tempfile.c \
                             tempfile.h \
                             test.c \
                             test.h \
                             testhlp.c \
                             testhlp.h \
                             thread.c \
                             thread.h

AM_CPPFLAGS = -iquote $(top_builddir)/tests/libtap \
              -iquote $(top_srcdir)/tests/libtap \
              -iquote $(top_builddir)/tests/libsafeblk \
              -iquote $(top_srcdir)/tests/libsafeblk \
              -iquote $(top_builddir)/include \
              -iquote $(top_srcdir)/include \
              -include config.h
