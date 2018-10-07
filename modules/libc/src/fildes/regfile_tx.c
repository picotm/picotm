/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "regfile_tx.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-tab.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "compat/temp_failure_retry.h"
#include "fchmodop.h"
#include "fchmodoptab.h"
#include "fcntlop.h"
#include "fcntloptab.h"
#include "file_tx_ops.h"
#include "ioop.h"
#include "iooptab.h"
#include "ofd_tx.h"
#include "range.h"
#include "region.h"
#include "regiontab.h"
#include "seekop.h"
#include "seekoptab.h"

static void
regfile_tx_2pl_release_locks(struct regfile_tx* self)
{
    assert(self);

    /* release all locks */

    struct region* region = self->locktab;
    const struct region* regionend = region+self->locktablen;

    while (region < regionend) {
        regfile_unlock_region(self->regfile,
                              region->offset,
                              region->nbyte,
                              &self->rwcountermap);
        ++region;
    }
}

static void
regfile_tx_try_rdlock_field(struct regfile_tx* self, enum regfile_field field,
                            struct picotm_error* error)
{
    assert(self);

    regfile_try_rdlock_field(self->regfile, field, self->rwstate + field,
                             error);
}

static void
regfile_tx_try_wrlock_field(struct regfile_tx* self, enum regfile_field field,
                            struct picotm_error* error)
{
    assert(self);

    regfile_try_wrlock_field(self->regfile, field, self->rwstate + field,
                             error);
}

static void
init_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end)
{
    while (beg < end) {
        picotm_rwstate_init(beg);
        ++beg;
    }
}

static void
uninit_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end)
{
    while (beg < end) {
        picotm_rwstate_uninit(beg);
        ++beg;
    }
}

static void
unlock_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end,
                struct regfile* regfile)
{
    enum regfile_field field = 0;

    while (beg < end) {
        regfile_unlock_field(regfile, field, beg);
        ++field;
        ++beg;
    }
}

static struct regfile_tx*
regfile_tx_of_file_tx(struct file_tx* file_tx)
{
    assert(file_tx);
    assert(file_tx_file_type(file_tx) == PICOTM_LIBC_FILE_TYPE_REGULAR);

    return picotm_containerof(file_tx, struct regfile_tx, base);
}

static void
ref(struct file_tx* file_tx, struct picotm_error* error)
{
    regfile_tx_ref(regfile_tx_of_file_tx(file_tx), error);
}

static void
unref(struct file_tx* file_tx)
{
    regfile_tx_unref(regfile_tx_of_file_tx(file_tx));
}

static off_t
append_to_iobuffer(struct regfile_tx* self, size_t nbyte, const void* buf,
                   struct picotm_error* error)
{
    off_t bufoffset;

    assert(self);

    bufoffset = self->wrbuflen;

    if (nbyte && buf) {

        /* resize */
        void* tmp = picotm_tabresize(self->wrbuf,
                                     self->wrbuflen,
                                     self->wrbuflen+nbyte,
                                     sizeof(self->wrbuf[0]),
                                     error);
        if (picotm_error_is_set(error)) {
            return (off_t)-1;
        }
        self->wrbuf = tmp;

        /* append */
        memcpy(self->wrbuf+self->wrbuflen, buf, nbyte);
        self->wrbuflen += nbyte;
    }

    return bufoffset;
}

static int
regfile_tx_append_to_writeset(struct regfile_tx* self, size_t nbyte, off_t offset,
                              const void* buf, struct picotm_error* error)
{
    assert(self);

    off_t bufoffset = append_to_iobuffer(self, nbyte, buf, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    unsigned long res = iooptab_append(&self->wrtab,
                                       &self->wrtablen,
                                       &self->wrtabsiz,
                                       nbyte, offset, bufoffset,
                                       error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return res;
}

int
regfile_tx_append_to_readset(struct regfile_tx* self, size_t nbyte, off_t offset,
                             const void* buf, struct picotm_error* error)
{
    assert(self);

    unsigned long res = iooptab_append(&self->rdtab,
                                       &self->rdtablen,
                                       &self->rdtabsiz,
                                       nbyte, offset, 0,
                                       error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return res;
}

/*
 * Concurrency Control
 */

static int
regfile_tx_2pl_lock_region(struct regfile_tx* self, size_t nbyte, off_t offset,
                           bool iswrite, struct picotm_error* error)
{
    void (* const try_lock_region[])(struct regfile*,
                                     off_t,
                                     size_t,
                                     struct rwcountermap*,
                                     struct picotm_error*) = {
        regfile_try_rdlock_region,
        regfile_try_wrlock_region
    };

    assert(self);

    try_lock_region[iswrite](self->regfile,
                             offset,
                             nbyte,
                             &self->rwcountermap,
                             error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int pos = regiontab_append(&self->locktab,
                               &self->locktablen,
                               &self->locktabsiz,
                               nbyte, offset,
                               error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    return pos;
}

/*
 * fchmod()
 */

static int
fchmod_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            mode_t mode, bool isnoundo, int* cookie,
            struct picotm_error* error)
{
    assert(cookie);

    struct regfile_tx* self = regfile_tx_of_file_tx(base);

    /* Acquire file-mode lock. */
    regfile_tx_try_wrlock_field(self, REGFILE_FIELD_FILE_MODE, error);
    if (picotm_error_is_set(error)) {
        picotm_error_set_errno(error, errno);
        return -1;
    }

    struct stat buf;
    int res = fstat(fildes, &buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }

    mode_t old_mode = buf.st_mode & ~S_IFMT;

    res = fchmod(fildes, mode);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }

    *cookie = fchmodoptab_append(&self->fchmodtab,
                                 &self->fchmodtablen,
                                 mode, old_mode,
                                 error);
    if (picotm_error_is_set(error)) {
        picotm_error_set_errno(error, errno);
    }

    return res;
}

static void
fchmod_undo(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            int cookie, struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);

    const struct fchmodop* op = self->fchmodtab + cookie;

    int res = fchmod(fildes, op->old_mode);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

/*
 * fcntl()
 */

static int
fcntl_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes, int cmd,
           union fcntl_arg* arg, bool isnoundo, int* cookie,
           struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);

    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN: {

            /* Read-lock regular file */
            regfile_tx_try_rdlock_field(self, REGFILE_FIELD_STATE, error);
            if (picotm_error_is_set(error)) {
                return -1;
            }

            int res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd));
            arg->arg0 = res;
            if (res < 0) {
                picotm_error_set_errno(error, errno);
                return res;
            }
            return res;
        }
        case F_GETLK: {

            /* Read-lock regular file */
            regfile_tx_try_rdlock_field(self, REGFILE_FIELD_STATE, error);
            if (picotm_error_is_set(error)) {
                return -1;
            }

            int res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg1));
            if (res < 0) {
                picotm_error_set_errno(error, errno);
                return res;
            }
            return res;
        }
        case F_SETFL:
        case F_SETFD:
        case F_SETOWN: {

            if (!isnoundo) {
                picotm_error_set_revocable(error);
                return -1;
            }

            int res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg0));
            if (res < 0) {
                picotm_error_set_errno(error, errno);
                return res;
            }
            return res;
        }
        case F_SETLK:
        case F_SETLKW: {

            if (!isnoundo) {
                picotm_error_set_revocable(error);
                return -1;
            }

            int res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg1));
            if (res < 0) {
                picotm_error_set_errno(error, errno);
                return res;
            }
            return res;
        }
        default:
            picotm_error_set_errno(error, EINVAL);
            return -1;
    }
}

/*
 * fstat()
 */

static int
fstat_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           struct stat* buf, bool isnoundo, int* cookie,
           struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);

    /* Acquire reader locks. */

    regfile_tx_try_rdlock_field(self, REGFILE_FIELD_FILE_MODE, error);
    if (picotm_error_is_set(error)) {
        picotm_error_set_errno(error, errno);
        return -1;
    }
    regfile_tx_try_rdlock_field(self, REGFILE_FIELD_FILE_SIZE, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* get transaction-local file size */
    off_t file_size = regfile_tx_get_file_size(self, fildes, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int res = fstat(fildes, buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }

    buf->st_size = file_size;

    return res;
}

/*
 * fsync()
 */

static int
fsync_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           bool isnoundo, int* cookie, struct picotm_error* error)
{
    assert(cookie);

    struct regfile_tx* self = regfile_tx_of_file_tx(base);

    if (self->wrmode == PICOTM_LIBC_WRITE_BACK) {
        *cookie = 0; /* apply fsync() during commit */
        return 0;
    }

    int res = fsync(fildes);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

static void
fsync_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            int cookie, struct picotm_error* error)
{
    int res = fsync(fildes);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

/*
 * lseek()
 */

static off_t
file_offset_after_lseek(struct regfile_tx* self, struct ofd_tx* ofd_tx,
                        int fildes, off_t offset, int whence,
                        struct picotm_error* error)
{
    switch (whence) {
        case SEEK_SET:
            if (offset < 0) {
                /* negative result */
                picotm_error_set_errno(error, EINVAL);
                return (off_t)-1;
            }
            return offset;
        case SEEK_CUR: {
            off_t pos = ofd_tx_get_file_offset(ofd_tx, fildes, error);
            if (picotm_error_is_set(error)) {
                return pos;
            }
            if (offset < 0) {
                if ((pos + offset) < 0) {
                    /* negative result */
                    picotm_error_set_errno(error, EINVAL);
                    return (off_t)-1;
                }
            } else if (offset > 0) {
                if ((pos + offset) < pos) {
                    /* overflow */
                    picotm_error_set_errno(error, EOVERFLOW);
                    return (off_t)-1;
                }
            }
            return pos + offset;
        }
        case SEEK_END: {
            off_t size = regfile_tx_get_file_size(self, fildes, error);
            if (picotm_error_is_set(error)) {
                return size;
            }
            if (offset < 0) {
                if ((size + offset) < 0) {
                    /* negative result */
                    picotm_error_set_errno(error, EINVAL);
                    return (off_t)-1;
                }
            } else if (offset > 0) {
                if ((size + offset) < size) {
                    /* overflow */
                    picotm_error_set_errno(error, EOVERFLOW);
                    return (off_t)-1;
                }
            }
            return size + offset;
        }
        default:
            picotm_error_set_errno(error, EINVAL);
            return (off_t)-1;
    }
}

static off_t
lseek_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           off_t offset, int whence, bool isnoundo, int* cookie,
           struct picotm_error* error)
{
	off_t from = ofd_tx_get_file_offset(ofd_tx, fildes, error);
    if (picotm_error_is_set(error)) {
        return from;
    }

	/* Fastpath: Read current position */
	if (!offset && (whence == SEEK_CUR)) {
        return from;
	}

    struct regfile_tx* self = regfile_tx_of_file_tx(base);

    off_t to = file_offset_after_lseek(self, ofd_tx, fildes, offset,
                                       whence, error);
    if (picotm_error_is_set(error)) {
        return to;
    }

    ofd_tx_set_file_offset(ofd_tx, to, error);
    if (picotm_error_is_set(error)) {
        return (off_t)-1;
    }

    if (self->wrmode == PICOTM_LIBC_WRITE_THROUGH) {
        off_t res = lseek(fildes, to, SEEK_SET);
        if (res == (off_t)-1) {
            picotm_error_set_errno(error, errno);
            return res;
        }
    }

    if (cookie) {
        *cookie = seekoptab_append(&ofd_tx->seektab,
                                   &ofd_tx->seektablen,
                                    from, offset, whence,
                                    error);
        if (picotm_error_is_set(error)) {
            return (off_t)-1;
        }
    }

    return to;
}

static void
lseek_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            int cookie, struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);

    if (self->wrmode != PICOTM_LIBC_WRITE_BACK) {
        return;
    }

    off_t res = lseek(fildes, ofd_tx->seektab[cookie].offset,
                              ofd_tx->seektab[cookie].whence);
    if (res == (off_t)-1) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

static void
lseek_undo(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           int cookie, struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);

    if (self->wrmode != PICOTM_LIBC_WRITE_THROUGH) {
        return;
    }

    off_t res = lseek(fildes, ofd_tx->seektab[cookie].from, SEEK_SET);
    if (res == (off_t)-1) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

/*
 * pread()
 */

/*

Busy-wait instead of syscall

typedef unsigned long long ticks;
static __inline__ ticks getticks(void)
{
     unsigned a, d;
     __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));
     return ((ticks)a) | (((ticks)d) << 32);
}

static ssize_t
pread_wait(int fildes, void *buf, size_t nbyte, off_t off)
{
    ticks t0 = getticks();
    memset(buf, 0, nbyte);
    ticks t1 = getticks();

    if (!nbyte) {
        while ( (t1-t0) < 631 ) {
            t1 = getticks();
        }
    } else {

        // approximation of single-thread cycles
        long limit = 1020 + (339*nbyte)/1000;

        while ( (t1-t0) < limit ) {
            t1 = getticks();
        }
    }

    return nbyte;
}*/

static bool
errno_signals_blocking_io(int errno_code)
{
    return (errno_code == EAGAIN) || (errno_code == EWOULDBLOCK);
}

static ssize_t
do_pread(int fildes, void* buf, size_t nbyte, off_t offset,
         struct picotm_error* error)
{
    uint8_t* pos = buf;
    const uint8_t* beg = pos;
    const uint8_t* end = beg + nbyte;
    off_t off = offset;

    while (pos < end) {

        ssize_t res = TEMP_FAILURE_RETRY(pread(fildes, pos, end - pos, off));
        if (res < 0) {
            if (pos != beg) {
                break; /* return read data */
            } else if (errno_signals_blocking_io(errno)) {
                return -1; /* error for non-blocking I/O */
            } else {
                picotm_error_set_errno(error, errno);
                return res;
            }
        } else if (!res) {
            break; /* EOF reached */
        }

        off += res;
        pos += res;
    }

    return pos - beg;
}

static ssize_t
pread_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes, void* buf,
           size_t nbyte, off_t offset, bool isnoundo, int* cookie,
           struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);

    /* lock region */
    regfile_tx_2pl_lock_region(self, nbyte, offset, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* read from file */
    ssize_t res = do_pread(fildes, buf, nbyte, offset, error);
    if (picotm_error_is_set(error)) {
        return res;
    }
    size_t len = res;

    /* read from local write set */
    size_t len2 = iooptab_read(self->wrtab,
                               self->wrtablen,
                               buf, nbyte, offset, self->wrbuf);

    return llmax(len, len2);
}

/*
 * pwrite()
 */

/*

Busy-wait instead of syscall

typedef unsigned long long ticks;
static __inline__ ticks getticks(void)
{
     unsigned a, d;
     __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));
     return ((ticks)a) | (((ticks)d) << 32);
}

static ssize_t
pwrite_wait(int fildes, const void *buf, size_t nbyte, off_t off)
{
    ticks t0 = getticks();
    ticks t1 = getticks();

    if (!nbyte) {
        while ( (t1-t0) < 765 ) {
            t1 = getticks();
        }
    } else {

        // approximation of single-thread cycles
        long limit = 1724 + (1139*nbyte)/1000;

        while ( (t1-t0) < limit ) {
            t1 = getticks();
        }
    }

    return nbyte;
}*/

static ssize_t
do_pwrite(int fildes, const void* buf, size_t nbyte, off_t offset,
          struct picotm_error* error)
{
    const uint8_t* pos = buf;
    const uint8_t* beg = pos;
    const uint8_t* end = beg + nbyte;
    off_t off = offset;

    while (pos < end) {

        ssize_t res = TEMP_FAILURE_RETRY(pwrite(fildes, pos, end - pos, off));
        if (res < 0) {
            if (pos != beg) {
                break; /* return written data */
            } else if (errno_signals_blocking_io(errno)) {
                return -1; /* error for non-blocking I/O */
            } else {
                picotm_error_set_errno(error, errno);
                return res;
            }
        }

        off += res;
        pos += res;
    }

    return pos - beg;
}

static ssize_t
pwrite_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            const void* buf, size_t nbyte, off_t offset, bool isnoundo,
            int* cookie, struct picotm_error* error)
{
    assert(cookie);

    struct regfile_tx* self = regfile_tx_of_file_tx(base);

    if ((self->wrmode == PICOTM_LIBC_WRITE_THROUGH) && !isnoundo) {
        picotm_error_set_revocable(error);
        return -1;
    }

    /* lock region */
    regfile_tx_2pl_lock_region(self, nbyte, offset, 1, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    size_t len;

    if (self->wrmode == PICOTM_LIBC_WRITE_THROUGH) {
        ssize_t res = do_pwrite(fildes, buf, nbyte, offset, error);
        if (picotm_error_is_set(error)) {
            return res;
        }
        len = res;
    } else {
        /* add data to write set */
        *cookie = regfile_tx_append_to_writeset(self, nbyte, offset, buf, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
        len = nbyte;
    }

    /* update file size if necessary */

    off_t file_size = regfile_tx_get_file_size(self, fildes, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    if (file_size < (off_t)(offset + len)) {
        regfile_tx_set_file_size(self, offset + len, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return len;
}

static void
pwrite_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
             int cookie, struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);

    if (self->wrmode != PICOTM_LIBC_WRITE_BACK) {
        return;
    }

    off_t  off = self->wrtab[cookie].off;
    size_t nbyte = self->wrtab[cookie].nbyte;
    off_t  bufoff = self->wrtab[cookie].bufoff;

    do_pwrite(fildes, self->wrbuf + bufoff, nbyte, off, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * read()
 */

static ssize_t
read_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes, void* buf,
          size_t nbyte, bool isnoundo, int* cookie,
          struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);

    off_t offset = ofd_tx_get_file_offset(ofd_tx, fildes, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* read-lock region */

    regfile_tx_2pl_lock_region(self, nbyte, offset, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* read from file */

    ssize_t res = do_pread(fildes, buf, nbyte, offset, error);
    if (picotm_error_is_set(error)) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    size_t len = res;

    /* read from local write set */

    ssize_t len2 = iooptab_read(self->wrtab,
                                self->wrtablen, buf, nbyte, offset,
                                self->wrbuf);

    res = llmax(len, len2);

    /* update file offset */

    ofd_tx_set_file_offset(ofd_tx, offset + res, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* add buf to write set */
    *cookie = regfile_tx_append_to_readset(self, nbyte, offset, buf, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    return res;
}

static void
read_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           int cookie, struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);

    off_t offset = self->rdtab[cookie].off + self->rdtab[cookie].nbyte;

    off_t res = lseek(fildes, offset, SEEK_SET);
    if (res == (off_t)-1) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

/*
 * write()
 */

static ssize_t
do_pwrite_and_lseek(int fildes, const void* buf, size_t nbyte, off_t offset,
                    struct picotm_error* error)
{
    ssize_t res_ssize_t = do_pwrite(fildes, buf, nbyte, offset, error);
    if (picotm_error_is_set(error)) {
        return res_ssize_t;
    }
    size_t len = res_ssize_t;

    off_t res_off_t = lseek(fildes, offset + len, SEEK_SET);
    if (res_off_t == (off_t)-1) {
        picotm_error_set_errno(error, errno);
        return -1;
    }

    return len;
}

static ssize_t
write_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           const void* buf, size_t nbyte, bool isnoundo, int* cookie,
           struct picotm_error* error)
{
    assert(*cookie);

    struct regfile_tx* self = regfile_tx_of_file_tx(base);

    if ((self->wrmode == PICOTM_LIBC_WRITE_THROUGH) && !isnoundo) {
        picotm_error_set_revocable(error);
        return -1;
    }

    off_t offset = ofd_tx_get_file_offset(ofd_tx, fildes, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* write-lock region */
    regfile_tx_2pl_lock_region(self, nbyte, offset, 1, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    size_t len;

    if (self->wrmode == PICOTM_LIBC_WRITE_THROUGH) {
        ssize_t res = do_pwrite_and_lseek(fildes, buf, nbyte, offset, error);
        if (picotm_error_is_set(error)) {
            return res;
        }
        len = res;
    } else {
        /* add buf to write set */
        *cookie = regfile_tx_append_to_writeset(self, nbyte, offset, buf, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
        len = nbyte;
    }

    /* update file offset */
    ofd_tx_set_file_offset(ofd_tx, offset + len, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* update file size if necessary */

    off_t file_size = regfile_tx_get_file_size(self, fildes, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    if (file_size < (off_t)(offset + len)) {
        regfile_tx_set_file_size(self, offset + len, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return len;
}

static void
write_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            int cookie, struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);

    if (self->wrmode != PICOTM_LIBC_WRITE_BACK) {
        return;
    }

    off_t  off = self->wrtab[cookie].off;
    size_t nbyte = self->wrtab[cookie].nbyte;
    off_t  bufoff = self->wrtab[cookie].bufoff;

    do_pwrite_and_lseek(fildes, self->wrbuf + bufoff, nbyte, off, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    /* TODO: error is less than len bytes were written */
}

/*
 * Public interface
 */

static void
finish(struct file_tx* base)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);

    /* release record locks */
    regfile_tx_2pl_release_locks(self);

    /* release reader/writer locks on file state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->regfile);
}

/*
 * Public interface
 */

static const struct file_tx_ops regfile_tx_ops = {
    PICOTM_LIBC_FILE_TYPE_REGULAR,
    /* ref counting */
    ref,
    unref,
    /* module interfaces */
    finish,
    /* file ops */
    file_tx_op_accept_exec_enotsock,
    NULL,
    NULL,
    file_tx_op_bind_exec_enotsock,
    NULL,
    NULL,
    file_tx_op_connect_exec_enotsock,
    NULL,
    NULL,
    fchmod_exec,
    file_tx_op_apply,
    fchmod_undo,
    fcntl_exec,
    file_tx_op_apply,
    file_tx_op_undo,
    fstat_exec,
    file_tx_op_apply,
    file_tx_op_undo,
    fsync_exec,
    fsync_apply,
    file_tx_op_undo,
    file_tx_op_listen_exec_enotsock,
    NULL,
    NULL,
    lseek_exec,
    lseek_apply,
    lseek_undo,
    pread_exec,
    file_tx_op_apply,
    file_tx_op_undo,
    pwrite_exec,
    pwrite_apply,
    file_tx_op_undo,
    read_exec,
    read_apply,
    file_tx_op_undo,
    file_tx_op_recv_exec_enotsock,
    NULL,
    NULL,
    file_tx_op_send_exec_enotsock,
    NULL,
    NULL,
    file_tx_op_shutdown_exec_enotsock,
    NULL,
    NULL,
    write_exec,
    write_apply,
    file_tx_op_undo
};

void
regfile_tx_init(struct regfile_tx* self)
{
    assert(self);

    picotm_ref_init(&self->ref, 0);

    file_tx_init(&self->base, &regfile_tx_ops);

    self->regfile = NULL;

    self->wrmode = PICOTM_LIBC_WRITE_BACK;

    self->wrbuf = NULL;
    self->wrbuflen = 0;
    self->wrbufsiz = 0;

    self->wrtab = NULL;
    self->wrtablen = 0;
    self->wrtabsiz = 0;

    self->rdtab = NULL;
    self->rdtablen = 0;
    self->rdtabsiz = 0;

    self->fchmodtab = NULL;
    self->fchmodtablen = 0;

    self->fcntltab = NULL;
    self->fcntltablen = 0;

    self->file_size = 0;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));

    rwcountermap_init(&self->rwcountermap);

    self->locktab = NULL;
    self->locktablen = 0;
    self->locktabsiz = 0;
}

void
regfile_tx_uninit(struct regfile_tx* self)
{
    assert(self);

    free(self->locktab);
    rwcountermap_uninit(&self->rwcountermap);

    fcntloptab_clear(&self->fcntltab, &self->fcntltablen);
    fchmodoptab_clear(&self->fchmodtab, &self->fchmodtablen);
    iooptab_clear(&self->wrtab, &self->wrtablen);
    iooptab_clear(&self->rdtab, &self->rdtablen);
    free(self->wrbuf);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));
}

off_t
regfile_tx_get_file_size(struct regfile_tx* self, int fildes,
                      struct picotm_error* error)
{
    assert(self);

    enum picotm_rwstate_status lock_status =
        picotm_rwstate_get_status(self->rwstate + REGFILE_FIELD_FILE_SIZE);

    if (lock_status != PICOTM_RWSTATE_UNLOCKED) {
        /* fast path: we have already read the file size. */
        return self->file_size;
    }

    /* Read-lock file-size field and ... */
    regfile_tx_try_rdlock_field(self, REGFILE_FIELD_FILE_SIZE, error);
    if (picotm_error_is_set(error)) {
        return (off_t)-1;
    }

    /* ...get current file buffer size. */
    struct stat buf;
    int res = fstat(fildes, &buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return (off_t)res;
    }
    self->file_size = buf.st_size;

    return self->file_size;
}

void
regfile_tx_set_file_size(struct regfile_tx* self, off_t size,
                         struct picotm_error* error)
{
    assert(self);

    regfile_tx_try_wrlock_field(self, REGFILE_FIELD_FILE_SIZE, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    self->file_size = size;
}

/*
 * Referencing
 */

void
regfile_tx_ref_or_set_up(struct regfile_tx* self, struct regfile* regfile,
                         struct picotm_error* error)
{
    assert(self);
    assert(regfile);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    /* get reference on file */
    regfile_ref(regfile, error);
    if (picotm_error_is_set(error)) {
        goto err_regfile_ref;
    }

    /* setup fields */

    self->regfile = regfile;

    self->wrmode = PICOTM_LIBC_WRITE_BACK;

    self->file_size = 0;

    self->fchmodtablen = 0;
    self->fcntltablen = 0;
    self->rdtablen = 0;
    self->wrtablen = 0;
    self->wrbuflen = 0;

    self->locktablen = 0;

    return;

err_regfile_ref:
    picotm_ref_down(&self->ref);
}

void
regfile_tx_ref(struct regfile_tx* self, struct picotm_error* error)
{
    picotm_ref_up(&self->ref);
}

void
regfile_tx_unref(struct regfile_tx* self)
{
    assert(self);

    bool final_ref = picotm_ref_down(&self->ref);
    if (!final_ref) {
        return;
    }

    regfile_unref(self->regfile);
    self->regfile = NULL;
}

bool
regfile_tx_holds_ref(struct regfile_tx* self)
{
    assert(self);

    return picotm_ref_count(&self->ref) > 0;
}
