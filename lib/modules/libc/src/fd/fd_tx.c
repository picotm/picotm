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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fcntlop.h"
#include "fcntloptab.h"
#include "fd.h"
#include "ofd_tx.h"

void
fd_tx_init(struct fd_tx* self)
{
    assert(self);

    picotm_ref_init(&self->ref, 0);

    memset(&self->active_list, 0, sizeof(self->active_list));

    self->fd = NULL;
    self->ofd_tx = NULL;
	self->flags = 0;
	self->cc_mode = PICOTM_LIBC_CC_MODE_2PL;

    picotm_rwstate_init(&self->rwstate);

    self->fcntltab = NULL;
    self->fcntltablen = 0;
}

void
fd_tx_uninit(struct fd_tx* self)
{
    assert(self);
}

void
fd_tx_ref_or_set_up(struct fd_tx* self, struct fd* fd, struct ofd_tx* ofd_tx,
                    unsigned long flags, struct picotm_error* error)
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

    ofd_tx_ref(ofd_tx);

    self->fd = fd;
    self->ofd_tx = ofd_tx;
    self->flags = flags & FD_FL_WANTNEW ? FDTX_FL_LOCALSTATE : 0;

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

    ofd_tx_unref(self->ofd_tx);
    fd_unref(self->fd);

    self->flags = 0;
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

	/* fastpath: no dependencies to other domains */
	if (!(self->flags&FDTX_FL_LOCALSTATE)) {
		return;
	}
}

void
fd_tx_update_cc(struct fd_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(fd_tx_holds_ref(self));

    /* release file-descriptor lock */
    fd_unlock(self->fd, &self->rwstate);
}

void
fd_tx_clear_cc(struct fd_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(fd_tx_holds_ref(self));

    /* release file-descriptor lock */
    fd_unlock(self->fd, &self->rwstate);
}

/*
 * bind()
 */

int
fd_tx_bind_exec(struct fd_tx* self, int sockfd, const struct sockaddr* address,
                socklen_t addresslen, int* cookie, int isnoundo,
                struct picotm_error* error)
{
    assert(self);

    int res = ofd_tx_bind_exec(self->ofd_tx,
                               sockfd, address, addresslen,
                               cookie, isnoundo, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    return res;
}

void
fd_tx_bind_apply(struct fd_tx* self, int sockfd,
                 const struct fd_event* event,
                 struct picotm_error* error)
{
    assert(self);

    ofd_tx_bind_apply(self->ofd_tx, sockfd, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fd_tx_bind_undo(struct fd_tx* self, int sockfd, int cookie,
                struct picotm_error* error)
{
    assert(self);

    ofd_tx_bind_undo(self->ofd_tx, sockfd, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * close()
 */

static int
close_exec_noundo(struct fd_tx* self, int fildes, int* cookie,
                  struct picotm_error* error)
{
    /* acquire writer lock on file descriptor */
    fd_try_wrlock(self->fd, &self->rwstate, error);
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
    fd_try_wrlock(self->fd, &self->rwstate, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    *cookie = 0; /* injects event */
    return 0;
}

int
fd_tx_close_exec(struct fd_tx* self, int fildes, int* cookie, int noundo,
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
                   int* cookie, int isnoundo, struct picotm_error* error)
{
    assert(self);

    int res = ofd_tx_connect_exec(self->ofd_tx,
                                  sockfd, address, addresslen,
                                  cookie, isnoundo, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    return res;
}

void
fd_tx_connect_apply(struct fd_tx* self, int sockfd,
                    const struct fd_event* event,
                    struct picotm_error* error)
{
    assert(self);

    ofd_tx_connect_apply(self->ofd_tx, sockfd, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fd_tx_connect_undo(struct fd_tx* self, int sockfd, int cookie,
                   struct picotm_error* error)
{
    assert(self);

    ofd_tx_connect_undo(self->ofd_tx, sockfd, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * fcntl()
 */

int
fd_tx_fcntl_exec(struct fd_tx* self, int fildes, int cmd,
                 union fcntl_arg* arg, int* cookie, int isnoundo,
                 struct picotm_error* error)
{
    assert(self);
    assert(self->fd);

    /* acquire reader lock on file descriptor */
    fd_try_rdlock(self->fd, &self->rwstate, error);
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
            res = ofd_tx_fcntl_exec(self->ofd_tx, fildes, cmd, arg, cookie,
                                    isnoundo, error);
            if (picotm_error_is_set(error)) {
                break;
            }
            break;
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

	self->flags |= FDTX_FL_LOCALSTATE;

    return res;

err_cmd:
    return -1;
}

void
fd_tx_fcntl_apply(struct fd_tx* self, int fildes, int cookie,
                  const struct fd_event* event,
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
            ofd_tx_fcntl_apply(self->ofd_tx, fildes, event, error);
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
            ofd_tx_fcntl_undo(self->ofd_tx, fildes, cookie, error);
            if (picotm_error_is_set(error)) {
                return;
            }
            break;
    }
}


/*
 * fsync()
 */

int
fd_tx_fsync_exec(struct fd_tx* self, int fildes, int isnoundo, int* cookie,
                 struct picotm_error* error)
{
    assert(self);

    int res = ofd_tx_fsync_exec(self->ofd_tx, fildes, isnoundo, cookie,
                                error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return res;
}

void
fd_tx_fsync_apply(struct fd_tx* self, int fildes,
                  const struct fd_event* event,
                  struct picotm_error* error)
{
    assert(self);

    ofd_tx_fsync_apply(self->ofd_tx, fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fd_tx_fsync_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    assert(self);

    ofd_tx_fsync_undo(self->ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * listen()
 */

int
fd_tx_listen_exec(struct fd_tx* self, int sockfd, int backlog, int* cookie,
                  int isnoundo, struct picotm_error* error)
{
    assert(self);

    int res = ofd_tx_listen_exec(self->ofd_tx, sockfd, backlog, cookie,
                                 isnoundo, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return res;
}

void
fd_tx_listen_apply(struct fd_tx* self, int sockfd,
                   const struct fd_event* event,
                   struct picotm_error* error)
{
    assert(self);

    ofd_tx_listen_apply(self->ofd_tx, sockfd, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fd_tx_listen_undo(struct fd_tx* self, int sockfd, int cookie,
                  struct picotm_error* error)
{
    assert(self);

    ofd_tx_listen_undo(self->ofd_tx, sockfd, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * lseek()
 */

off_t
fd_tx_lseek_exec(struct fd_tx* self, int fildes, off_t offset, int whence,
                 int* cookie, int isnoundo, struct picotm_error* error)
{
    assert(self);

    off_t pos = ofd_tx_lseek_exec(self->ofd_tx,
                                  fildes, offset, whence,
                                  cookie, isnoundo, error);
    if (picotm_error_is_set(error)) {
        return (off_t)-1;
    }
    return pos;
}

void
fd_tx_lseek_apply(struct fd_tx* self, int fildes,
                  const struct fd_event* event,
                  struct picotm_error* error)
{
    assert(self);

    ofd_tx_lseek_apply(self->ofd_tx, fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fd_tx_lseek_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    assert(self);

    ofd_tx_lseek_undo(self->ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * pread()
 */

ssize_t
fd_tx_pread_exec(struct fd_tx* self, int fildes, void* buf, size_t nbyte,
                 off_t off, int* cookie, int isnoundo,
                 enum picotm_libc_validation_mode val_mode,
                 struct picotm_error* error)
{
    assert(self);

    ssize_t len = ofd_tx_pread_exec(self->ofd_tx,
                                    fildes, buf, nbyte, off,
                                    cookie, isnoundo, val_mode, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return len;
}

void
fd_tx_pread_apply(struct fd_tx* self, int fildes,
                  const struct fd_event* event,
                  struct picotm_error* error)
{
    assert(self);

    ofd_tx_pread_apply(self->ofd_tx, fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fd_tx_pread_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    assert(self);

    ofd_tx_pread_undo(self->ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * pwrite()
 */

ssize_t
fd_tx_pwrite_exec(struct fd_tx* self, int fildes, const void* buf,
                  size_t nbyte, off_t off, int* cookie, int isnoundo,
                  struct picotm_error* error)
{
    assert(self);

    ssize_t len = ofd_tx_pwrite_exec(self->ofd_tx,
                                     fildes, buf, nbyte, off,
                                     cookie, isnoundo, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return len;
}

void
fd_tx_pwrite_apply(struct fd_tx* self, int fildes,
                   const struct fd_event* event,
                   struct picotm_error* error)
{
    assert(self);

    ofd_tx_pwrite_apply(self->ofd_tx, fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fd_tx_pwrite_undo(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    assert(self);

    ofd_tx_pwrite_undo(self->ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * read()
 */

ssize_t
fd_tx_read_exec(struct fd_tx* self, int fildes, void *buf, size_t nbyte,
                int* cookie, int isnoundo,
                enum picotm_libc_validation_mode val_mode,
                struct picotm_error* error)
{
    assert(self);

    ssize_t len = ofd_tx_read_exec(self->ofd_tx,
                                   fildes, buf, nbyte,
                                   cookie, isnoundo, val_mode,
                                   error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return len;
}

void
fd_tx_read_apply(struct fd_tx* self, int fildes,
                 const struct fd_event* event,
                 struct picotm_error* error)
{
    assert(self);

    ofd_tx_read_apply(self->ofd_tx, fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fd_tx_read_undo(struct fd_tx* self, int fildes, int cookie,
                struct picotm_error* error)
{
    assert(self);

    ofd_tx_read_undo(self->ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * recv()
 */

ssize_t
fd_tx_recv_exec(struct fd_tx* self, int sockfd, void* buffer, size_t length,
                int flags, int* cookie, int isnoundo,
                struct picotm_error* error)
{
    assert(self);

    ssize_t len = ofd_tx_recv_exec(self->ofd_tx,
                                   sockfd, buffer, length, flags,
                                   cookie, isnoundo, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return len;
}

void
fd_tx_recv_apply(struct fd_tx* self, int sockfd,
                 const struct fd_event* event,
                 struct picotm_error* error)
{
    assert(self);

    ofd_tx_recv_apply(self->ofd_tx, sockfd, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fd_tx_recv_undo(struct fd_tx* self, int sockfd, int cookie,
                struct picotm_error* error)
{
    assert(self);

    ofd_tx_recv_undo(self->ofd_tx, sockfd, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * send()
 */

ssize_t
fd_tx_send_exec(struct fd_tx* self, int sockfd, const void* buffer,
                size_t length, int flags, int* cookie, int isnoundo,
                struct picotm_error* error)
{
    assert(self);

    ssize_t len = ofd_tx_send_exec(self->ofd_tx,
                                   sockfd, buffer, length, flags,
                                   cookie, isnoundo, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return len;
}

void
fd_tx_send_apply(struct fd_tx* self, int fildes,
                 const struct fd_event* event,
                 struct picotm_error* error)
{
    assert(self);

    ofd_tx_send_apply(self->ofd_tx, fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fd_tx_send_undo(struct fd_tx* self, int fildes, int cookie,
                struct picotm_error* error)
{
    assert(self);

    ofd_tx_send_undo(self->ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * shutdown()
 */

int
fd_tx_shutdown_exec(struct fd_tx* self, int sockfd, int how, int* cookie,
                    int isnoundo, struct picotm_error* error)
{
    assert(self);

    int len = ofd_tx_shutdown_exec(self->ofd_tx, sockfd, how, cookie,
                                   isnoundo, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return len;
}

void
fd_tx_shutdown_apply(struct fd_tx* self, int fildes,
                     const struct fd_event* event,
                     struct picotm_error* error)
{
    assert(self);

    ofd_tx_shutdown_apply(self->ofd_tx, fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fd_tx_shutdown_undo(struct fd_tx* self, int fildes, int cookie,
                    struct picotm_error* error)
{
    assert(self);

    ofd_tx_shutdown_undo(self->ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * write()
 */

ssize_t
fd_tx_write_exec(struct fd_tx* self, int fildes, const void* buf,
                 size_t nbyte, int* cookie, int isnoundo,
                 struct picotm_error* error)
{
    assert(self);

    ssize_t len = ofd_tx_write_exec(self->ofd_tx, fildes, buf, nbyte, cookie,
                                    isnoundo, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return len;
}

void
fd_tx_write_apply(struct fd_tx* self, int fildes,
                  const struct fd_event* event,
                  struct picotm_error* error)
{
    assert(self);

    ofd_tx_write_apply(self->ofd_tx, fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fd_tx_write_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    assert(self);

    ofd_tx_write_undo(self->ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}
