/* Permission is hereby granted, free of charge, to any person obtaining a
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
 */

#include "dir_tx.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-ptr.h>
#include <picotm/picotm-lib-tab.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "fchmodop.h"
#include "fchmodoptab.h"
#include "fcntlop.h"
#include "fcntloptab.h"

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
ref_file_tx(struct file_tx* file_tx)
{
    dir_tx_ref(dir_tx_of_file_tx(file_tx));
}

static void
unref_file_tx(struct file_tx* file_tx)
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
 * accept()
 */

static int
accept_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
            struct sockaddr* address, socklen_t* address_len, bool isnoundo,
            int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

/*
 * bind()
 */

static int
bind_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
          const struct sockaddr* address, socklen_t addresslen, bool isnoundo,
          int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

/*
 * connect()
 */

static int
connect_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
             const struct sockaddr* address, socklen_t addresslen,
             bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

/*
 * fchmod()
 */

static int
fchmod_exec_noundo(struct dir_tx* self, int fildes, mode_t mode,
                   int* cookie, struct picotm_error* error)
{
    int res = fchmod(fildes, mode);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

static int
fchmod_exec_2pl(struct dir_tx* self, int fildes, mode_t mode,
                int* cookie, struct picotm_error* error)
{
    assert(self);
    assert(cookie);

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

static int
dir_tx_fchmod_exec(struct dir_tx* self, int fildes, mode_t mode,
                      bool isnoundo, int* cookie, struct picotm_error* error)
{
    static int (* const fchmod_exec[2])(struct dir_tx*,
                                        int,
                                        mode_t,
                                        int*,
                                        struct picotm_error*) = {
        fchmod_exec_noundo,
        fchmod_exec_2pl
    };

    if (isnoundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !fchmod_exec[self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return fchmod_exec[self->cc_mode](self, fildes, mode, cookie, error);
}

static int
fchmod_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            mode_t mode, bool isnoundo, int* cookie,
            struct picotm_error* error)
{
    return dir_tx_fchmod_exec(dir_tx_of_file_tx(base), fildes, mode,
                              isnoundo, cookie, error);
}

static void
fchmod_apply_noundo(int fildes, struct picotm_error* error)
{ }

static void
fchmod_apply_2pl(int fildes, struct picotm_error* error)
{ }

static void
dir_tx_fchmod_apply(struct dir_tx* self, int fildes, int cookie,
                    struct picotm_error* error)
{
    static void (* const fchmod_apply[2])(int, struct picotm_error*) = {
        fchmod_apply_noundo,
        fchmod_apply_2pl
    };

    assert(fchmod_apply[self->cc_mode]);

    fchmod_apply[self->cc_mode](fildes, error);
}

static void
fchmod_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
             int cookie, struct picotm_error* error)
{
    dir_tx_fchmod_apply(dir_tx_of_file_tx(base), fildes, cookie, error);
}

static void
fchmod_undo_2pl(struct dir_tx* self, int fildes, int cookie,
                struct picotm_error* error)
{
    const struct fchmodop* op = self->fchmodtab + cookie;

    int res = fchmod(fildes, op->old_mode);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

static void
dir_tx_fchmod_undo(struct dir_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{
    static void (* const fchmod_undo[2])(struct dir_tx*,
                                         int,
                                         int,
                                         struct picotm_error*) = {
        NULL,
        fchmod_undo_2pl
    };

    assert(fchmod_undo[self->cc_mode]);

    fchmod_undo[self->cc_mode](self, fildes, cookie, error);
}

static void
fchmod_undo(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            int cookie, struct picotm_error* error)
{
    dir_tx_fchmod_undo(dir_tx_of_file_tx(base), fildes, cookie, error);
}

/*
 * fcntl()
 */

static int
fcntl_exec_noundo(struct dir_tx* self, int fildes, int cmd,
                  union fcntl_arg* arg, int* cookie,
                  struct picotm_error* error)
{
    int res = 0;

    assert(arg);

    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN:
            res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd));
            arg->arg0 = res;
            break;
        case F_GETLK:
            res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg1));
            break;
        case F_SETFL:
        case F_SETFD:
        case F_SETOWN:
            res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg0));
            break;
        case F_SETLK:
        case F_SETLKW:
            res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg1));
            break;
        default:
            errno = EINVAL;
            res = -1;
            break;
    }

    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }

    return res;
}

static int
fcntl_exec_2pl(struct dir_tx* self, int fildes, int cmd,
               union fcntl_arg* arg, int* cookie, struct picotm_error* error)
{
    assert(arg);

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
        case F_SETOWN:
        case F_SETLK:
        case F_SETLKW:
            picotm_error_set_revocable(error);
            return -1;
        default:
            break;
    }

    picotm_error_set_errno(error, EINVAL);
    return -1;
}

static int
dir_tx_fcntl_exec(struct dir_tx* self, int fildes, int cmd,
                  union fcntl_arg* arg, bool isnoundo, int* cookie,
                  struct picotm_error* error)
{
    static int (* const fcntl_exec[2])(struct dir_tx*,
                                       int,
                                       int,
                                       union fcntl_arg*,
                                       int*,
                                       struct picotm_error*) = {
        fcntl_exec_noundo,
        fcntl_exec_2pl
    };

    if (isnoundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !fcntl_exec[self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return fcntl_exec[self->cc_mode](self, fildes, cmd, arg, cookie, error);
}

static int
fcntl_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes, int cmd,
           union fcntl_arg* arg, bool isnoundo, int* cookie,
           struct picotm_error* error)
{
    return dir_tx_fcntl_exec(dir_tx_of_file_tx(base), fildes, cmd, arg,
                             isnoundo, cookie, error);
}

static void
dir_tx_fcntl_apply(struct dir_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{ }

static void
fcntl_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            int cookie, struct picotm_error* error)
{
    dir_tx_fcntl_apply(dir_tx_of_file_tx(base), fildes, cookie, error);
}

static void
dir_tx_fcntl_undo(struct dir_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{ }

static void
fcntl_undo(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           int cookie, struct picotm_error* error)
{
    dir_tx_fcntl_undo(dir_tx_of_file_tx(base), fildes, cookie, error);
}

/*
 * fstat()
 */

static int
fstat_exec_noundo(struct dir_tx* self, int fildes, struct stat* buf,
                  int* cookie, struct picotm_error* error)
{
    int res = fstat(fildes, buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

static int
fstat_exec_2pl(struct dir_tx* self, int fildes, struct stat* buf,
               int* cookie, struct picotm_error* error)
{
    assert(self);
    assert(buf);

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

static int
dir_tx_fstat_exec(struct dir_tx* self, int fildes, struct stat* buf,
                  bool isnoundo, int* cookie, struct picotm_error* error)
{
    static int (* const fstat_exec[2])(struct dir_tx*,
                                       int,
                                       struct stat*,
                                       int*,
                                       struct picotm_error*) = {
        fstat_exec_noundo,
        fstat_exec_2pl
    };

    if (isnoundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !fstat_exec[self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return fstat_exec[self->cc_mode](self, fildes, buf, cookie, error);
}

static int
fstat_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           struct stat* buf, bool isnoundo, int* cookie,
           struct picotm_error* error)
{
    return dir_tx_fstat_exec(dir_tx_of_file_tx(base), fildes, buf, isnoundo,
                             cookie, error);
}

static void
fstat_apply_noundo(struct dir_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{ }

static void
fstat_apply_2pl(struct dir_tx* self, int fildes, int cookie,
                struct picotm_error* error)
{ }

static void
dir_tx_fstat_apply(struct dir_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{
    static void (* const fstat_apply[2])(struct dir_tx*,
                                         int,
                                         int,
                                         struct picotm_error*) = {
        fstat_apply_noundo,
        fstat_apply_2pl
    };

    assert(fstat_apply[self->cc_mode]);

    fstat_apply[self->cc_mode](self, fildes, cookie, error);
}

static void
fstat_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            int cookie, struct picotm_error* error)
{
    dir_tx_fstat_apply(dir_tx_of_file_tx(base), fildes, cookie, error);
}

static void
fstat_undo_2pl(struct dir_tx* self, int fildes, int cookie,
               struct picotm_error* error)
{ }

static void
dir_tx_fstat_undo(struct dir_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static void (* const fstat_undo[2])(struct dir_tx*,
                                        int,
                                        int,
                                        struct picotm_error*) = {
        NULL,
        fstat_undo_2pl
    };

    assert(fstat_undo[self->cc_mode]);

    fstat_undo[self->cc_mode](self, fildes, cookie, error);
}

static void
fstat_undo(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           int cookie, struct picotm_error* error)
{
    dir_tx_fstat_undo(dir_tx_of_file_tx(base), fildes, cookie, error);
}

/*
 * fsync()
 */

static int
fsync_exec_noundo(int fildes, int* cookie, struct picotm_error* error)
{
    int res = fsync(fildes);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

static int
fsync_exec_2pl(int fildes, int* cookie, struct picotm_error* error)
{
    /* Signal apply/undo */
    *cookie = 0;

    return 0;
}

static int
dir_tx_fsync_exec(struct dir_tx* self, int fildes, bool isnoundo, int* cookie,
                  struct picotm_error* error)
{
    static int (* const fsync_exec[2])(int, int*, struct picotm_error*) = {
        fsync_exec_noundo,
        fsync_exec_2pl
    };

    if (isnoundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !fsync_exec[self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return fsync_exec[self->cc_mode](fildes, cookie, error);
}

static int
fsync_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           bool isnoundo, int* cookie, struct picotm_error* error)
{
    return dir_tx_fsync_exec(dir_tx_of_file_tx(base), fildes, isnoundo,
                             cookie, error);
}

static void
fsync_apply_noundo(int fildes, struct picotm_error* error)
{ }

static void
fsync_apply_2pl(int fildes, struct picotm_error* error)
{
    int res = fsync(fildes);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

static void
dir_tx_fsync_apply(struct dir_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{
    static void (* const fsync_apply[2])(int, struct picotm_error*) = {
        fsync_apply_noundo,
        fsync_apply_2pl
    };

    assert(fsync_apply[self->cc_mode]);

    fsync_apply[self->cc_mode](fildes, error);
}

static void
fsync_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            int cookie, struct picotm_error* error)
{
    dir_tx_fsync_apply(dir_tx_of_file_tx(base), fildes, cookie, error);
}

static void
fsync_undo_2pl(int fildes, int cookie, struct picotm_error* error)
{ }

static void
dir_tx_fsync_undo(struct dir_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static void (* const fsync_undo[2])(int, int, struct picotm_error*) = {
        NULL,
        fsync_undo_2pl
    };

    assert(fsync_undo[self->cc_mode]);

    fsync_undo[self->cc_mode](fildes, cookie, error);
}

static void
fsync_undo(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           int cookie, struct picotm_error* error)
{
    dir_tx_fsync_undo(dir_tx_of_file_tx(base), fildes, cookie, error);
}

/*
 * listen()
 */

static int
listen_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
            int backlog, bool isnoundo, int* cookie,
            struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

/*
 * lseek()
 */

static off_t
lseek_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           off_t offset, int whence, bool isnoundo, int* cookie,
           struct picotm_error* error)
{
    picotm_error_set_errno(error, EINVAL);
    return (off_t)-1;
}

/*
 * pread()
 */

static ssize_t
pread_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes, void* buf,
           size_t nbyte, off_t off, bool isnoundo, int* cookie,
           struct picotm_error* error)
{
    picotm_error_set_errno(error, EISDIR);
    return -1;
}

/*
 * pwrite()
 */

static ssize_t
pwrite_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            const void* buf, size_t nbyte, off_t off, bool isnoundo,
            int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, EISDIR);
    return -1;
}

/*
 * read()
 */

static ssize_t
read_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes, void* buf,
          size_t nbyte, bool isnoundo, int* cookie,
          struct picotm_error* error)
{
    picotm_error_set_errno(error, EISDIR);
    return -1;
}

/*
 * recv()
 */

static ssize_t
recv_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
          void* buffer, size_t length, int flags, bool isnoundo, int* cookie,
          struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

/*
 * send()
 */

static ssize_t
send_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
          const void* buffer, size_t length, int flags, bool isnoundo,
          int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

/*
 * shutdown()
 */

static int
shutdown_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
              int how, bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

/*
 * write()
 */

static ssize_t
write_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           const void* buf, size_t nbyte, bool isnoundo, int* cookie,
           struct picotm_error* error)
{
    picotm_error_set_errno(error, EISDIR);
    return -1;
}

/*
 * Module interface
 */

static void
lock_file_tx(struct file_tx* base, struct picotm_error* error)
{ }

static void
unlock_file_tx(struct file_tx* base, struct picotm_error* error)
{ }

/* Validation
 */

static void
validate_noundo(struct dir_tx* self, struct picotm_error* error)
{ }

static void
validate_2pl(struct dir_tx* self, struct picotm_error* error)
{
    assert(self);
}

static void
dir_tx_validate(struct dir_tx* self, struct picotm_error* error)
{
    static void (* const validate[])(struct dir_tx*, struct picotm_error*) = {
        validate_noundo,
        validate_2pl
    };

    if (!dir_tx_holds_ref(self)) {
        return;
    }

    validate[self->cc_mode](self, error);
}

static void
validate_file_tx(struct file_tx* base, struct picotm_error* error)
{
    dir_tx_validate(dir_tx_of_file_tx(base), error);
}

/* Update CC
 */

static void
update_cc_noundo(struct dir_tx* self, struct picotm_error* error)
{ }

static void
update_cc_2pl(struct dir_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_2PL);

    /* release reader/writer locks on directory state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->dir);
}

static void
dir_tx_update_cc(struct dir_tx* self, struct picotm_error* error)
{
    static void (* const update_cc[])(struct dir_tx*, struct picotm_error*) = {
        update_cc_noundo,
        update_cc_2pl
    };

    assert(dir_tx_holds_ref(self));

    update_cc[self->cc_mode](self, error);
}

static void
update_cc_file_tx(struct file_tx* base, struct picotm_error* error)
{
    dir_tx_update_cc(dir_tx_of_file_tx(base), error);
}

/* Clear CC
 */

static void
clear_cc_noundo(struct dir_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO);
}

static void
clear_cc_2pl(struct dir_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_2PL);

    /* release lock on directory */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->dir);
}

static void
dir_tx_clear_cc(struct dir_tx* self, struct picotm_error* error)
{
    static void (* const clear_cc[])(struct dir_tx*, struct picotm_error*) = {
        clear_cc_noundo,
        clear_cc_2pl
    };

    assert(dir_tx_holds_ref(self));

    clear_cc[self->cc_mode](self, error);
}

static void
clear_cc_file_tx(struct file_tx* base, struct picotm_error* error)
{
    dir_tx_clear_cc(dir_tx_of_file_tx(base), error);
}

/*
 * Public interface
 */

static const struct file_tx_ops dir_tx_ops = {
    PICOTM_LIBC_FILE_TYPE_DIR,
    /* ref counting */
    ref_file_tx,
    unref_file_tx,
    /* module interfaces */
    lock_file_tx,
    unlock_file_tx,
    validate_file_tx,
    update_cc_file_tx,
    clear_cc_file_tx,
    /* file ops */
    accept_exec,
    NULL,
    NULL,
    bind_exec,
    NULL,
    NULL,
    connect_exec,
    NULL,
    NULL,
    fchmod_exec,
    fchmod_apply,
    fchmod_undo,
    fcntl_exec,
    fcntl_apply,
    fcntl_undo,
    fstat_exec,
    fstat_apply,
    fstat_undo,
    fsync_exec,
    fsync_apply,
    fsync_undo,
    listen_exec,
    NULL,
    NULL,
    lseek_exec,
    NULL,
    NULL,
    pread_exec,
    NULL,
    NULL,
    pwrite_exec,
    NULL,
    NULL,
    read_exec,
    NULL,
    NULL,
    recv_exec,
    NULL,
    NULL,
    send_exec,
    NULL,
    NULL,
    shutdown_exec,
    NULL,
    NULL,
    write_exec,
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

    self->fchmodtab = NULL;
    self->fchmodtablen = 0;

    self->fcntltab = NULL;
    self->fcntltablen = 0;

    self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;

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
    dir_ref(dir);

    /* setup fields */

    self->dir = dir;
    self->cc_mode = dir_get_cc_mode(dir);

    self->fchmodtablen = 0;
    self->fcntltablen = 0;
}

void
dir_tx_ref(struct dir_tx* self)
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
