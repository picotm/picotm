/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 */

#include "dir_tx.h"
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
#include "fchmodop.h"
#include "fchmodoptab.h"
#include "fcntlop.h"
#include "fcntloptab.h"
#include "file_tx_ops.h"

static struct dir_tx*
dir_tx_of_file_tx(struct file_tx* file_tx)
{
    assert(file_tx);
    assert(file_tx_file_type(file_tx) == PICOTM_LIBC_FILE_TYPE_DIR);

    return picotm_containerof(file_tx, struct dir_tx, base);
}

static void
dir_tx_try_rdlock_field(struct dir_tx* self, enum dir_field field,
                        struct picotm_error* error)
{
    assert(self);

    dir_try_rdlock_field(self->dir, field, self->rwstate + field, error);
}

static void
dir_tx_try_wrlock_field(struct dir_tx* self, enum dir_field field,
                        struct picotm_error* error)
{
    assert(self);

    dir_try_wrlock_field(self->dir, field, self->rwstate + field, error);
}

static void
ref(struct file_tx* file_tx, struct picotm_error* error)
{
    dir_tx_ref(dir_tx_of_file_tx(file_tx), error);
}

static void
unref(struct file_tx* file_tx)
{
    dir_tx_unref(dir_tx_of_file_tx(file_tx));
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
                struct dir* dir)
{
    enum dir_field field = 0;

    while (beg < end) {
        dir_unlock_field(dir, field, beg);
        ++field;
        ++beg;
    }
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
fchmod_undo(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            int cookie, struct picotm_error* error)
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
fcntl_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes, int cmd,
           union fcntl_arg* arg, bool isnoundo, int* cookie,
           struct picotm_error* error)
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
fstat_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           struct stat* buf, bool isnoundo, int* cookie,
           struct picotm_error* error)
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
fsync_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
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
 * Module interface
 */

static void
finish(struct file_tx* base)
{
    struct dir_tx* self = dir_tx_of_file_tx(base);

    /* release reader/writer locks on directory state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->dir);
}

/*
 * Public interface
 */

static const struct file_tx_ops dir_tx_ops = {
    PICOTM_LIBC_FILE_TYPE_DIR,
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

void
dir_tx_init(struct dir_tx* self)
{
    assert(self);

    picotm_ref_init(&self->ref, 0);

    file_tx_init(&self->base, &dir_tx_ops);

    self->dir = NULL;

    self->wrmode = PICOTM_LIBC_WRITE_THROUGH;

    self->fchmodtab = NULL;
    self->fchmodtablen = 0;

    self->fcntltab = NULL;
    self->fcntltablen = 0;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));
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

/*
 * Referencing
 */

void
dir_tx_ref_or_set_up(struct dir_tx* self, struct dir* dir,
                     struct picotm_error* error)
{
    assert(self);
    assert(dir);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    /* acquire reference on directory */
    dir_ref(dir, error);
    if (picotm_error_is_set(error)) {
        goto err_dir_ref;
    }

    /* setup fields */

    self->dir = dir;

    /* Regular files use write-back mode, but a directory might contain
     * files in write-through mode. to ensure their fsyncs reach disk, we
     * set the directory to write-through mode by default. Fsyncs will be
     * performed during execution and apply phases. */
    self->wrmode = PICOTM_LIBC_WRITE_THROUGH;

    self->fchmodtablen = 0;
    self->fcntltablen = 0;

    return;

err_dir_ref:
    picotm_ref_down(&self->ref);
}

void
dir_tx_ref(struct dir_tx* self, struct picotm_error* error)
{
    picotm_ref_up(&self->ref);
}

void
dir_tx_unref(struct dir_tx* self)
{
    assert(self);

    bool final_ref = picotm_ref_down(&self->ref);
    if (!final_ref) {
        return;
    }

    dir_unref(self->dir);
    self->dir = NULL;
}

bool
dir_tx_holds_ref(struct dir_tx* self)
{
    assert(self);

    return picotm_ref_count(&self->ref) > 0;
}
