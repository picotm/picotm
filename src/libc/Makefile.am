# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

lib_LTLIBRARIES = libsystx-c.la

libsystx_c_la_SOURCES = alloc/comalloc.c \
                        alloc/comalloc.h \
                        alloc/comalloc_free.c \
                        alloc/comalloc_posix_memalign.c \
                        alloc/comalloctx.c \
                        alloc/comalloctx.h \
                        alloc/rnd2wb.c \
                        alloc/rnd2wb.h \
                        counter.c \
                        counter.h \
                        fd/cmap.c \
                        fd/cmap.h \
                        fd/cmapss.c \
                        fd/cmapss.h \
                        fd/comfd_accept.c \
                        fd/comfd_bind.c \
                        fd/comfd.c \
                        fd/comfd_close.c \
                        fd/comfd_connect.c \
                        fd/comfd_dup.c \
                        fd/comfd_fcntl.c \
                        fd/comfd_fsync.c \
                        fd/comfd.h \
                        fd/comfd_listen.c \
                        fd/comfd_lseek.c \
                        fd/comfd_open.c \
                        fd/comfd_pipe.c \
                        fd/comfd_pread.c \
                        fd/comfd_pwrite.c \
                        fd/comfd_read.c \
                        fd/comfd_recv.c \
                        fd/comfd_select.c \
                        fd/comfd_send.c \
                        fd/comfd_shutdown.c \
                        fd/comfd_socket.c \
                        fd/comfd_sync.c \
                        fd/comfdtx.c \
                        fd/comfdtx.h \
                        fd/comfd_write.c \
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
                        spinlock.c \
                        spinlock.h \
                        stdlib-tm.c \
                        stdlib.c \
                        string-tm.c \
                        string.c \
                        types.h \
                        unistd.c

CC = $(PTHREAD_CC)

AM_CPPFLAGS = -I$(top_srcdir)/include \
              -I$(top_srcdir)/src

AM_CFLAGS = @VISIBILITY_CFLAGS@ \
            $(PTHREAD_CFLAGS)

AM_LDFLAGS = $(PTHREAD_LDFLAGS)
