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

#include "dir_tx.h"

#include "dir_tx.h"
#include "fchmodop.h"
#include "fchmodoptab.h"
#include "fcntlop.h"
#include "fcntloptab.h"
#include "file_tx.h"
#include "file_tx_ops.h"

#include "compat/temp_failure_retry.h"
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

static void
dir_tx_try_rdlock_field(struct dir_tx* self, enum dir_field field,
                        struct picotm_error* error)
{
    assert(self);

    dir_try_rdlock_field(dir_of_base(self->base.file), field,
                         self->rwstate + field, error);
}

static void
dir_tx_try_wrlock_field(struct dir_tx* self, enum dir_field field,
                        struct picotm_error* error)
{
    assert(self);

    dir_try_wrlock_field(dir_of_base(self->base.file), field,
                         self->rwstate + field, error);
}

/*
 * struct file_tx_ops
 */

static void
dir_tx_op_prepare(struct file_tx* file_tx, struct file* file, void* data,
                  struct picotm_error* error)
{
    struct dir_tx* self = dir_tx_of_file_tx(file_tx);

    /* Regular files use write-back mode, but a directory might contain
     * files in write-through mode. to ensure their fsyncs reach disk, we
     * set the directory to write-through mode by default. Fsyncs will be
     * performed during execution and apply phases. */
    self->wrmode = PICOTM_LIBC_WRITE_THROUGH;

    self->fchmodtablen = 0;
    self->fcntltablen = 0;
}

static void
dir_tx_op_release(struct file_tx* file_tx)
{ }

static void
unlock_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end,
                struct dir* dir)
{
    enum dir_field field = 0;

    while (beg < end) {
        dir_unlock_field(dir, field, beg);
        ++field;
        ++beg;
    }
}

static void
dir_tx_op_finish(struct file_tx* base)
{
    struct dir_tx* self = dir_tx_of_file_tx(base);

    /* release reader/writer locks on directory state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    dir_of_base(self->base.file));
}

/* fchmod() */

static int
dir_tx_op_fchmod_exec(struct file_tx* base,
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
dir_tx_op_fchmod_undo(struct file_tx* base, int fildes, int cookie,
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

/* fcntl() */

static int
dir_tx_op_fcntl_exec(struct file_tx* base,
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

/* fstat() */

static int
dir_tx_op_fstat_exec(struct file_tx* base,
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

/* fsync() */

static int
dir_tx_op_fsync_exec(struct file_tx* base,
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
dir_tx_op_fsync_apply(struct file_tx* base,
                      int fildes, int cookie, struct picotm_error* error)
{
    int res = fsync(fildes);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

static const struct file_tx_ops dir_tx_ops = {
    PICOTM_LIBC_FILE_TYPE_DIR,
    /* file handling */
    .prepare = dir_tx_op_prepare,
    .release = dir_tx_op_release,
    /* module interfaces */
    .finish = dir_tx_op_finish,
    /* file ops */
    .accept_exec = file_tx_op_accept_exec_enotsock,
    .bind_exec = file_tx_op_bind_exec_enotsock,
    .connect_exec = file_tx_op_connect_exec_enotsock,
    .fchmod_exec  = dir_tx_op_fchmod_exec,
    .fchmod_apply = file_tx_op_apply,
    .fchmod_undo  = dir_tx_op_fchmod_undo,
    .fcntl_exec  = dir_tx_op_fcntl_exec,
    .fcntl_apply = file_tx_op_apply,
    .fcntl_undo  = file_tx_op_undo,
    .fstat_exec  = dir_tx_op_fstat_exec,
    .fstat_apply = file_tx_op_apply,
    .fstat_undo  = file_tx_op_undo,
    .fsync_exec  = dir_tx_op_fsync_exec,
    .fsync_apply = dir_tx_op_fsync_apply,
    .fsync_undo  = file_tx_op_undo,
    .listen_exec = file_tx_op_listen_exec_enotsock,
    .lseek_exec = file_tx_op_lseek_exec_einval,
    .pread_exec = file_tx_op_pread_exec_eisdir,
    .pwrite_exec = file_tx_op_pwrite_exec_eisdir,
    .read_exec = file_tx_op_read_exec_eisdir,
    .recv_exec = file_tx_op_recv_exec_enotsock,
    .send_exec = file_tx_op_send_exec_enotsock,
    .shutdown_exec = file_tx_op_shutdown_exec_enotsock,
    .write_exec = file_tx_op_write_exec_eisdir,
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
dir_tx_init(struct dir_tx* self)
{
    assert(self);

    file_tx_init(&self->base, &dir_tx_ops);

    self->wrmode = PICOTM_LIBC_WRITE_THROUGH;

    self->fchmodtab = nullptr;
    self->fchmodtablen = 0;

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
dir_tx_uninit(struct dir_tx* self)
{
    assert(self);

    fcntloptab_clear(&self->fcntltab, &self->fcntltablen);
    fchmodoptab_clear(&self->fchmodtab, &self->fchmodtablen);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));
}
