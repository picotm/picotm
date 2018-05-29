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

check_LTLIBRARIES = libpicotm_safeblk.la

libpicotm_safeblk_la_SOURCES = compat/pthread_barrier.c \
                               compat/pthread_barrier.h \
                               compat/static_assert.h \
                               compat/temp_failure_retry.h \
                               safeblk.c \
                               safeblk.h \
                               safe_fcntl.c \
                               safe_fcntl.h \
                               safe_pthread.c \
                               safe_pthread.h \
                               safe_sched.c \
                               safe_sched.h \
                               safe_stdio.c \
                               safe_stdio.h \
                               safe_stdlib.c \
                               safe_stdlib.h \
                               safe_sys_stat.c \
                               safe_sys_stat.h \
                               safe_sys_time.c \
                               safe_sys_time.h \
                               safe_time.c \
                               safe_time.h \
                               safe_unistd.c \
                               safe_unistd.h

AM_CPPFLAGS = -iquote $(top_builddir)/tests/libtap \
              -iquote $(top_srcdir)/tests/libtap \
              -include config.h
