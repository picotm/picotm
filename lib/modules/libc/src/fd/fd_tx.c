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

#include "fd_tx.h"
#include <assert.h>
#include <errno.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "chrdev_tx.h"
#include "dir_tx.h"
#include "fcntlop.h"
#include "fcntloptab.h"
#include "fifo_tx.h"
#include "regfile_tx.h"
#include "socket_tx.h"

static void
fd_tx_try_rdlock_field(struct fd_tx* self, enum fd_field field,
                       struct picotm_error* error)
{
    assert(self);

    fd_try_rdlock_field(self->fd, field, self->rwstate + field, error);
}

static void
fd_tx_try_wrlock_field(struct fd_tx* self, enum fd_field field,
                       struct picotm_error* error)
{
    assert(self);

    fd_try_wrlock_field(self->fd, field, self->rwstate + field, error);
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
                struct fd* fd)
{
    enum fd_field field = 0;

    while (beg < end) {
        fd_unlock_field(fd, field, beg);
        ++field;
        ++beg;
    }
}

void
fd_tx_init(struct fd_tx* self)
{
    assert(self);

    picotm_ref_init(&self->ref, 0);

    memset(&self->active_list, 0, sizeof(self->active_list));

    self->fd = NULL;
    self->file_tx = NULL;
	self->cc_mode = PICOTM_LIBC_CC_MODE_2PL;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));

    self->fcntltab = NULL;
    self->fcntltablen = 0;
}

void
fd_tx_uninit(struct fd_tx* self)
{
    assert(self);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));
}

void
fd_tx_ref_or_set_up(struct fd_tx* self, struct fd* fd, struct file_tx* file_tx,
                    struct picotm_error* error)
{
    assert(self);
    assert(fd);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    fd_ref(fd, error);
    if (picotm_error_is_set(error)) {
        goto err_fd_ref;
    }

    file_tx_ref(file_tx);

    self->fd = fd;
    self->file_tx = file_tx;

    return;

err_fd_ref:
    picotm_ref_down(&self->ref);
}

void
fd_tx_ref(struct fd_tx* self)
{
    picotm_ref_up(&self->ref);
}

void
fd_tx_unref(struct fd_tx* self)
{
    assert(self);

    bool final_ref = picotm_ref_down(&self->ref);
    if (!final_ref) {
        return;
    }

    file_tx_unref(self->file_tx);
    fd_unref(self->fd);

    self->fd = NULL;
}

bool
fd_tx_holds_ref(const struct fd_tx* self)
{
    assert(self);

    return picotm_ref_count(&self->ref) > 0;
}

void
fd_tx_signal_close(struct fd_tx* self)
{
    assert(self);

    fd_close(self->fd);
}

void
fd_tx_dump(const struct fd_tx* self)
{
    fprintf(stderr, "%p: %p %p %zu\n", (void*)self,
                                       (void*)self->fd,
                                       (void*)self->fcntltab,
                                              self->fcntltablen);
}

/*
 * Module interface
 */

void
fd_tx_lock(struct fd_tx* self)
{ }

void
fd_tx_unlock(struct fd_tx* self)
{ }

void
fd_tx_validate(struct fd_tx* self, struct picotm_error* error)
{
    assert(self);

    if (!fd_tx_holds_ref(self)) {
        return;
    }

	/* file descriptor is still open; previously locked */
	if (!fd_is_open_nl(self->fd)) {
        picotm_error_set_conflicting(error, NULL);
		return;
	}
}

void
fd_tx_update_cc(struct fd_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(fd_tx_holds_ref(self));

    /* release reader/writer locks on file-descriptor state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->fd);
}

void
fd_tx_clear_cc(struct fd_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(fd_tx_holds_ref(self));

    /* release reader/writer locks on file-descriptor state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->fd);
}

/*
 * accept()
 */

int
fd_tx_accept_exec(struct fd_tx* self, int sockfd, struct sockaddr* address,
                  socklen_t* addresslen, bool isnoundo, int* cookie,
                  struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->accept_exec);

    return self->file_tx->ops->accept_exec(self->file_tx, sockfd, address,
                                           addresslen, isnoundo, cookie,
                                           error);
}

void
fd_tx_accept_apply(struct fd_tx* self, int sockfd, int cookie,
                   struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->accept_apply);

    self->file_tx->ops->accept_apply(self->file_tx, sockfd, cookie, error);
}

void
fd_tx_accept_undo(struct fd_tx* self, int sockfd, int cookie,
                  struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->accept_undo);

    self->file_tx->ops->accept_undo(self->file_tx, sockfd, cookie, error);
}

/*
 * bind()
 */

int
fd_tx_bind_exec(struct fd_tx* self, int sockfd, const struct sockaddr* address,
                socklen_t addresslen, bool isnoundo, int* cookie,
                struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->bind_exec);

    return self->file_tx->ops->bind_exec(self->file_tx, sockfd, address,
                                         addresslen, isnoundo, cookie, error);
}

void
fd_tx_bind_apply(struct fd_tx* self, int sockfd, int cookie,
                 struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->bind_apply);

    self->file_tx->ops->bind_apply(self->file_tx, sockfd, cookie, error);
}

void
fd_tx_bind_undo(struct fd_tx* self, int sockfd, int cookie,
                struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->bind_undo);

    self->file_tx->ops->bind_undo(self->file_tx, sockfd, cookie, error);
}

/*
 * close()
 */

static int
close_exec_noundo(struct fd_tx* self, int fildes, int* cookie,
                  struct picotm_error* error)
{
    /* acquire writer lock on file descriptor */
    fd_tx_try_wrlock_field(self, FD_FIELD_STATE, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    *cookie = 0; /* injects event */
    return 0;
}

static int
close_exec_ts(struct fd_tx* self, int fildes, int* cookie,
              struct picotm_error* error)
{
    /* acquire writer lock on file descriptor */
    fd_tx_try_wrlock_field(self, FD_FIELD_STATE, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    *cookie = 0; /* injects event */
    return 0;
}

int
fd_tx_close_exec(struct fd_tx* self, int fildes, bool isnoundo, int* cookie,
                 struct picotm_error* error)
{
    static int (* const close_exec[2])(struct fd_tx*,
                                       int,
                                       int*,
                                       struct picotm_error*) = {
        close_exec_noundo,
        close_exec_ts
    };

    assert(self->cc_mode < sizeof(close_exec)/sizeof(close_exec[0]));
    assert(close_exec[self->cc_mode]);

    return close_exec[self->cc_mode](self, fildes, cookie, error);
}

static void
close_apply_noundo(struct fd_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{
    fd_close(self->fd);
}

static void
close_apply_ts(struct fd_tx* self, int fildes, int cookie,
               struct picotm_error* error)
{
    fd_close(self->fd);
}

void
fd_tx_close_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static void (* const close_apply[2])(struct fd_tx*,
                                         int,
                                         int,
                                         struct picotm_error*) = {
        close_apply_noundo,
        close_apply_ts
    };

    assert(self->cc_mode < sizeof(close_apply)/sizeof(close_apply[0]));

    close_apply[self->cc_mode](self, fildes, cookie, error);
}

static void
close_undo_ts(struct fd_tx* self, int fildes, int cookie,
              struct picotm_error* error)
{ }

void
fd_tx_close_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    static void (* const close_undo[2])(struct fd_tx*,
                                        int,
                                        int,
                                        struct picotm_error*) = {
        NULL,
        close_undo_ts
    };

    assert(self->cc_mode < sizeof(close_undo)/sizeof(close_undo[0]));
    assert(close_undo[self->cc_mode]);

    close_undo[self->cc_mode](self, fildes, cookie, error);
}

/*
 * connect()
 */

int
fd_tx_connect_exec(struct fd_tx* self, int sockfd,
                   const struct sockaddr* address, socklen_t addresslen,
                   bool isnoundo, int* cookie, struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->connect_exec);

    return self->file_tx->ops->connect_exec(self->file_tx,
                                            sockfd, address, addresslen,
                                            isnoundo, cookie, error);
}

void
fd_tx_connect_apply(struct fd_tx* self, int sockfd, int cookie,
                    struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->connect_apply);

    self->file_tx->ops->connect_apply(self->file_tx, sockfd, cookie, error);
}

void
fd_tx_connect_undo(struct fd_tx* self, int sockfd, int cookie,
                   struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->connect_undo);

    self->file_tx->ops->connect_undo(self->file_tx, sockfd, cookie, error);
}

/*
 * dup()
 */

int
fd_tx_dup_exec(struct fd_tx* self, int fildes, bool cloexec,
               bool isnoundo, int* cookie, struct picotm_error* error)
{
    static const int fcntl_cmd[] = {
        F_DUPFD,
        F_DUPFD_CLOEXEC
    };

    static const int start_fildes = 0;

    int res = TEMP_FAILURE_RETRY(fcntl(fildes, fcntl_cmd[cloexec],
                                       start_fildes));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }
    return res;
}

void
fd_tx_dup_apply(struct fd_tx* self, int fildes, int cookie,
                struct picotm_error* error)
{ }

void
fd_tx_dup_undo(struct fd_tx* self, int fildes, int cookie,
               struct picotm_error* error)
{ }

/*
 * fchmod()
 */

int
fd_tx_fchmod_exec(struct fd_tx* self, int fildes, mode_t mode,
                  bool isnoundo, int* cookie, struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->fchmod_exec);

    return self->file_tx->ops->fchmod_exec(self->file_tx, fildes, mode,
                                           isnoundo, cookie, error);
}

void
fd_tx_fchmod_apply(struct fd_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->fchmod_apply);

    self->file_tx->ops->fchmod_apply(self->file_tx, fildes, cookie, error);
}

void
fd_tx_fchmod_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->fchmod_undo);

    self->file_tx->ops->fchmod_undo(self->file_tx, fildes, cookie, error);
}

/*
 * fcntl()
 */

int
fd_tx_fcntl_exec(struct fd_tx* self, int fildes, int cmd,
                 union fcntl_arg* arg, bool isnoundo, int* cookie,
                 struct picotm_error* error)
{
    /* acquire reader lock on file descriptor */
    fd_tx_try_rdlock_field(self, FD_FIELD_STATE, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    union fcntl_arg oldvalue;

    int res = -1;

    switch (cmd) {
        case F_SETFD:
            if ( !isnoundo ) {
                picotm_error_set_revocable(error);
                break;
            }
            res = fd_setfd(self->fd, arg->arg0, error);
            if (picotm_error_is_set(error)) {
                break;
            }
            break;
        case F_GETFD:
            res = fd_getfd(self->fd, error);
            if (picotm_error_is_set(error)) {
                break;
            }
            arg->arg0 = res;
            break;
        default:
            assert(self);
            assert(self->file_tx);
            assert(self->file_tx->ops);
            assert(self->file_tx->ops->fcntl_exec);
            return self->file_tx->ops->fcntl_exec(self->file_tx, fildes, cmd,
                                                  arg, isnoundo, cookie,
                                                  error);
    }

    if (picotm_error_is_set(error)) {
        goto err_cmd;
    }

    /* register fcntl */

    if (cookie) {
        *cookie = fcntloptab_append(&self->fcntltab,
                                    &self->fcntltablen, cmd, arg, &oldvalue,
                                    error);
        if (picotm_error_is_set(error)) {
            abort();
        }
    }

    return res;

err_cmd:
    return -1;
}

void
fd_tx_fcntl_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    assert(self);
    assert(self->fd);
    assert(cookie < (ssize_t)self->fcntltablen);

    switch (self->fcntltab[cookie].command) {
        case F_SETFD: {
            fd_setfd(self->fd, self->fcntltab[cookie].value.arg0, error);
            if (picotm_error_is_set(error)) {
                return;
            }
            break;
        }
        case F_GETFD:
            break;
        default:
            assert(self);
            assert(self->file_tx);
            assert(self->file_tx->ops);
            assert(self->file_tx->ops->fcntl_apply);
            self->file_tx->ops->fcntl_apply(self->file_tx, fildes, cookie,
                                            error);
            if (picotm_error_is_set(error)) {
                return;
            }
            break;
    }
}

void
fd_tx_fcntl_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    assert(self);
    assert(self->fd);
    assert(cookie < (ssize_t)self->fcntltablen);

    switch (self->fcntltab[cookie].command) {
        case F_SETFD: {
            fd_setfd(self->fd, self->fcntltab[cookie].oldvalue.arg0, error);
            if (picotm_error_is_set(error)) {
                return;
            }
            break;
        }
        case F_GETFD:
            break;
        default:
            assert(self);
            assert(self->file_tx);
            assert(self->file_tx->ops);
            assert(self->file_tx->ops->fcntl_undo);
            self->file_tx->ops->fcntl_undo(self->file_tx, fildes, cookie,
                                           error);
            if (picotm_error_is_set(error)) {
                return;
            }
            break;
    }
}

/*
 * fstat()
 */

int
fd_tx_fstat_exec(struct fd_tx* self, int fildes, struct stat* buf,
                 bool isnoundo, int* cookie, struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->fstat_exec);

    return self->file_tx->ops->fstat_exec(self->file_tx, fildes, buf,
                                          isnoundo, cookie, error);
}

void
fd_tx_fstat_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->fstat_apply);

    self->file_tx->ops->fstat_apply(self->file_tx, fildes, cookie, error);
}

void
fd_tx_fstat_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->fstat_undo);

    self->file_tx->ops->fstat_undo(self->file_tx, fildes, cookie, error);
}

/*
 * fsync()
 */

int
fd_tx_fsync_exec(struct fd_tx* self, int fildes, bool isnoundo, int* cookie,
                 struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->fsync_exec);

    return self->file_tx->ops->fsync_exec(self->file_tx, fildes, isnoundo,
                                          cookie, error);
}

void
fd_tx_fsync_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->fsync_apply);

    self->file_tx->ops->fsync_apply(self->file_tx, fildes, cookie, error);
}

void
fd_tx_fsync_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->fsync_undo);

    self->file_tx->ops->fsync_undo(self->file_tx, fildes, cookie, error);
}

/*
 * listen()
 */

int
fd_tx_listen_exec(struct fd_tx* self, int sockfd, int backlog, bool isnoundo,
                  int* cookie, struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->listen_exec);

    return self->file_tx->ops->listen_exec(self->file_tx, sockfd, backlog, isnoundo,
                                           cookie, error);
}

void
fd_tx_listen_apply(struct fd_tx* self, int sockfd, int cookie,
                   struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->listen_apply);

    self->file_tx->ops->listen_apply(self->file_tx, sockfd, cookie, error);
}

void
fd_tx_listen_undo(struct fd_tx* self, int sockfd, int cookie,
                  struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->listen_undo);

    self->file_tx->ops->listen_undo(self->file_tx, sockfd, cookie, error);
}

/*
 * lseek()
 */

off_t
fd_tx_lseek_exec(struct fd_tx* self, int fildes, off_t offset, int whence,
                 bool isnoundo, int* cookie, struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->lseek_exec);

    return self->file_tx->ops->lseek_exec(self->file_tx, fildes, offset,
                                          whence, isnoundo, cookie, error);
}

void
fd_tx_lseek_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->lseek_apply);

    self->file_tx->ops->lseek_apply(self->file_tx, fildes, cookie, error);
}

void
fd_tx_lseek_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->lseek_undo);

    self->file_tx->ops->lseek_undo(self->file_tx, fildes, cookie, error);
}

/*
 * pread()
 */

ssize_t
fd_tx_pread_exec(struct fd_tx* self, int fildes, void* buf, size_t nbyte,
                 off_t off, bool isnoundo,
                 enum picotm_libc_validation_mode val_mode, int* cookie,
                 struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->pread_exec);

    return self->file_tx->ops->pread_exec(self->file_tx, fildes, buf, nbyte,
                                          off, isnoundo, val_mode, cookie,
                                          error);
}

void
fd_tx_pread_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->pread_apply);

    self->file_tx->ops->pread_apply(self->file_tx, fildes, cookie, error);
}

void
fd_tx_pread_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->pread_undo);

    self->file_tx->ops->pread_undo(self->file_tx, fildes, cookie, error);
}

/*
 * pwrite()
 */

ssize_t
fd_tx_pwrite_exec(struct fd_tx* self, int fildes, const void* buf,
                  size_t nbyte, off_t off, bool isnoundo, int* cookie,
                  struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->pwrite_exec);

    return self->file_tx->ops->pwrite_exec(self->file_tx, fildes, buf, nbyte,
                                           off, isnoundo, cookie, error);
}

void
fd_tx_pwrite_apply(struct fd_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->pwrite_apply);

    self->file_tx->ops->pwrite_apply(self->file_tx, fildes, cookie, error);
}

void
fd_tx_pwrite_undo(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->pwrite_undo);

    self->file_tx->ops->pwrite_undo(self->file_tx, fildes, cookie, error);
}

/*
 * read()
 */

ssize_t
fd_tx_read_exec(struct fd_tx* self, int fildes, void *buf, size_t nbyte,
                bool isnoundo, enum picotm_libc_validation_mode val_mode,
                int* cookie, struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->read_exec);

    return self->file_tx->ops->read_exec(self->file_tx, fildes, buf, nbyte,
                                         isnoundo, val_mode, cookie, error);
}

void
fd_tx_read_apply(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->read_apply);

    self->file_tx->ops->read_apply(self->file_tx, fildes, cookie, error);
}

void
fd_tx_read_undo(struct fd_tx* self, int fildes, int cookie,
                struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->read_undo);

    self->file_tx->ops->read_undo(self->file_tx, fildes, cookie, error);
}

/*
 * recv()
 */

ssize_t
fd_tx_recv_exec(struct fd_tx* self, int sockfd, void* buffer, size_t length,
                int flags, bool isnoundo, int* cookie,
                struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->recv_exec);

    return self->file_tx->ops->recv_exec(self->file_tx, sockfd, buffer,
                                         length, flags, isnoundo, cookie,
                                         error);
}

void
fd_tx_recv_apply(struct fd_tx* self, int sockfd, int cookie,
                 struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->recv_apply);

    self->file_tx->ops->recv_apply(self->file_tx, sockfd, cookie, error);
}

void
fd_tx_recv_undo(struct fd_tx* self, int sockfd, int cookie,
                struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->recv_undo);

    self->file_tx->ops->recv_undo(self->file_tx, sockfd, cookie, error);
}

/*
 * send()
 */

ssize_t
fd_tx_send_exec(struct fd_tx* self, int sockfd, const void* buffer,
                size_t length, int flags, bool isnoundo, int* cookie,
                struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->send_exec);

    return self->file_tx->ops->send_exec(self->file_tx, sockfd, buffer,
                                         length, flags, isnoundo, cookie,
                                         error);
}

void
fd_tx_send_apply(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->send_apply);

    self->file_tx->ops->send_apply(self->file_tx, fildes, cookie, error);
}

void
fd_tx_send_undo(struct fd_tx* self, int fildes, int cookie,
                struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->send_undo);

    self->file_tx->ops->send_undo(self->file_tx, fildes, cookie, error);
}

/*
 * shutdown()
 */

int
fd_tx_shutdown_exec(struct fd_tx* self, int sockfd, int how, bool isnoundo,
                    int* cookie, struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->shutdown_exec);

    return self->file_tx->ops->shutdown_exec(self->file_tx, sockfd, how,
                                             isnoundo, cookie, error);
}

void
fd_tx_shutdown_apply(struct fd_tx* self, int fildes, int cookie,
                     struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->shutdown_apply);

    self->file_tx->ops->shutdown_apply(self->file_tx, fildes, cookie, error);
}

void
fd_tx_shutdown_undo(struct fd_tx* self, int fildes, int cookie,
                    struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->shutdown_undo);

    self->file_tx->ops->shutdown_undo(self->file_tx, fildes, cookie, error);
}

/*
 * write()
 */

ssize_t
fd_tx_write_exec(struct fd_tx* self, int fildes, const void* buf,
                 size_t nbyte, bool isnoundo, int* cookie,
                 struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->write_exec);

    return self->file_tx->ops->write_exec(self->file_tx, fildes, buf, nbyte,
                                          isnoundo, cookie, error);
}

void
fd_tx_write_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->write_apply);

    self->file_tx->ops->write_apply(self->file_tx, fildes, cookie, error);
}

void
fd_tx_write_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    assert(self);
    assert(self->file_tx);
    assert(self->file_tx->ops);
    assert(self->file_tx->ops->write_undo);

    self->file_tx->ops->write_undo(self->file_tx, fildes, cookie, error);
}
