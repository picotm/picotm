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
age = 0

lib_LTLIBRARIES =
if ENABLE_MODULE_LIBC
lib_LTLIBRARIES += libpicotm-c.la
endif

libpicotm_c_la_SOURCES = allocator/allocator_event.c \
                         allocator/allocator_event.h \
                         allocator/allocator_log.c \
                         allocator/allocator_log.h \
                         allocator/allocator_tx.c \
                         allocator/allocator_tx.h \
                         allocator/module.c \
                         allocator/module.h \
                         compat/get_current_dir_name.c \
                         compat/get_current_dir_name.h \
                         compat/malloc_usable_size.c \
                         compat/malloc_usable_size.h \
                         compat/static_assert.h \
                         compat/temp_failure_retry.h \
                         cwd/cwd.c \
                         cwd/cwd.h \
                         cwd/cwd_event.c \
                         cwd/cwd_event.h \
                         cwd/cwd_log.c \
                         cwd/cwd_log.h \
                         cwd/cwd_tx.c \
                         cwd/cwd_tx.h \
                         cwd/module.c \
                         cwd/module.h \
                         errno.c \
                         error/error_tx.c \
                         error/error_tx.h \
                         error/module.c \
                         error/module.h \
                         fcntl-tm.c \
                         fcntl.c \
                         fildes/chrdev.c \
                         fildes/chrdev.h \
                         fildes/chrdevtab.c \
                         fildes/chrdevtab.h \
                         fildes/chrdev_tx.c \
                         fildes/chrdev_tx.h \
                         fildes/dir.c \
                         fildes/dir.h \
                         fildes/dirtab.c \
                         fildes/dirtab.h \
                         fildes/dir_tx.c \
                         fildes/dir_tx.h \
                         fildes/fchmodop.c \
                         fildes/fchmodop.h \
                         fildes/fchmodoptab.c \
                         fildes/fchmodoptab.h \
                         fildes/fcntlop.c \
                         fildes/fcntlop.h \
                         fildes/fcntloptab.c \
                         fildes/fcntloptab.h \
                         fildes/fd.c \
                         fildes/fd.h \
                         fildes/fdtab.c \
                         fildes/fdtab.h \
                         fildes/fdtab_tx.c \
                         fildes/fdtab_tx.h \
                         fildes/fd_tx.c \
                         fildes/fd_tx.h \
                         fildes/fifo.c \
                         fildes/fifo.h \
                         fildes/fifotab.c \
                         fildes/fifotab.h \
                         fildes/fifo_tx.c \
                         fildes/fifo_tx.h \
                         fildes/fildes.c \
                         fildes/fildes.h \
                         fildes/fildes_event.c \
                         fildes/fildes_event.h \
                         fildes/fildes_log.c \
                         fildes/fildes_log.h \
                         fildes/fildes_tx.c \
                         fildes/fildes_tx.h \
                         fildes/file_tx.c \
                         fildes/file_tx.h \
                         fildes/file_tx_ops.c \
                         fildes/file_tx_ops.h \
                         fildes/fileid.c \
                         fildes/fileid.h \
                         fildes/ioop.c \
                         fildes/ioop.h \
                         fildes/iooptab.c \
                         fildes/iooptab.h \
                         fildes/module.c \
                         fildes/module.h \
                         fildes/ofd.c \
                         fildes/ofd.h \
                         fildes/ofd_id.c \
                         fildes/ofd_id.h \
                         fildes/ofdtab.c \
                         fildes/ofdtab.h \
                         fildes/ofd_tx.c \
                         fildes/ofd_tx.h \
                         fildes/openop.c \
                         fildes/openop.h \
                         fildes/openoptab.c \
                         fildes/openoptab.h \
                         fildes/pipeop.c \
                         fildes/pipeop.h \
                         fildes/pipeoptab.c \
                         fildes/pipeoptab.h \
                         fildes/range.c \
                         fildes/range.h \
                         fildes/regfile.c \
                         fildes/regfile.h \
                         fildes/regfiletab.c \
                         fildes/regfiletab.h \
                         fildes/regfile_tx.c \
                         fildes/regfile_tx.h \
                         fildes/region.c \
                         fildes/region.h \
                         fildes/regiontab.c \
                         fildes/regiontab.h \
                         fildes/rwcounter.c \
                         fildes/rwcounter.h \
                         fildes/rwcountermap.c \
                         fildes/rwcountermap.h \
                         fildes/rwlockmap.c \
                         fildes/rwlockmap.h \
                         fildes/seekop.c \
                         fildes/seekop.h \
                         fildes/seekoptab.c \
                         fildes/seekoptab.h \
                         fildes/socket.c \
                         fildes/socket.h \
                         fildes/sockettab.c \
                         fildes/sockettab.h \
                         fildes/socket_tx.c \
                         fildes/socket_tx.h \
                         libc.c \
                         libc.h \
                         sched.c \
                         signal/module.c \
                         signal/module.h \
                         signal/signal_tx.c \
                         signal/signal_tx.h \
                         signal/sigstate.c \
                         signal/sigstate.h \
                         stdio-tm.c \
                         stdio.c \
                         stdlib-tm.c \
                         stdlib.c \
                         string-tm.c \
                         string.c \
                         sys_socket.c \
                         sys_socket-tm.c \
                         sys_stat.c \
                         sys_stat-tm.c \
                         unistd.c \
                         unistd-tm.c

libpicotm_c_la_LDFLAGS = -version-info $(current):$(revision):$(age)

AM_CPPFLAGS = -iquote $(top_builddir)/modules/libc/include \
              -iquote $(top_srcdir)/modules/libc/include \
              -iquote $(top_builddir)/modules/tm/include \
              -iquote $(top_srcdir)/modules/tm/include \
              -iquote $(top_builddir)/include \
              -iquote $(top_srcdir)/include \
              -include config.h
