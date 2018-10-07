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

current = 7
revision = 0
age = 0

lib_LTLIBRARIES = libpicotm.la

libpicotm_la_SOURCES = picotm.c \
                       picotm.h \
                       picotm-error.c \
                       picotm-lib-rwlock.c \
                       picotm-lib-rwlock.h \
                       picotm-lib-rwstate.c \
                       picotm-lib-shared-ref-obj.c \
                       picotm-lib-shared-treemap.c \
                       picotm-lib-tab.c \
                       picotm-lib-treemap.c \
                       picotm_event.c \
                       picotm_event.h \
                       picotm_lock_manager.c \
                       picotm_lock_manager.h \
                       picotm_lock_owner.c \
                       picotm_lock_owner.h \
                       picotm_log.c \
                       picotm_log.h \
                       picotm_module.c \
                       picotm_module.h \
                       picotm_os_cond.c \
                       picotm_os_cond.h \
                       picotm_os_mutex.c \
                       picotm_os_mutex.h \
                       picotm_os_rwlock.c \
                       picotm_os_rwlock.h \
                       picotm_os_timespec.c \
                       picotm_os_timespec.h \
                       picotm_tx.c \
                       picotm_tx.h \
                       table.c \
                       table.h

libpicotm_la_LDFLAGS = -version-info $(current):$(revision):$(age)

AM_CPPFLAGS = -iquote $(top_builddir)/include \
              -iquote $(top_srcdir)/include \
              -include config.h
