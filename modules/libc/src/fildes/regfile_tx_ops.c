/*
 * picotm - A system-level transaction manager
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

#include "regfile_tx_ops.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-ptr.h"
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
#include "range.h"
#include "regfile_tx.h"
#include "seekbuf_tx.h"
#include "seekop.h"
#include "seekoptab.h"

/*
 * File handling
 */

static void
prepare(struct file_tx* file_tx, struct file* file, void* data,
        struct picotm_error* error)
{
    regfile_tx_prepare(regfile_tx_of_file_tx(file_tx), regfile_of_base(file),
                       error);
}

static void
release(struct file_tx* file_tx)
{
    regfile_tx_release(regfile_tx_of_file_tx(file_tx));
}

/*
 * Module interface
 */

static void
finish(struct file_tx* base)
{
    regfile_tx_finish(regfile_tx_of_file_tx(base));
}

/*
 * fchmod()
 */

static int
fchmod_exec(struct file_tx* base,
            int fildes, mode_t mode,
            bool isnoundo, int* cookie, struct picotm_error* error)
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
fchmod_undo(struct file_tx* base, int fildes, int cookie,
            struct picotm_error* error)
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
fcntl_exec(struct file_tx* base,
           int fildes, int cmd, union fcntl_arg* arg,
           bool isnoundo, int* cookie, struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);
    struct seekbuf_tx* seekbuf_tx = self->seekbuf_tx;

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
            seekbuf_tx_try_rdlock_field(seekbuf_tx, SEEKBUF_FIELD_STATE,
                                        error);
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
fstat_exec(struct file_tx* base,
           int fildes, struct stat* buf,
           bool isnoundo, int* cookie, struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);
    struct seekbuf_tx* seekbuf_tx = self->seekbuf_tx;

    /* Acquire reader locks. */

    regfile_tx_try_rdlock_field(self, REGFILE_FIELD_FILE_MODE, error);
    if (picotm_error_is_set(error)) {
        picotm_error_set_errno(error, errno);
        return -1;
    }
    seekbuf_tx_try_rdlock_field(seekbuf_tx, SEEKBUF_FIELD_FILE_SIZE, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* get transaction-local file size */
    off_t file_size = seekbuf_tx_get_file_size(seekbuf_tx, fildes, error);
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
fsync_exec(struct file_tx* base,
           int fildes,
           bool isnoundo, int* cookie, struct picotm_error* error)
{
    assert(cookie);

    struct regfile_tx* self = regfile_tx_of_file_tx(base);
    struct seekbuf_tx* seekbuf_tx = self->seekbuf_tx;

    if (seekbuf_tx->wrmode == PICOTM_LIBC_WRITE_BACK) {
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
fsync_apply(struct file_tx* base, int fildes, int cookie,
            struct picotm_error* error)
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
file_offset_after_lseek(struct regfile_tx* self, int fildes, off_t offset,
                        int whence, struct picotm_error* error)
{
    struct seekbuf_tx* seekbuf_tx = self->seekbuf_tx;

    switch (whence) {
        case SEEK_SET:
            if (offset < 0) {
                /* negative result */
                picotm_error_set_errno(error, EINVAL);
                return (off_t)-1;
            }
            return offset;
        case SEEK_CUR: {
            off_t pos = regfile_tx_get_file_offset(self, fildes, error);
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
            off_t size = seekbuf_tx_get_file_size(seekbuf_tx, fildes, error);
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
lseek_exec(struct file_tx* base,
           int fildes, off_t offset, int whence,
           bool isnoundo, int* cookie, struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);

	off_t from = regfile_tx_get_file_offset(self, fildes, error);
    if (picotm_error_is_set(error)) {
        return from;
    }

	/* Fastpath: Read current position */
	if (!offset && (whence == SEEK_CUR)) {
        return from;
	}

    off_t to = file_offset_after_lseek(self, fildes, offset, whence, error);
    if (picotm_error_is_set(error)) {
        return to;
    }

    regfile_tx_set_file_offset(self, to, error);
    if (picotm_error_is_set(error)) {
        return (off_t)-1;
    }

    struct seekbuf_tx* seekbuf_tx = self->seekbuf_tx;

    if (seekbuf_tx->wrmode == PICOTM_LIBC_WRITE_THROUGH) {
        off_t res = lseek(fildes, to, SEEK_SET);
        if (res == (off_t)-1) {
            picotm_error_set_errno(error, errno);
            return res;
        }
    }

    if (cookie) {
        *cookie = seekoptab_append(&self->seektab,
                                   &self->seektablen,
                                   from, offset, whence,
                                   error);
        if (picotm_error_is_set(error)) {
            return (off_t)-1;
        }
    }

    return to;
}

static void
lseek_apply(struct file_tx* base, int fildes, int cookie,
            struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);
    struct seekbuf_tx* seekbuf_tx = self->seekbuf_tx;

    if (seekbuf_tx->wrmode != PICOTM_LIBC_WRITE_BACK) {
        return;
    }

    off_t res = lseek(fildes, self->seektab[cookie].offset,
                              self->seektab[cookie].whence);
    if (res == (off_t)-1) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

static void
lseek_undo(struct file_tx* base, int fildes, int cookie,
           struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);
    struct seekbuf_tx* seekbuf_tx = self->seekbuf_tx;

    if (seekbuf_tx->wrmode != PICOTM_LIBC_WRITE_THROUGH) {
        return;
    }

    off_t res = lseek(fildes, self->seektab[cookie].from, SEEK_SET);
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
pread_exec(struct file_tx* base,
           int fildes, void* buf, size_t nbyte, off_t offset,
           bool isnoundo, int* cookie, struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);
    struct seekbuf_tx* seekbuf_tx = self->seekbuf_tx;

    /* lock region */
    seekbuf_tx_try_rdlock_region(seekbuf_tx, nbyte, offset, error);
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
    size_t len2 = iooptab_read(seekbuf_tx->wrtab,
                               seekbuf_tx->wrtablen,
                               buf, nbyte, offset, seekbuf_tx->wrbuf);

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
pwrite_exec(struct file_tx* base,
            int fildes, const void* buf, size_t nbyte, off_t offset,
            bool isnoundo, int* cookie, struct picotm_error* error)
{
    assert(cookie);

    struct regfile_tx* self = regfile_tx_of_file_tx(base);
    struct seekbuf_tx* seekbuf_tx = self->seekbuf_tx;

    if ((seekbuf_tx->wrmode == PICOTM_LIBC_WRITE_THROUGH) && !isnoundo) {
        picotm_error_set_revocable(error);
        return -1;
    }

    /* lock region */
    seekbuf_tx_try_wrlock_region(seekbuf_tx, nbyte, offset, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    size_t len;

    if (seekbuf_tx->wrmode == PICOTM_LIBC_WRITE_THROUGH) {
        ssize_t res = do_pwrite(fildes, buf, nbyte, offset, error);
        if (picotm_error_is_set(error)) {
            return res;
        }
        len = res;
    } else {
        /* add data to write set */
        *cookie = seekbuf_tx_append_to_writeset(seekbuf_tx, nbyte, offset,
                                                buf, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
        len = nbyte;
    }

    /* update file size if necessary */

    off_t file_size = seekbuf_tx_get_file_size(seekbuf_tx, fildes, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    if (file_size < (off_t)(offset + len)) {
        seekbuf_tx_set_file_size(seekbuf_tx, offset + len, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return len;
}

static void
pwrite_apply(struct file_tx* base, int fildes, int cookie,
             struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);
    struct seekbuf_tx* seekbuf_tx = self->seekbuf_tx;

    if (seekbuf_tx->wrmode != PICOTM_LIBC_WRITE_BACK) {
        return;
    }

    off_t  off = seekbuf_tx->wrtab[cookie].off;
    size_t nbyte = seekbuf_tx->wrtab[cookie].nbyte;
    off_t  bufoff = seekbuf_tx->wrtab[cookie].bufoff;

    do_pwrite(fildes, seekbuf_tx->wrbuf + bufoff, nbyte, off, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * read()
 */

static ssize_t
read_exec(struct file_tx* base,
          int fildes, void* buf, size_t nbyte,
          bool isnoundo, int* cookie, struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);
    struct seekbuf_tx* seekbuf_tx = self->seekbuf_tx;

    off_t offset = regfile_tx_get_file_offset(self, fildes, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* read-lock region */

    seekbuf_tx_try_rdlock_region(seekbuf_tx, nbyte, offset, error);
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

    ssize_t len2 = iooptab_read(seekbuf_tx->wrtab,
                                seekbuf_tx->wrtablen, buf, nbyte, offset,
                                seekbuf_tx->wrbuf);

    res = llmax(len, len2);

    /* update file offset */

    regfile_tx_set_file_offset(self, offset + res, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* add buf to write set */
    *cookie = seekbuf_tx_append_to_readset(seekbuf_tx, nbyte, offset, buf,
                                           error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    return res;
}

static void
read_apply(struct file_tx* base, int fildes, int cookie,
           struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);
    struct seekbuf_tx* seekbuf_tx = self->seekbuf_tx;

    off_t offset = seekbuf_tx->rdtab[cookie].off +
                   seekbuf_tx->rdtab[cookie].nbyte;

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
write_exec(struct file_tx* base,
           int fildes, const void* buf, size_t nbyte,
           bool isnoundo, int* cookie, struct picotm_error* error)
{
    assert(*cookie);

    struct regfile_tx* self = regfile_tx_of_file_tx(base);
    struct seekbuf_tx* seekbuf_tx = self->seekbuf_tx;

    if ((seekbuf_tx->wrmode == PICOTM_LIBC_WRITE_THROUGH) && !isnoundo) {
        picotm_error_set_revocable(error);
        return -1;
    }

    off_t offset = regfile_tx_get_file_offset(self, fildes, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* write-lock region */
    seekbuf_tx_try_wrlock_region(seekbuf_tx, nbyte, offset, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    size_t len;

    if (seekbuf_tx->wrmode == PICOTM_LIBC_WRITE_THROUGH) {
        ssize_t res = do_pwrite_and_lseek(fildes, buf, nbyte, offset, error);
        if (picotm_error_is_set(error)) {
            return res;
        }
        len = res;
    } else {
        /* add buf to write set */
        *cookie = seekbuf_tx_append_to_writeset(seekbuf_tx, nbyte, offset,
                                                buf, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
        len = nbyte;
    }

    /* update file offset */
    regfile_tx_set_file_offset(self, offset + len, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* update file size if necessary */

    off_t file_size = seekbuf_tx_get_file_size(seekbuf_tx, fildes, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    if (file_size < (off_t)(offset + len)) {
        seekbuf_tx_set_file_size(seekbuf_tx, offset + len, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return len;
}

static void
write_apply(struct file_tx* base, int fildes, int cookie,
            struct picotm_error* error)
{
    struct regfile_tx* self = regfile_tx_of_file_tx(base);
    struct seekbuf_tx* seekbuf_tx = self->seekbuf_tx;

    if (seekbuf_tx->wrmode != PICOTM_LIBC_WRITE_BACK) {
        return;
    }

    off_t  off = seekbuf_tx->wrtab[cookie].off;
    size_t nbyte = seekbuf_tx->wrtab[cookie].nbyte;
    off_t  bufoff = seekbuf_tx->wrtab[cookie].bufoff;

    do_pwrite_and_lseek(fildes, seekbuf_tx->wrbuf + bufoff, nbyte, off,
                        error);
    if (picotm_error_is_set(error)) {
        return;
    }

    /* TODO: error if less than len bytes were written */
}

/*
 * Public interface
 */

const struct file_tx_ops regfile_tx_ops = {
    PICOTM_LIBC_FILE_TYPE_REGULAR,
    /* file handling */
    prepare,
    release,
    /* module interfaces */
    finish,
    /* file ops */
    file_tx_op_accept_exec_enotsock,
    nullptr,
    nullptr,
    file_tx_op_bind_exec_enotsock,
    nullptr,
    nullptr,
    file_tx_op_connect_exec_enotsock,
    nullptr,
    nullptr,
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
    nullptr,
    nullptr,
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
    nullptr,
    nullptr,
    file_tx_op_send_exec_enotsock,
    nullptr,
    nullptr,
    file_tx_op_shutdown_exec_enotsock,
    nullptr,
    nullptr,
    write_exec,
    write_apply,
    file_tx_op_undo
};
