# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

lib_LTLIBRARIES = libpicotm-c.la

libpicotm_c_la_SOURCES = allocator/allocator_tx.c \
                         allocator/allocator_tx.h \
                         allocator/module.c \
                         allocator/module.h \
                         counter.c \
                         counter.h \
                         errcode.h \
                         errno.c \
                         error/error_tx.c \
                         error/error_tx.h \
                         error/module.c \
                         error/module.h \
                         fcntl-tm.c \
                         fcntl.c \
                         fd/cmap.c \
                         fd/cmap.h \
                         fd/cmapss.c \
                         fd/cmapss.h \
                         fd/comfd.c \
                         fd/comfd.h \
                         fd/event.h \
                         fd/fcntlop.c \
                         fd/fcntlop.h \
                         fd/fcntloptab.c \
                         fd/fcntloptab.h \
                         fd/fd.c \
                         fd/fd.h \
                         fd/fdtab.c \
                         fd/fdtab.h \
                         fd/fdtx.c \
                         fd/fdtx_close.c \
                         fd/fdtx_fcntl.c \
                         fd/fdtx.h \
                         fd/ioop.c \
                         fd/ioop.h \
                         fd/iooptab.c \
                         fd/iooptab.h \
                         fd/module.c \
                         fd/module.h \
                         fd/ofd.c \
                         fd/ofd.h \
                         fd/ofdid.c \
                         fd/ofdid.h \
                         fd/ofdtab.c \
                         fd/ofdtab.h \
                         fd/ofdtx_bind.c \
                         fd/ofdtx.c \
                         fd/ofdtx_connect.c \
                         fd/ofdtx_fcntl.c \
                         fd/ofdtx_fsync.c \
                         fd/ofdtx.h \
                         fd/ofdtx_listen.c \
                         fd/ofdtx_lseek.c \
                         fd/ofdtx_pread.c \
                         fd/ofdtx_pwrite.c \
                         fd/ofdtx_read.c \
                         fd/ofdtx_recv.c \
                         fd/ofdtx_send.c \
                         fd/ofdtx_shutdown.c \
                         fd/ofdtx_write.c \
                         fd/openop.c \
                         fd/openop.h \
                         fd/openoptab.c \
                         fd/openoptab.h \
                         fd/pgtree.c \
                         fd/pgtree.h \
                         fd/pgtreess.c \
                         fd/pgtreess.h \
                         fd/pipeop.c \
                         fd/pipeop.h \
                         fd/pipeoptab.c \
                         fd/pipeoptab.h \
                         fd/range.c \
                         fd/range.h \
                         fd/region.c \
                         fd/region.h \
                         fd/regiontab.c \
                         fd/regiontab.h \
                         fd/rwlock.c \
                         fd/rwlock.h \
                         fd/rwlockmap.c \
                         fd/rwlockmap.h \
                         fd/rwstatemap.c \
                         fd/rwstatemap.h \
                         fd/seekop.c \
                         fd/seekop.h \
                         fd/seekoptab.c \
                         fd/seekoptab.h \
                         fs/comfs.c \
                         fs/comfs_fchdir.c \
                         fs/comfs.h \
                         fs/comfs_mkstemp.c \
                         fs/comfstx.c \
                         fs/comfstx.h \
                         fs/fchdirop.c \
                         fs/fchdirop.h \
                         fs/fchdiroptab.c \
                         fs/fchdiroptab.h \
                         libc.c \
                         mutex.c \
                         mutex.h \
                         sched.c \
                         spinlock.c \
                         spinlock.h \
                         stdlib-tm.c \
                         stdlib.c \
                         string-tm.c \
                         string.c \
                         sys_socket.c \
                         sys_socket-tm.c \
                         sys_stat.c \
                         sys_stat-tm.c \
                         types.h \
                         unistd.c \
                         unistd-tm.c

CC = $(PTHREAD_CC)

AM_CPPFLAGS = -I$(top_srcdir)/modules/libc/include \
              -I$(top_srcdir)/modules/tm/include \
              -I$(top_srcdir)/include

AM_CFLAGS = @VISIBILITY_CFLAGS@ \
            $(PTHREAD_CFLAGS)

AM_LDFLAGS = $(PTHREAD_LDFLAGS)
