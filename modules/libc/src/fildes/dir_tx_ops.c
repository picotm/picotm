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

#include "dir_tx_ops.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-tab.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "compat/temp_failure_retry.h"
#include "dir_tx.h"
#include "fchmodop.h"
#include "fchmodoptab.h"
#include "fcntlop.h"
#include "fcntloptab.h"
#include "file_tx.h"

/*
 * File handling
 */

static void
prepare(struct file_tx* file_tx, struct file* file, void* data,
        struct picotm_error* error)
{
    dir_tx_prepare(dir_tx_of_file_tx(file_tx), dir_of_base(file), error);
}

static void
release(struct file_tx* file_tx)
{
    dir_tx_release(dir_tx_of_file_tx(file_tx));
}

/*
 * Module interface
 */

static void
finish(struct file_tx* base)
{
    dir_tx_finish(dir_tx_of_file_tx(base));
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

    struct dir_tx* self = dir_tx_of_file_tx(base);

    /* Acquire file-mode lock. */
    dir_tx_try_wrlock_field(self, DIR_FIELD_FILE_MODE, error);
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
    struct dir_tx* self = dir_tx_of_file_tx(base);

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
    struct dir_tx* self = dir_tx_of_file_tx(base);

    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN: {

            /* Read-lock directory */
            dir_tx_try_rdlock_field(self, DIR_FIELD_STATE, error);
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

            /* Read-lock directory */
            dir_tx_try_rdlock_field(self, DIR_FIELD_STATE, error);
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
    struct dir_tx* self = dir_tx_of_file_tx(base);

    /* Acquire file-mode reader lock. */
    dir_tx_try_rdlock_field(self, DIR_FIELD_FILE_MODE, error);
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

/*
 * fsync()
 */

static int
fsync_exec(struct file_tx* base,
           int fildes,
           bool isnoundo, int* cookie, struct picotm_error* error)
{
    assert(cookie);

    struct dir_tx* self = dir_tx_of_file_tx(base);

    *cookie = 0; /* apply fsync() during commit */

    if (self->wrmode == PICOTM_LIBC_WRITE_BACK) {
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
fsync_apply(struct file_tx* base,
            int fildes, int cookie, struct picotm_error* error)
{
    int res = fsync(fildes);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

/*
 * Public interface
 */

const struct file_tx_ops dir_tx_ops = {
    PICOTM_LIBC_FILE_TYPE_DIR,
    /* file handling */
    prepare,
    release,
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
    file_tx_op_lseek_exec_einval,
    NULL,
    NULL,
    file_tx_op_pread_exec_eisdir,
    NULL,
    NULL,
    file_tx_op_pwrite_exec_eisdir,
    NULL,
    NULL,
    file_tx_op_read_exec_eisdir,
    NULL,
    NULL,
    file_tx_op_recv_exec_enotsock,
    NULL,
    NULL,
    file_tx_op_send_exec_enotsock,
    NULL,
    NULL,
    file_tx_op_shutdown_exec_enotsock,
    NULL,
    NULL,
    file_tx_op_write_exec_eisdir,
    NULL,
    NULL
};
