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

lib_LTLIBRARIES = libpicotm-c.la

libpicotm_c_la_SOURCES = allocator/allocator_tx.c \
                         allocator/allocator_tx.h \
                         allocator/module.c \
                         allocator/module.h \
                         cwd/cwd.c \
                         cwd/cwd.h \
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
                         fd/chrdev.c \
                         fd/chrdev.h \
                         fd/chrdevtab.c \
                         fd/chrdevtab.h \
                         fd/chrdev_tx.c \
                         fd/chrdev_tx.h \
                         fd/dir.c \
                         fd/dir.h \
                         fd/dirtab.c \
                         fd/dirtab.h \
                         fd/dir_tx.c \
                         fd/dir_tx.h \
                         fd/fchmodop.c \
                         fd/fchmodop.h \
                         fd/fchmodoptab.c \
                         fd/fchmodoptab.h \
                         fd/fcntlop.c \
                         fd/fcntlop.h \
                         fd/fcntloptab.c \
                         fd/fcntloptab.h \
                         fd/fd.c \
                         fd/fd.h \
                         fd/fdtab.c \
                         fd/fdtab.h \
                         fd/fdtab_tx.c \
                         fd/fdtab_tx.h \
                         fd/fd_event.h \
                         fd/fd_tx.c \
                         fd/fd_tx.h \
                         fd/fifo.c \
                         fd/fifo.h \
                         fd/fifotab.c \
                         fd/fifotab.h \
                         fd/fifo_tx.c \
                         fd/fifo_tx.h \
                         fd/fildes_tx.c \
                         fd/fildes_tx.h \
                         fd/file_tx.c \
                         fd/file_tx.h \
                         fd/file_tx_ops.c \
                         fd/file_tx_ops.h \
                         fd/fileid.c \
                         fd/fileid.h \
                         fd/ioop.c \
                         fd/ioop.h \
                         fd/iooptab.c \
                         fd/iooptab.h \
                         fd/module.c \
                         fd/module.h \
                         fd/ofd.c \
                         fd/ofd.h \
                         fd/ofd_id.c \
                         fd/ofd_id.h \
                         fd/ofdtab.c \
                         fd/ofdtab.h \
                         fd/ofd_tx.c \
                         fd/ofd_tx.h \
                         fd/openop.c \
                         fd/openop.h \
                         fd/openoptab.c \
                         fd/openoptab.h \
                         fd/pipeop.c \
                         fd/pipeop.h \
                         fd/pipeoptab.c \
                         fd/pipeoptab.h \
                         fd/range.c \
                         fd/range.h \
                         fd/regfile.c \
                         fd/regfile.h \
                         fd/regfiletab.c \
                         fd/regfiletab.h \
                         fd/regfile_tx.c \
                         fd/regfile_tx.h \
                         fd/region.c \
                         fd/region.h \
                         fd/regiontab.c \
                         fd/regiontab.h \
                         fd/rwcounter.c \
                         fd/rwcounter.h \
                         fd/rwcountermap.c \
                         fd/rwcountermap.h \
                         fd/rwlockmap.c \
                         fd/rwlockmap.h \
                         fd/seekop.c \
                         fd/seekop.h \
                         fd/seekoptab.c \
                         fd/seekoptab.h \
                         fd/socket.c \
                         fd/socket.h \
                         fd/sockettab.c \
                         fd/sockettab.h \
                         fd/socket_tx.c \
                         fd/socket_tx.h \
                         libc.c \
                         libc.h \
                         sched.c \
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

AM_CPPFLAGS = -I$(top_srcdir)/modules/libc/include \
              -I$(top_srcdir)/modules/tm/include \
              -I$(top_srcdir)/include
