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

current = 5
revision = 0
age = 1

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
                         compat/cmp_eq_files.c \
                         compat/cmp_eq_files.h \
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
                         fildes/file.c \
                         fildes/file.h \
                         fildes/filebuf.c \
                         fildes/filebuf.h \
                         fildes/filebuf_id.c \
                         fildes/filebuf_id.h \
                         fildes/filebuftab.c \
                         fildes/filebuftab.h \
                         fildes/filetab.c \
                         fildes/filetab.h \
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
                         fildes/openop.c \
                         fildes/openop.h \
                         fildes/openoptab.c \
                         fildes/openoptab.h \
                         fildes/pipebuf.c \
                         fildes/pipebuf.h \
                         fildes/pipebuftab.c \
                         fildes/pipebuftab.h \
                         fildes/pipebuf_tx.c \
                         fildes/pipebuf_tx.h \
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
                         fildes/seekbuf.c \
                         fildes/seekbuf.h \
                         fildes/seekbuftab.c \
                         fildes/seekbuftab.h \
                         fildes/seekbuf_tx.c \
                         fildes/seekbuf_tx.h \
                         fildes/seekop.c \
                         fildes/seekop.h \
                         fildes/seekoptab.c \
                         fildes/seekoptab.h \
                         fildes/sockbuf.c \
                         fildes/sockbuf.h \
                         fildes/sockbuftab.c \
                         fildes/sockbuftab.h \
                         fildes/sockbuf_tx.c \
                         fildes/sockbuf_tx.h \
                         fildes/socket.c \
                         fildes/socket.h \
                         fildes/sockettab.c \
                         fildes/sockettab.h \
                         fildes/socket_tx.c \
                         fildes/socket_tx.h \
                         libc.c \
                         libc.h \
                         locale-tm.c \
                         locale.c \
                         locale/locale.c \
                         locale/locale.h \
                         locale/locale_event.c \
                         locale/locale_event.h \
                         locale/locale_log.c \
                         locale/locale_log.h \
                         locale/locale_tx.c \
                         locale/locale_tx.h \
                         locale/module.c \
                         locale/module.h \
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
                         time.c \
                         unistd.c \
                         unistd-tm.c

libpicotm_c_la_LDFLAGS = -version-info $(current):$(revision):$(age)

AM_CPPFLAGS = -iquote $(top_builddir)/modules/libc/include \
              -iquote $(top_srcdir)/modules/libc/include \
              -iquote $(top_builddir)/modules/tm/include \
              -iquote $(top_srcdir)/modules/tm/include \
              -iquote $(top_builddir)/modules/ptrdata/include \
              -iquote $(top_srcdir)/modules/ptrdata/include \
              -iquote $(top_builddir)/include \
              -iquote $(top_srcdir)/include \
              -include config.h
