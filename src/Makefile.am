#
# MIT License
# Copyright (c) 2017-2018   Thomas Zimmermann <tdz@users.sourceforge.net>
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
# SPDX-License-Identifier: MIT

current = 4
revision = 0
age = 1

lib_LTLIBRARIES = libpicotm.la

libpicotm_la_SOURCES = module.c \
                       module.h \
                       picotm.c \
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
                       picotm_os_cond.c \
                       picotm_os_cond.h \
                       picotm_os_mutex.c \
                       picotm_os_mutex.h \
                       picotm_os_rwlock.c \
                       picotm_os_rwlock.h \
                       picotm_os_timespec.c \
                       picotm_os_timespec.h \
                       table.c \
                       table.h \
                       tx.c \
                       tx.h \
                       tx_shared.c \
                       tx_shared.h

libpicotm_la_LDFLAGS = -version-info $(current):$(revision):$(age)

AM_CPPFLAGS = -I$(top_builddir)/include \
              -I$(top_srcdir)/include \
              -include config.h
