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

#include "chrdev_tx.h"

#include "fcntlop.h"
#include "fcntloptab.h"
#include "file_tx.h"
#include "file_tx_ops.h"
#include "ioop.h"
#include "iooptab.h"
#include "seekbuf_tx.h"

#include "compat/temp_failure_retry.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-tab.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static void
chrdev_tx_try_rdlock_field(struct chrdev_tx* self, enum chrdev_field field,
                           struct picotm_error* error)
{
    assert(self);

    chrdev_try_rdlock_field(chrdev_of_base(self->base.file), field,
                            self->rwstate + field, error);
}

static void
chrdev_tx_try_wrlock_field(struct chrdev_tx* self, enum chrdev_field field,
                           struct picotm_error* error)
{
    assert(self);

    chrdev_try_wrlock_field(chrdev_of_base(self->base.file), field,
                            self->rwstate + field, error);
}

/*
 * struct file_tx_ops
 */

static void
chrdev_tx_op_prepare(struct file_tx* file_tx, struct file* file, void* data,
                     struct picotm_error* error)
{
    struct chrdev_tx* self = chrdev_tx_of_file_tx(file_tx);

    self->seekbuf_tx = nullptr;
    self->fcntltablen = 0;
}

static void
chrdev_tx_op_release(struct file_tx* file_tx)
{ }

static void
unlock_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end,
                struct chrdev* chrdev)
{
    enum chrdev_field field = 0;

    while (beg < end) {
        chrdev_unlock_field(chrdev, field, beg);
        ++field;
        ++beg;
    }
}

static void
chrdev_tx_op_finish(struct file_tx* base)
{
    struct chrdev_tx* self = chrdev_tx_of_file_tx(base);

    /* release reader/writer locks on character-device state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    chrdev_of_base(self->base.file));
}

/* fcntl() */

static int
chrdev_tx_op_fcntl_exec(struct file_tx* base,
                        int fildes, int cmd, union fcntl_arg* arg,
                        bool isnoundo, int* cookie, struct picotm_error* error)
{
    struct chrdev_tx* self = chrdev_tx_of_file_tx(base);
    struct seekbuf_tx* seekbuf_tx = self->seekbuf_tx;

    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN: {

            /* Read-lock character device */
            chrdev_tx_try_rdlock_field(self, CHRDEV_FIELD_STATE, error);
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

            /* Read-lock character device */
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

/* fstat() */

static int
chrdev_tx_op_fstat_exec(struct file_tx* base,
                        int fildes, struct stat* buf,
                        bool isnoundo, int* cookie, struct picotm_error* error)
{
    struct chrdev_tx* self = chrdev_tx_of_file_tx(base);

    /* Acquire file-mode reader lock. */
    chrdev_tx_try_rdlock_field(self, CHRDEV_FIELD_FILE_MODE, error);
    if (picotm_error_is_set(error)) {
        picotm_error_set_errno(error, errno);
        return -1;
    }

    int res = fstat(fildes, buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

/* read() */

static bool
errno_signals_blocking_io(int errno_code)
{
    return (errno_code == EAGAIN) || (errno_code == EWOULDBLOCK);
}

static ssize_t
do_read(int fildes, void* buf, size_t nbyte, struct picotm_error* error)
{
    uint8_t* pos = buf;
    const uint8_t* beg = pos;
    const uint8_t* end = beg + nbyte;

    while (pos < end) {

        ssize_t res = TEMP_FAILURE_RETRY(read(fildes, pos, end - pos));
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

        pos += res;
    }

    return pos - beg;
}

static ssize_t
chrdev_tx_op_read_exec(struct file_tx* base,
                       int fildes, void* buf, size_t nbyte,
                       bool isnoundo, int* cookie, struct picotm_error* error)
{
    if (!isnoundo) {
        picotm_error_set_revocable(error);
        return -1;
    }

    struct chrdev_tx* self = chrdev_tx_of_file_tx(base);

    chrdev_tx_try_wrlock_field(self, CHRDEV_FIELD_STATE, error);
    if (picotm_error_is_set(error)) {
        return (off_t)-1;
    }

    ssize_t res = do_read(fildes, buf, nbyte, error);
    if (picotm_error_is_set(error)) {
        return res;
    }
    return res;
}

/* write() */

static ssize_t
do_write(int fildes, const void* buf, size_t nbyte,
         struct picotm_error* error)
{
    const uint8_t* pos = buf;
    const uint8_t* beg = pos;
    const uint8_t* end = beg + nbyte;

    while (pos < end) {

        ssize_t res = TEMP_FAILURE_RETRY(write(fildes, pos, end - pos));
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

        pos += res;
    }

    return pos - beg;
}

static ssize_t
chrdev_tx_op_write_exec(struct file_tx* base,
                        int fildes, const void* buf, size_t nbyte,
                        bool isnoundo, int* cookie, struct picotm_error* error)
{
    struct chrdev_tx* self = chrdev_tx_of_file_tx(base);
    struct seekbuf_tx* seekbuf_tx = self->seekbuf_tx;

    if (isnoundo) {
        seekbuf_tx->wrmode = PICOTM_LIBC_WRITE_THROUGH;
    } else if (seekbuf_tx->wrmode == PICOTM_LIBC_WRITE_THROUGH) {
        picotm_error_set_revocable(error);
        return -1;
    }

    /* Write-lock character device, because we change the file position */
    /* TODO: wrlock write */
    chrdev_tx_try_wrlock_field(self, CHRDEV_FIELD_STATE, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    ssize_t res;

    if (seekbuf_tx->wrmode == PICOTM_LIBC_WRITE_THROUGH) {
        res = do_write(fildes, buf, nbyte, error);
        if (picotm_error_is_set(error)) {
            return res;
        }
    } else {

        /* Register write data */
        *cookie = seekbuf_tx_append_to_writeset(seekbuf_tx, nbyte, 0, buf,
                                                error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
        res = nbyte;
    }

    return res;
}

static void
chrdev_tx_op_write_apply(struct file_tx* base, int fildes, int cookie,
                         struct picotm_error* error)
{
    struct chrdev_tx* self = chrdev_tx_of_file_tx(base);
    struct seekbuf_tx* seekbuf_tx = self->seekbuf_tx;

    if (seekbuf_tx->wrmode == PICOTM_LIBC_WRITE_BACK) {
        do_write(fildes,
                 seekbuf_tx->wrbuf + seekbuf_tx->wrtab[cookie].bufoff,
                 seekbuf_tx->wrtab[cookie].nbyte, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
}

static const struct file_tx_ops chrdev_tx_ops = {
    PICOTM_LIBC_FILE_TYPE_CHRDEV,
    /* file handling */
    .prepare = chrdev_tx_op_prepare,
    .release = chrdev_tx_op_release,
    /* module interfaces */
    .finish = chrdev_tx_op_finish,
    /* file ops */
    .accept_exec = file_tx_op_accept_exec_enotsock,
    .bind_exec = file_tx_op_bind_exec_enotsock,
    .connect_exec = file_tx_op_connect_exec_enotsock,
    .fchmod_exec = file_tx_op_fchmod_exec_einval,
    .fcntl_exec  = chrdev_tx_op_fcntl_exec,
    .fcntl_apply = file_tx_op_apply,
    .fcntl_undo  = file_tx_op_undo,
    .fstat_exec  = chrdev_tx_op_fstat_exec,
    .fstat_apply = file_tx_op_apply,
    .fstat_undo  = file_tx_op_undo,
    .fsync_exec  = file_tx_op_fsync_exec_einval,
    .listen_exec = file_tx_op_listen_exec_enotsock,
    .lseek_exec = file_tx_op_lseek_exec_espipe,
    .pread_exec = file_tx_op_pread_exec_espipe,
    .pwrite_exec = file_tx_op_pwrite_exec_espipe,
    .read_exec  = chrdev_tx_op_read_exec,
    .read_apply = file_tx_op_apply,
    .read_undo  = file_tx_op_undo,
    .recv_exec = file_tx_op_recv_exec_enotsock,
    .send_exec = file_tx_op_send_exec_enotsock,
    .shutdown_exec = file_tx_op_shutdown_exec_enotsock,
    .write_exec  = chrdev_tx_op_write_exec,
    .write_apply = chrdev_tx_op_write_apply,
    .write_undo  = file_tx_op_undo,
};

/*
 * Public interface
 */

static void
init_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end)
{
    while (beg < end) {
        picotm_rwstate_init(beg);
        ++beg;
    }
}

void
chrdev_tx_init(struct chrdev_tx* self)
{
    assert(self);

    file_tx_init(&self->base, &chrdev_tx_ops);

    self->seekbuf_tx = nullptr;

    self->fcntltab = nullptr;
    self->fcntltablen = 0;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));
}

static void
uninit_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end)
{
    while (beg < end) {
        picotm_rwstate_uninit(beg);
        ++beg;
    }
}

void
chrdev_tx_uninit(struct chrdev_tx* self)
{
    assert(self);

    fcntloptab_clear(&self->fcntltab, &self->fcntltablen);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));
}
