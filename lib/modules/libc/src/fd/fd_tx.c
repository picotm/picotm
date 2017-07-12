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
#include "chrdev_tx.h"
#include "dir_tx.h"
#include "fcntlop.h"
#include "fcntloptab.h"
#include "fd.h"
#include "fifo_tx.h"
#include "regfile_tx.h"
#include "socket_tx.h"

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

static int
bind_exec_chrdev(struct ofd_tx* ofd_tx, int sockfd,
                 const struct sockaddr* address, socklen_t addresslen,
                 bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static int
bind_exec_fifo(struct ofd_tx* ofd_tx, int sockfd,
               const struct sockaddr* address, socklen_t addresslen,
               bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static int
bind_exec_regfile(struct ofd_tx* ofd_tx, int sockfd,
                  const struct sockaddr* address, socklen_t addresslen,
                  bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static int
bind_exec_dir(struct ofd_tx* ofd_tx, int sockfd,
              const struct sockaddr* address, socklen_t addresslen,
              bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static int
bind_exec_socket(struct ofd_tx* ofd_tx, int sockfd,
                 const struct sockaddr* address, socklen_t addresslen,
                 bool isnoundo, int* cookie, struct picotm_error* error)
{
    return socket_tx_bind_exec(socket_tx_of_ofd_tx(ofd_tx),
                               sockfd, address, addresslen,
                               isnoundo, cookie, error);
}

int
fd_tx_bind_exec(struct fd_tx* self, int sockfd, const struct sockaddr* address,
                socklen_t addresslen, bool isnoundo, int* cookie,
                struct picotm_error* error)
{
    static int (* const bind_exec[])(struct ofd_tx*,
                                     int,
                                     const struct sockaddr*,
                                     socklen_t,
                                     bool,
                                     int*,
                                     struct picotm_error*) = {
        bind_exec_chrdev,
        bind_exec_fifo,
        bind_exec_regfile,
        bind_exec_dir,
        bind_exec_socket
    };

    assert(self);

    return bind_exec[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, sockfd,
                                                     address, addresslen,
                                                     isnoundo, cookie, error);
}

static void
bind_apply_socket(struct ofd_tx* ofd_tx, int sockfd, int cookie,
                  struct picotm_error* error)
{
    socket_tx_bind_apply(socket_tx_of_ofd_tx(ofd_tx), sockfd, cookie, error);
}

void
fd_tx_bind_apply(struct fd_tx* self, int sockfd, int cookie,
                 struct picotm_error* error)
{
    static void (* const bind_apply[])(struct ofd_tx*,
                                       int,
                                       int,
                                       struct picotm_error*) = {
        NULL,
        NULL,
        NULL,
        NULL,
        bind_apply_socket
    };

    assert(self);

    return bind_apply[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, sockfd,
                                                      cookie, error);
}

static void
bind_undo_socket(struct ofd_tx* ofd_tx, int sockfd, int cookie,
                 struct picotm_error* error)
{
    socket_tx_bind_undo(socket_tx_of_ofd_tx(ofd_tx), sockfd, cookie, error);
}

void
fd_tx_bind_undo(struct fd_tx* self, int sockfd, int cookie,
                struct picotm_error* error)
{
    static void (* const bind_undo[])(struct ofd_tx*,
                                      int,
                                      int,
                                      struct picotm_error*) = {
        NULL,
        NULL,
        NULL,
        NULL,
        bind_undo_socket
    };

    assert(self);

    return bind_undo[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, sockfd,
                                                     cookie, error);
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
connect_exec_chrdev(struct ofd_tx* ofd_tx, int sockfd,
                    const struct sockaddr* address, socklen_t addresslen,
                    bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

int
connect_exec_fifo(struct ofd_tx* ofd_tx, int sockfd,
                  const struct sockaddr* address, socklen_t addresslen,
                  bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

int
connect_exec_regfile(struct ofd_tx* ofd_tx, int sockfd,
                     const struct sockaddr* address, socklen_t addresslen,
                     bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

int
connect_exec_dir(struct ofd_tx* ofd_tx, int sockfd,
                 const struct sockaddr* address, socklen_t addresslen,
                 bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

int
connect_exec_socket(struct ofd_tx* ofd_tx, int sockfd,
                    const struct sockaddr* address, socklen_t addresslen,
                    bool isnoundo, int* cookie, struct picotm_error* error)
{
    return socket_tx_connect_exec(socket_tx_of_ofd_tx(ofd_tx), sockfd,
                                  address, addresslen, isnoundo, cookie,
                                  error);
}

int
fd_tx_connect_exec(struct fd_tx* self, int sockfd,
                   const struct sockaddr* address, socklen_t addresslen,
                   bool isnoundo, int* cookie, struct picotm_error* error)
{
    static int (* const connect_exec[])(struct ofd_tx*,
                                        int,
                                        const struct sockaddr*,
                                        socklen_t,
                                        bool,
                                        int*,
                                        struct picotm_error*) = {
        connect_exec_chrdev,
        connect_exec_fifo,
        connect_exec_regfile,
        connect_exec_dir,
        connect_exec_socket
    };

    assert(self);

    return connect_exec[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx,
                                         sockfd, address, addresslen,
                                         isnoundo, cookie, error);
}

static void
connect_apply_socket(struct ofd_tx* ofd_tx, int sockfd, int cookie,
                     struct picotm_error* error)
{
    socket_tx_connect_apply(socket_tx_of_ofd_tx(ofd_tx), sockfd, cookie,
                            error);
}

void
fd_tx_connect_apply(struct fd_tx* self, int sockfd, int cookie,
                    struct picotm_error* error)
{
    static void (* const connect_apply[])(struct ofd_tx*,
                                          int,
                                          int,
                                          struct picotm_error*) = {
        NULL,
        NULL,
        NULL,
        NULL,
        connect_apply_socket
    };

    assert(self);

    connect_apply[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, sockfd,
                                                  cookie, error);
}

static void
connect_undo_socket(struct ofd_tx* ofd_tx, int sockfd, int cookie,
                    struct picotm_error* error)
{
    socket_tx_connect_undo(socket_tx_of_ofd_tx(ofd_tx), sockfd, cookie,
                           error);
}

void
fd_tx_connect_undo(struct fd_tx* self, int sockfd, int cookie,
                   struct picotm_error* error)
{
    static void (* const connect_undo[])(struct ofd_tx*,
                                         int,
                                         int,
                                         struct picotm_error*) = {
        NULL,
        NULL,
        NULL,
        NULL,
        connect_undo_socket
    };

    assert(self);

    connect_undo[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, sockfd,
                                                 cookie, error);
}

/*
 * fcntl()
 */

static int
fcntl_exec_chrdev(struct ofd_tx* ofd_tx, int fildes, int cmd,
                  union fcntl_arg* arg, bool isnoundo, int* cookie,
                  struct picotm_error* error)
{
    return chrdev_tx_fcntl_exec(chrdev_tx_of_ofd_tx(ofd_tx), fildes, cmd, arg,
                                isnoundo, cookie, error);
}

static int
fcntl_exec_fifo(struct ofd_tx* ofd_tx, int fildes, int cmd,
                union fcntl_arg* arg, bool isnoundo, int* cookie,
                struct picotm_error* error)
{
    return fifo_tx_fcntl_exec(fifo_tx_of_ofd_tx(ofd_tx), fildes, cmd, arg,
                              isnoundo, cookie, error);
}

static int
fcntl_exec_regfile(struct ofd_tx* ofd_tx, int fildes, int cmd,
                   union fcntl_arg* arg, bool isnoundo, int* cookie,
                   struct picotm_error* error)
{
    return regfile_tx_fcntl_exec(regfile_tx_of_ofd_tx(ofd_tx), fildes, cmd,
                                 arg, isnoundo, cookie, error);
}

static int
fcntl_exec_dir(struct ofd_tx* ofd_tx, int fildes, int cmd,
               union fcntl_arg* arg, bool isnoundo, int* cookie,
               struct picotm_error* error)
{
    return dir_tx_fcntl_exec(dir_tx_of_ofd_tx(ofd_tx), fildes, cmd,
                             arg, isnoundo, cookie, error);
}

static int
fcntl_exec_socket(struct ofd_tx* ofd_tx, int fildes, int cmd,
                  union fcntl_arg* arg, bool isnoundo, int* cookie,
                  struct picotm_error* error)
{
    return socket_tx_fcntl_exec(socket_tx_of_ofd_tx(ofd_tx), fildes, cmd,
                                arg, isnoundo, cookie, error);
}

int
fd_tx_fcntl_exec(struct fd_tx* self, int fildes, int cmd,
                 union fcntl_arg* arg, bool isnoundo, int* cookie,
                 struct picotm_error* error)
{
    static int (* const fcntl_exec[])(struct ofd_tx*,
                                      int,
                                      int,
                                      union fcntl_arg*,
                                      bool,
                                      int*,
                                      struct picotm_error*) = {
        fcntl_exec_chrdev,
        fcntl_exec_fifo,
        fcntl_exec_regfile,
        fcntl_exec_dir,
        fcntl_exec_socket
    };

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
            return fcntl_exec[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx,
                                                              fildes, cmd,
                                                              arg, isnoundo,
                                                              cookie, error);
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
fcntl_apply_chrdev(struct ofd_tx* ofd_tx, int fildes, int cookie,
                   struct picotm_error* error)
{
    chrdev_tx_fcntl_apply(chrdev_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

void
fcntl_apply_fifo(struct ofd_tx* ofd_tx, int fildes, int cookie,
                 struct picotm_error* error)
{
    fifo_tx_fcntl_apply(fifo_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

void
fcntl_apply_regfile(struct ofd_tx* ofd_tx, int fildes, int cookie,
                    struct picotm_error* error)
{
    regfile_tx_fcntl_apply(regfile_tx_of_ofd_tx(ofd_tx), fildes, cookie,
                           error);
}

void
fcntl_apply_dir(struct ofd_tx* ofd_tx, int fildes, int cookie,
                struct picotm_error* error)
{
    dir_tx_fcntl_apply(dir_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

void
fcntl_apply_socket(struct ofd_tx* ofd_tx, int fildes, int cookie,
                   struct picotm_error* error)
{
    socket_tx_fcntl_apply(socket_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

void
fd_tx_fcntl_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static void (* const fcntl_apply[])(struct ofd_tx*,
                                        int,
                                        int,
                                        struct picotm_error*) = {
        fcntl_apply_chrdev,
        fcntl_apply_fifo,
        fcntl_apply_regfile,
        fcntl_apply_dir,
        fcntl_apply_socket
    };

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
            fcntl_apply[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                                        cookie, error);
            if (picotm_error_is_set(error)) {
                return;
            }
            break;
    }
}

void
fcntl_undo_chrdev(struct ofd_tx* ofd_tx, int fildes, int cookie,
                  struct picotm_error* error)
{
    chrdev_tx_fcntl_undo(chrdev_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

void
fcntl_undo_fifo(struct ofd_tx* ofd_tx, int fildes, int cookie,
                struct picotm_error* error)
{
    fifo_tx_fcntl_undo(fifo_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

void
fcntl_undo_regfile(struct ofd_tx* ofd_tx, int fildes, int cookie,
                   struct picotm_error* error)
{
    regfile_tx_fcntl_undo(regfile_tx_of_ofd_tx(ofd_tx), fildes, cookie,
                          error);
}

void
fcntl_undo_dir(struct ofd_tx* ofd_tx, int fildes, int cookie,
               struct picotm_error* error)
{
    dir_tx_fcntl_undo(dir_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

void
fcntl_undo_socket(struct ofd_tx* ofd_tx, int fildes, int cookie,
                  struct picotm_error* error)
{
    socket_tx_fcntl_undo(socket_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}


void
fd_tx_fcntl_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    static void (* const fcntl_undo[])(struct ofd_tx*,
                                       int,
                                       int,
                                       struct picotm_error*) = {
        fcntl_undo_chrdev,
        fcntl_undo_fifo,
        fcntl_undo_regfile,
        fcntl_undo_dir,
        fcntl_undo_socket
    };

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
            fcntl_undo[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                                       cookie, error);
            if (picotm_error_is_set(error)) {
                return;
            }
            break;
    }
}

/*
 * fsync()
 */

static int
fsync_exec_chrdev(struct ofd_tx* ofd_tx, int fildes, bool isnoundo,
                  int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, EINVAL);
    return -1;
}

static int
fsync_exec_fifo(struct ofd_tx* ofd_tx, int fildes, bool isnoundo,
                int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, EINVAL);
    return -1;
}

static int
fsync_exec_regfile(struct ofd_tx* ofd_tx, int fildes, bool isnoundo,
                   int* cookie, struct picotm_error* error)
{
    return regfile_tx_fsync_exec(regfile_tx_of_ofd_tx(ofd_tx), fildes,
                                 isnoundo, cookie, error);
}

static int
fsync_exec_dir(struct ofd_tx* ofd_tx, int fildes, bool isnoundo,
               int* cookie, struct picotm_error* error)
{
    return dir_tx_fsync_exec(dir_tx_of_ofd_tx(ofd_tx), fildes,
                             isnoundo, cookie, error);
}

static int
fsync_exec_socket(struct ofd_tx* ofd_tx, int fildes, bool isnoundo,
                  int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, EINVAL);
    return -1;
}

int
fd_tx_fsync_exec(struct fd_tx* self, int fildes, bool isnoundo, int* cookie,
                 struct picotm_error* error)
{
    static int (* const fsync_exec[])(struct ofd_tx*,
                                      int,
                                      bool,
                                      int* cookie,
                                      struct picotm_error*) = {
        fsync_exec_chrdev,
        fsync_exec_fifo,
        fsync_exec_regfile,
        fsync_exec_dir,
        fsync_exec_socket
    };

    assert(self);

    return fsync_exec[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                                      isnoundo, cookie,
                                                      error);
}

static void
fsync_apply_regfile(struct ofd_tx* ofd_tx, int fildes, int cookie,
                    struct picotm_error* error)
{
    regfile_tx_fsync_apply(regfile_tx_of_ofd_tx(ofd_tx), fildes, cookie,
                           error);
}

static void
fsync_apply_dir(struct ofd_tx* ofd_tx, int fildes, int cookie,
                struct picotm_error* error)
{
    dir_tx_fsync_apply(dir_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

void
fd_tx_fsync_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static void (* const fsync_apply[])(struct ofd_tx*,
                                        int,
                                        int,
                                        struct picotm_error*) = {
        NULL,
        NULL,
        fsync_apply_regfile,
        fsync_apply_dir,
        NULL
    };

    assert(self);

    fsync_apply[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                                cookie, error);
}

static void
fsync_undo_regfile(struct ofd_tx* ofd_tx, int fildes, int cookie,
                   struct picotm_error* error)
{
    regfile_tx_fsync_undo(regfile_tx_of_ofd_tx(ofd_tx), fildes, cookie,
                          error);
}

static void
fsync_undo_dir(struct ofd_tx* ofd_tx, int fildes, int cookie,
               struct picotm_error* error)
{
    dir_tx_fsync_undo(dir_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

void
fd_tx_fsync_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    static void (* const fsync_undo[])(struct ofd_tx*,
                                       int,
                                       int,
                                       struct picotm_error*) = {
        NULL,
        NULL,
        fsync_undo_regfile,
        fsync_undo_dir,
        NULL
    };

    assert(self);

    fsync_undo[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                               cookie, error);
}

/*
 * listen()
 */

static int
listen_exec_chrdev(struct ofd_tx* ofd_tx, int sockfd, int backlog,
                   bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static int
listen_exec_fifo(struct ofd_tx* ofd_tx, int sockfd, int backlog,
                 bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static int
listen_exec_regfile(struct ofd_tx* ofd_tx, int sockfd, int backlog,
                    bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static int
listen_exec_dir(struct ofd_tx* ofd_tx, int sockfd, int backlog,
                bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static int
listen_exec_socket(struct ofd_tx* ofd_tx, int sockfd, int backlog,
                   bool isnoundo, int* cookie, struct picotm_error* error)
{
    return socket_tx_listen_exec(socket_tx_of_ofd_tx(ofd_tx), sockfd, backlog,
                                 isnoundo, cookie, error);
}

int
fd_tx_listen_exec(struct fd_tx* self, int sockfd, int backlog, bool isnoundo,
                  int* cookie, struct picotm_error* error)
{
    static int (* const listen_exec[])(struct ofd_tx*,
                                       int,
                                       int,
                                       bool,
                                       int*,
                                       struct picotm_error*) = {
        listen_exec_chrdev,
        listen_exec_fifo,
        listen_exec_regfile,
        listen_exec_dir,
        listen_exec_socket
    };

    assert(self);

    return listen_exec[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, sockfd,
                                                       backlog, isnoundo,
                                                       cookie, error);
}

static void
listen_apply_socket(struct ofd_tx* ofd_tx, int sockfd, int cookie,
                    struct picotm_error* error)
{
    socket_tx_listen_apply(socket_tx_of_ofd_tx(ofd_tx), sockfd, cookie, error);
}

void
fd_tx_listen_apply(struct fd_tx* self, int sockfd, int cookie,
                   struct picotm_error* error)
{
    static void (* const listen_apply[])(struct ofd_tx*,
                                         int,
                                         int,
                                         struct picotm_error*) = {
        NULL,
        NULL,
        NULL,
        NULL,
        listen_apply_socket
    };

    assert(self);

    listen_apply[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, sockfd,
                                                 cookie, error);
}

static void
listen_undo_socket(struct ofd_tx* ofd_tx, int sockfd, int cookie,
                   struct picotm_error* error)
{
    socket_tx_listen_undo(socket_tx_of_ofd_tx(ofd_tx), sockfd, cookie, error);
}

void
fd_tx_listen_undo(struct fd_tx* self, int sockfd, int cookie,
                  struct picotm_error* error)
{
    static void (* const listen_undo[])(struct ofd_tx*,
                                        int,
                                        int,
                                        struct picotm_error*) = {
        NULL,
        NULL,
        NULL,
        NULL,
        listen_undo_socket
    };

    assert(self);

    listen_undo[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, sockfd,
                                                cookie, error);
}

/*
 * lseek()
 */

static off_t
lseek_exec_chrdev(struct ofd_tx* ofd_tx, int fildes, off_t offset, int whence,
                  bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return (off_t)-1;
}

static off_t
lseek_exec_fifo(struct ofd_tx* ofd_tx, int fildes, off_t offset, int whence,
                bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return (off_t)-1;
}

static off_t
lseek_exec_regfile(struct ofd_tx* ofd_tx, int fildes, off_t offset, int whence,
                   bool isnoundo, int* cookie, struct picotm_error* error)
{
    return regfile_tx_lseek_exec(regfile_tx_of_ofd_tx(ofd_tx), fildes, offset,
                                 whence, isnoundo, cookie, error);
}

static off_t
lseek_exec_dir(struct ofd_tx* ofd_tx, int fildes, off_t offset, int whence,
               bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, EINVAL);
    return (off_t)-1;
}

static off_t
lseek_exec_socket(struct ofd_tx* ofd_tx, int fildes, off_t offset, int whence,
                  bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return (off_t)-1;
}

off_t
fd_tx_lseek_exec(struct fd_tx* self, int fildes, off_t offset, int whence,
                 bool isnoundo, int* cookie, struct picotm_error* error)
{
    static off_t (*lseek_exec[])(struct ofd_tx*,
                                 int,
                                 off_t,
                                 int,
                                 bool,
                                 int*,
                                 struct picotm_error*) = {
        lseek_exec_chrdev,
        lseek_exec_fifo,
        lseek_exec_regfile,
        lseek_exec_dir,
        lseek_exec_socket
    };

    assert(self);

    return lseek_exec[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                                      offset, whence,
                                                      isnoundo, cookie,
                                                      error);
}

static void
lseek_apply_regfile(struct ofd_tx* ofd_tx, int fildes, int cookie,
                    struct picotm_error* error)
{
    regfile_tx_lseek_apply(regfile_tx_of_ofd_tx(ofd_tx), fildes, cookie,
                           error);
}

void
fd_tx_lseek_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static void (* const lseek_apply[])(struct ofd_tx*,
                                        int,
                                        int,
                                        struct picotm_error*) = {
        NULL,
        NULL,
        lseek_apply_regfile,
        NULL,
        NULL
    };

    assert(self);

    lseek_apply[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                                cookie, error);
}

static void
lseek_undo_regfile(struct ofd_tx* ofd_tx, int fildes, int cookie,
                   struct picotm_error* error)
{
    regfile_tx_lseek_undo(regfile_tx_of_ofd_tx(ofd_tx), fildes, cookie,
                          error);
}

void
fd_tx_lseek_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    static void (* const lseek_undo[])(struct ofd_tx*,
                                       int,
                                       int,
                                       struct picotm_error*) = {
        NULL,
        NULL,
        lseek_undo_regfile,
        NULL,
        NULL
    };

    assert(self);

    lseek_undo[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                               cookie, error);
}

/*
 * pread()
 */

static ssize_t
pread_exec_chrdev(struct ofd_tx* ofd_tx, int fildes, void* buf, size_t nbyte,
                  off_t off, bool isnoundo,
                  enum picotm_libc_validation_mode val_mode, int* cookie,
                  struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return -1;
}

static ssize_t
pread_exec_fifo(struct ofd_tx* ofd_tx, int fildes, void* buf, size_t nbyte,
                off_t off, bool isnoundo,
                enum picotm_libc_validation_mode val_mode, int* cookie,
                struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return -1;
}

static ssize_t
pread_exec_regfile(struct ofd_tx* ofd_tx, int fildes, void* buf, size_t nbyte,
                   off_t off, bool isnoundo,
                   enum picotm_libc_validation_mode val_mode, int* cookie,
                   struct picotm_error* error)
{
    return regfile_tx_pread_exec(regfile_tx_of_ofd_tx(ofd_tx), fildes, buf,
                                 nbyte, off, isnoundo, val_mode, cookie,
                                 error);
}

static ssize_t
pread_exec_dir(struct ofd_tx* ofd_tx, int fildes, void* buf, size_t nbyte,
               off_t off, bool isnoundo,
               enum picotm_libc_validation_mode val_mode, int* cookie,
               struct picotm_error* error)
{
    picotm_error_set_errno(error, EISDIR);
    return -1;
}

static ssize_t
pread_exec_socket(struct ofd_tx* ofd_tx, int fildes, void* buf, size_t nbyte,
                  off_t off, bool isnoundo,
                  enum picotm_libc_validation_mode val_mode, int* cookie,
                  struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return -1;
}

ssize_t
fd_tx_pread_exec(struct fd_tx* self, int fildes, void* buf, size_t nbyte,
                 off_t off, bool isnoundo,
                 enum picotm_libc_validation_mode val_mode, int* cookie,
                 struct picotm_error* error)
{
    static ssize_t (* const pread_exec[])(struct ofd_tx*,
                                          int,
                                          void*,
                                          size_t,
                                          off_t,
                                          bool,
                                          enum picotm_libc_validation_mode,
                                          int*,
                                          struct picotm_error*) = {
        pread_exec_chrdev,
        pread_exec_fifo,
        pread_exec_regfile,
        pread_exec_dir,
        pread_exec_socket
    };

    assert(self);

    return pread_exec[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                                      buf, nbyte, off,
                                                      isnoundo, val_mode,
                                                      cookie, error);
}

static void
pread_apply_regfile(struct ofd_tx* ofd_tx, int fildes, int cookie,
                    struct picotm_error* error)
{
    regfile_tx_pread_apply(regfile_tx_of_ofd_tx(ofd_tx), fildes, cookie,
                           error);
}

void
fd_tx_pread_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static void (* const pread_apply[])(struct ofd_tx*,
                                        int,
                                        int,
                                        struct picotm_error*) = {
        NULL,
        NULL,
        pread_apply_regfile,
        NULL,
        NULL
    };

    assert(self);

    pread_apply[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                                cookie, error);
}

static void
pread_undo_regfile(struct ofd_tx* ofd_tx, int fildes, int cookie,
                   struct picotm_error* error)
{
    regfile_tx_pread_undo(regfile_tx_of_ofd_tx(ofd_tx), fildes, cookie,
                          error);
}

void
fd_tx_pread_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    static void (* const pread_undo[])(struct ofd_tx*,
                                       int,
                                       int,
                                       struct picotm_error*) = {
        NULL,
        NULL,
        pread_undo_regfile,
        NULL,
        NULL
    };

    assert(self);

    pread_undo[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes, cookie,
                                               error);
}

/*
 * pwrite()
 */

static ssize_t
pwrite_exec_chrdev(struct ofd_tx* ofd_tx, int fildes, const void* buf,
                   size_t nbyte, off_t off, bool isnoundo, int* cookie,
                   struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return -1;
}

static ssize_t
pwrite_exec_fifo(struct ofd_tx* ofd_tx, int fildes, const void* buf,
                 size_t nbyte, off_t off, bool isnoundo, int* cookie,
                 struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return -1;
}

static ssize_t
pwrite_exec_regfile(struct ofd_tx* ofd_tx, int fildes, const void* buf,
                    size_t nbyte, off_t off, bool isnoundo, int* cookie,
                    struct picotm_error* error)
{
    return regfile_tx_pwrite_exec(regfile_tx_of_ofd_tx(ofd_tx), fildes,
                                  buf, nbyte, off, isnoundo, cookie, error);
}

static ssize_t
pwrite_exec_dir(struct ofd_tx* ofd_tx, int fildes, const void* buf,
                size_t nbyte, off_t off, bool isnoundo, int* cookie,
                struct picotm_error* error)
{
    picotm_error_set_errno(error, EISDIR);
    return -1;
}

static ssize_t
pwrite_exec_socket(struct ofd_tx* ofd_tx, int fildes, const void* buf,
                   size_t nbyte, off_t off, bool isnoundo, int* cookie,
                   struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return -1;
}

ssize_t
fd_tx_pwrite_exec(struct fd_tx* self, int fildes, const void* buf,
                  size_t nbyte, off_t off, bool isnoundo, int* cookie,
                  struct picotm_error* error)
{
    static ssize_t (* const pwrite_exec[])(struct ofd_tx*,
                                           int,
                                           const void*,
                                           size_t,
                                           off_t,
                                           bool,
                                           int*,
                                           struct picotm_error*) = {
        pwrite_exec_chrdev,
        pwrite_exec_fifo,
        pwrite_exec_regfile,
        pwrite_exec_dir,
        pwrite_exec_socket
    };

    assert(self);

    return pwrite_exec[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                                       buf, nbyte, off,
                                                       isnoundo, cookie,
                                                       error);
}

static void
pwrite_apply_regfile(struct ofd_tx* ofd_tx, int fildes, int cookie,
                     struct picotm_error* error)
{
    regfile_tx_pwrite_apply(regfile_tx_of_ofd_tx(ofd_tx), fildes, cookie,
                            error);
}

void
fd_tx_pwrite_apply(struct fd_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{
    static void (* const pwrite_apply[])(struct ofd_tx*,
                                         int,
                                         int,
                                         struct picotm_error*) = {
        NULL,
        NULL,
        pwrite_apply_regfile,
        NULL,
        NULL
    };

    assert(self);

    pwrite_apply[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                                cookie, error);
}

static void
pwrite_undo_regfile(struct ofd_tx* ofd_tx, int fildes, int cookie,
                    struct picotm_error* error)
{
    regfile_tx_pwrite_undo(regfile_tx_of_ofd_tx(ofd_tx), fildes, cookie,
                           error);
}

void
fd_tx_pwrite_undo(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static void (* const pwrite_undo[])(struct ofd_tx*,
                                        int,
                                        int,
                                        struct picotm_error*) = {
        NULL,
        NULL,
        pwrite_undo_regfile,
        NULL,
        NULL
    };

    assert(self);

    pwrite_undo[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                                cookie, error);
}

/*
 * read()
 */

static ssize_t
read_exec_chrdev(struct ofd_tx* ofd_tx, int fildes, void* buf, size_t nbyte,
                 bool isnoundo, enum picotm_libc_validation_mode val_mode,
                 int* cookie, struct picotm_error* error)
{
    return chrdev_tx_read_exec(chrdev_tx_of_ofd_tx(ofd_tx), fildes, buf, nbyte,
                               isnoundo, val_mode, cookie, error);
}

static ssize_t
read_exec_fifo(struct ofd_tx* ofd_tx, int fildes, void* buf, size_t nbyte,
               bool isnoundo, enum picotm_libc_validation_mode val_mode,
               int* cookie, struct picotm_error* error)
{
    return fifo_tx_read_exec(fifo_tx_of_ofd_tx(ofd_tx), fildes, buf, nbyte,
                             isnoundo, val_mode, cookie, error);
}

static ssize_t
read_exec_regfile(struct ofd_tx* ofd_tx, int fildes, void* buf, size_t nbyte,
                  bool isnoundo, enum picotm_libc_validation_mode val_mode,
                  int* cookie, struct picotm_error* error)
{
    return regfile_tx_read_exec(regfile_tx_of_ofd_tx(ofd_tx), fildes, buf,
                                nbyte, isnoundo, val_mode, cookie, error);
}

static ssize_t
read_exec_dir(struct ofd_tx* ofd_tx, int fildes, void* buf, size_t nbyte,
              bool isnoundo, enum picotm_libc_validation_mode val_mode,
              int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, EISDIR);
    return -1;
}

static ssize_t
read_exec_socket(struct ofd_tx* ofd_tx, int fildes, void* buf, size_t nbyte,
                 bool isnoundo, enum picotm_libc_validation_mode val_mode,
                 int* cookie, struct picotm_error* error)
{
    return socket_tx_read_exec(socket_tx_of_ofd_tx(ofd_tx), fildes, buf,
                               nbyte, isnoundo, val_mode, cookie, error);
}

ssize_t
fd_tx_read_exec(struct fd_tx* self, int fildes, void *buf, size_t nbyte,
                bool isnoundo, enum picotm_libc_validation_mode val_mode,
                int* cookie, struct picotm_error* error)
{
    static ssize_t (* const read_exec[])(struct ofd_tx*,
                                         int,
                                         void*,
                                         size_t,
                                         bool,
                                         enum picotm_libc_validation_mode,
                                         int*,
                                         struct picotm_error*) = {
        read_exec_chrdev,
        read_exec_fifo,
        read_exec_regfile,
        read_exec_dir,
        read_exec_socket
    };

    assert(self);

    return read_exec[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                                     buf, nbyte, isnoundo,
                                                     val_mode, cookie, error);
}

static void
read_apply_chrdev(struct ofd_tx* ofd_tx, int fildes, int cookie,
                  struct picotm_error* error)
{
    chrdev_tx_read_apply(chrdev_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

static void
read_apply_fifo(struct ofd_tx* ofd_tx, int fildes, int cookie,
                struct picotm_error* error)
{
    fifo_tx_read_apply(fifo_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

static void
read_apply_regfile(struct ofd_tx* ofd_tx, int fildes, int cookie,
                   struct picotm_error* error)
{
    regfile_tx_read_apply(regfile_tx_of_ofd_tx(ofd_tx), fildes, cookie,
                          error);
}

static void
read_apply_socket(struct ofd_tx* ofd_tx, int fildes, int cookie,
                  struct picotm_error* error)
{
    socket_tx_read_apply(socket_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

void
fd_tx_read_apply(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    static void (* const read_apply[])(struct ofd_tx*,
                                       int,
                                       int,
                                       struct picotm_error*) = {
        read_apply_chrdev,
        read_apply_fifo,
        read_apply_regfile,
        NULL,
        read_apply_socket
    };

    assert(self);

    read_apply[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                               cookie, error);
}

static void
read_undo_chrdev(struct ofd_tx* ofd_tx, int fildes, int cookie,
                 struct picotm_error* error)
{
    chrdev_tx_read_undo(chrdev_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

static void
read_undo_fifo(struct ofd_tx* ofd_tx, int fildes, int cookie,
               struct picotm_error* error)
{
    fifo_tx_read_undo(fifo_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

static void
read_undo_regfile(struct ofd_tx* ofd_tx, int fildes, int cookie,
                  struct picotm_error* error)
{
    regfile_tx_read_undo(regfile_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

static void
read_undo_socket(struct ofd_tx* ofd_tx, int fildes, int cookie,
                 struct picotm_error* error)
{
    socket_tx_read_undo(socket_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

void
fd_tx_read_undo(struct fd_tx* self, int fildes, int cookie,
                struct picotm_error* error)
{
    static void (* const read_undo[])(struct ofd_tx*,
                                      int,
                                      int,
                                      struct picotm_error*) = {
        read_undo_chrdev,
        read_undo_fifo,
        read_undo_regfile,
        NULL,
        read_undo_socket
    };

    assert(self);

    read_undo[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                              cookie, error);
}

/*
 * recv()
 */

static ssize_t
recv_exec_chrdev(struct ofd_tx* ofd_tx, int sockfd, void* buffer,
                 size_t length, int flags, bool isnoundo, int* cookie,
                 struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static ssize_t
recv_exec_fifo(struct ofd_tx* ofd_tx, int sockfd, void* buffer,
               size_t length, int flags, bool isnoundo, int* cookie,
               struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static ssize_t
recv_exec_regfile(struct ofd_tx* ofd_tx, int sockfd, void* buffer,
                  size_t length, int flags, bool isnoundo, int* cookie,
                  struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static ssize_t
recv_exec_dir(struct ofd_tx* ofd_tx, int sockfd, void* buffer,
              size_t length, int flags, bool isnoundo, int* cookie,
              struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static ssize_t
recv_exec_socket(struct ofd_tx* ofd_tx, int sockfd, void* buffer,
                 size_t length, int flags, bool isnoundo, int* cookie,
                 struct picotm_error* error)
{
    return socket_tx_recv_exec(socket_tx_of_ofd_tx(ofd_tx), sockfd, buffer,
                               length, flags, isnoundo, cookie, error);
}

ssize_t
fd_tx_recv_exec(struct fd_tx* self, int sockfd, void* buffer, size_t length,
                int flags, bool isnoundo, int* cookie,
                struct picotm_error* error)
{
    static ssize_t (* const recv_exec[])(struct ofd_tx*,
                                         int,
                                         void*,
                                         size_t,
                                         int,
                                         bool,
                                         int*,
                                         struct picotm_error*) = {
        recv_exec_chrdev,
        recv_exec_fifo,
        recv_exec_regfile,
        recv_exec_dir,
        recv_exec_socket
    };

    assert(self);

    return recv_exec[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, sockfd,
                                                     buffer, length, flags,
                                                     isnoundo, cookie, error);
}

static void
recv_apply_socket(struct ofd_tx* ofd_tx, int sockfd, int cookie,
                  struct picotm_error* error)
{
    socket_tx_recv_apply(socket_tx_of_ofd_tx(ofd_tx), sockfd, cookie, error);
}

void
fd_tx_recv_apply(struct fd_tx* self, int sockfd, int cookie,
                 struct picotm_error* error)
{
    static void (*recv_apply[])(struct ofd_tx*,
                                int,
                                int,
                                struct picotm_error*) = {
        NULL,
        NULL,
        NULL,
        NULL,
        recv_apply_socket
    };

    assert(self);

    recv_apply[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, sockfd,
                                               cookie, error);
}

static void
recv_undo_socket(struct ofd_tx* ofd_tx, int sockfd, int cookie,
                 struct picotm_error* error)
{
    socket_tx_recv_undo(socket_tx_of_ofd_tx(ofd_tx), sockfd, cookie, error);
}

void
fd_tx_recv_undo(struct fd_tx* self, int sockfd, int cookie,
                struct picotm_error* error)
{
    static void (*recv_undo[])(struct ofd_tx*,
                               int,
                               int,
                               struct picotm_error*) = {
        NULL,
        NULL,
        NULL,
        NULL,
        recv_undo_socket
    };

    assert(self);

    recv_undo[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, sockfd,
                                              cookie, error);
}

/*
 * send()
 */

static ssize_t
send_exec_chrdev(struct ofd_tx* ofd_tx, int sockfd, const void* buffer,
                 size_t length, int flags, bool isnoundo, int* cookie,
                 struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static ssize_t
send_exec_fifo(struct ofd_tx* ofd_tx, int sockfd, const void* buffer,
               size_t length, int flags, bool isnoundo, int* cookie,
               struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static ssize_t
send_exec_regfile(struct ofd_tx* ofd_tx, int sockfd, const void* buffer,
                  size_t length, int flags, bool isnoundo, int* cookie,
                  struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static ssize_t
send_exec_dir(struct ofd_tx* ofd_tx, int sockfd, const void* buffer,
              size_t length, int flags, bool isnoundo, int* cookie,
              struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static ssize_t
send_exec_socket(struct ofd_tx* ofd_tx, int sockfd, const void* buffer,
                 size_t length, int flags, bool isnoundo, int* cookie,
                 struct picotm_error* error)
{
    return socket_tx_send_exec(socket_tx_of_ofd_tx(ofd_tx), sockfd, buffer,
                               length, flags, isnoundo, cookie, error);
}

ssize_t
fd_tx_send_exec(struct fd_tx* self, int sockfd, const void* buffer,
                size_t length, int flags, bool isnoundo, int* cookie,
                struct picotm_error* error)
{
    static ssize_t (* const send_exec[])(struct ofd_tx*,
                                         int,
                                         const void*,
                                         size_t,
                                         int,
                                         bool,
                                         int*,
                                         struct picotm_error*) = {
        send_exec_chrdev,
        send_exec_fifo,
        send_exec_regfile,
        send_exec_dir,
        send_exec_socket
    };

    assert(self);

    return send_exec[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, sockfd,
                                                     buffer, length, flags,
                                                     isnoundo, cookie, error);
}

static void
send_apply_socket(struct ofd_tx* ofd_tx, int fildes, int cookie,
                  struct picotm_error* error)
{
    socket_tx_send_apply(socket_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

void
fd_tx_send_apply(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    static void (* const send_apply[])(struct ofd_tx*,
                                       int,
                                       int,
                                       struct picotm_error*) = {
        NULL,
        NULL,
        NULL,
        NULL,
        send_apply_socket
    };

    assert(self);

    send_apply[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                               cookie, error);
}

static void
send_undo_socket(struct ofd_tx* ofd_tx, int fildes, int cookie,
                 struct picotm_error* error)
{
    socket_tx_send_undo(socket_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

void
fd_tx_send_undo(struct fd_tx* self, int fildes, int cookie,
                struct picotm_error* error)
{
    static void (* const send_undo[])(struct ofd_tx*,
                                      int,
                                      int,
                                      struct picotm_error*) = {
        NULL,
        NULL,
        NULL,
        NULL,
        send_undo_socket
    };

    assert(self);

    send_undo[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                              cookie, error);
}

/*
 * shutdown()
 */

static int
shutdown_exec_chrdev(struct ofd_tx* ofd_tx, int sockfd, int how, bool isnoundo,
                     int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static int
shutdown_exec_fifo(struct ofd_tx* ofd_tx, int sockfd, int how, bool isnoundo,
                   int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static int
shutdown_exec_regfile(struct ofd_tx* ofd_tx, int sockfd, int how,
                      bool isnoundo, int* cookie,
                      struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static int
shutdown_exec_dir(struct ofd_tx* ofd_tx, int sockfd, int how,
                  bool isnoundo, int* cookie,
                  struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

static int
shutdown_exec_socket(struct ofd_tx* ofd_tx, int sockfd, int how,
                     bool isnoundo, int* cookie,
                     struct picotm_error* error)
{
    return socket_tx_shutdown_exec(socket_tx_of_ofd_tx(ofd_tx), sockfd, how,
                                   isnoundo, cookie, error);
}

int
fd_tx_shutdown_exec(struct fd_tx* self, int sockfd, int how, bool isnoundo,
                    int* cookie, struct picotm_error* error)
{
    static int (* const shutdown_exec[])(struct ofd_tx*,
                                         int,
                                         int,
                                         bool,
                                         int*,
                                         struct picotm_error*) = {
        shutdown_exec_chrdev,
        shutdown_exec_fifo,
        shutdown_exec_regfile,
        shutdown_exec_dir,
        shutdown_exec_socket
    };

    assert(self);

    return shutdown_exec[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, sockfd,
                                                         how, isnoundo,
                                                         cookie, error);
}

static void
shutdown_apply_socket(struct ofd_tx* ofd_tx, int fildes, int cookie,
                      struct picotm_error* error)
{
    socket_tx_shutdown_apply(socket_tx_of_ofd_tx(ofd_tx), fildes, cookie,
                             error);
}

void
fd_tx_shutdown_apply(struct fd_tx* self, int fildes, int cookie,
                     struct picotm_error* error)
{
    static void (* const shutdown_apply[])(struct ofd_tx*,
                                           int,
                                           int,
                                           struct picotm_error*) = {
        NULL,
        NULL,
        NULL,
        NULL,
        shutdown_apply_socket
    };

    assert(self);

    shutdown_apply[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                                   cookie, error);
}

static void
shutdown_undo_socket(struct ofd_tx* ofd_tx, int fildes, int cookie,
                     struct picotm_error* error)
{
    socket_tx_shutdown_undo(socket_tx_of_ofd_tx(ofd_tx), fildes, cookie,
                            error);
}

void
fd_tx_shutdown_undo(struct fd_tx* self, int fildes, int cookie,
                    struct picotm_error* error)
{
    static void (* const shutdown_undo[])(struct ofd_tx*,
                                          int,
                                          int,
                                          struct picotm_error*) = {
        NULL,
        NULL,
        NULL,
        NULL,
        shutdown_undo_socket
    };

    assert(self);

    shutdown_undo[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                                  cookie, error);
}

/*
 * write()
 */

static ssize_t
write_exec_chrdev(struct ofd_tx* ofd_tx, int fildes, const void* buf,
                  size_t nbyte, bool isnoundo, int* cookie,
                  struct picotm_error* error)
{
    return chrdev_tx_write_exec(chrdev_tx_of_ofd_tx(ofd_tx), fildes, buf, nbyte,
                                isnoundo, cookie, error);
}

static ssize_t
write_exec_fifo(struct ofd_tx* ofd_tx, int fildes, const void* buf,
                size_t nbyte, bool isnoundo, int* cookie,
                struct picotm_error* error)
{
    return fifo_tx_write_exec(fifo_tx_of_ofd_tx(ofd_tx), fildes, buf, nbyte,
                              isnoundo, cookie, error);
}

static ssize_t
write_exec_regfile(struct ofd_tx* ofd_tx, int fildes, const void* buf,
                   size_t nbyte, bool isnoundo, int* cookie,
                   struct picotm_error* error)
{
    return regfile_tx_write_exec(regfile_tx_of_ofd_tx(ofd_tx), fildes, buf,
                                 nbyte, isnoundo, cookie, error);
}

static ssize_t
write_exec_dir(struct ofd_tx* ofd_tx, int fildes, const void* buf,
               size_t nbyte, bool isnoundo, int* cookie,
               struct picotm_error* error)
{
    picotm_error_set_errno(error, EISDIR);
    return -1;
}

static ssize_t
write_exec_socket(struct ofd_tx* ofd_tx, int fildes, const void* buf,
                  size_t nbyte, bool isnoundo, int* cookie,
                  struct picotm_error* error)
{
    return socket_tx_write_exec(socket_tx_of_ofd_tx(ofd_tx), fildes, buf,
                                nbyte, isnoundo, cookie, error);
}

ssize_t
fd_tx_write_exec(struct fd_tx* self, int fildes, const void* buf,
                 size_t nbyte, bool isnoundo, int* cookie,
                 struct picotm_error* error)
{
    static ssize_t (* const write_exec[])(struct ofd_tx*,
                                          int,
                                          const void*,
                                          size_t,
                                          bool,
                                          int*,
                                          struct picotm_error*) = {
        write_exec_chrdev,
        write_exec_fifo,
        write_exec_regfile,
        write_exec_dir,
        write_exec_socket
    };

    assert(self);

    return write_exec[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                                      buf, nbyte, isnoundo,
                                                      cookie, error);
}

static void
write_apply_chrdev(struct ofd_tx* ofd_tx, int fildes, int cookie,
                   struct picotm_error* error)
{
    chrdev_tx_write_apply(chrdev_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

static void
write_apply_fifo(struct ofd_tx* ofd_tx, int fildes, int cookie,
                 struct picotm_error* error)
{
    fifo_tx_write_apply(fifo_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

static void
write_apply_regfile(struct ofd_tx* ofd_tx, int fildes, int cookie,
                    struct picotm_error* error)
{
    regfile_tx_write_apply(regfile_tx_of_ofd_tx(ofd_tx), fildes, cookie,
                           error);
}

static void
write_apply_socket(struct ofd_tx* ofd_tx, int fildes, int cookie,
                   struct picotm_error* error)
{
    socket_tx_write_apply(socket_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

void
fd_tx_write_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static void (* const write_apply[])(struct ofd_tx*,
                                        int,
                                        int,
                                        struct picotm_error*) = {
        write_apply_chrdev,
        write_apply_fifo,
        write_apply_regfile,
        NULL,
        write_apply_socket
    };

    assert(self);

    write_apply[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                                cookie, error);
}

static void
write_undo_chrdev(struct ofd_tx* ofd_tx, int fildes, int cookie,
                  struct picotm_error* error)
{
    chrdev_tx_write_undo(chrdev_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

static void
write_undo_fifo(struct ofd_tx* ofd_tx, int fildes, int cookie,
                struct picotm_error* error)
{
    fifo_tx_write_undo(fifo_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

static void
write_undo_regfile(struct ofd_tx* ofd_tx, int fildes, int cookie,
                   struct picotm_error* error)
{
    regfile_tx_write_undo(regfile_tx_of_ofd_tx(ofd_tx), fildes, cookie,
                          error);
}

static void
write_undo_socket(struct ofd_tx* ofd_tx, int fildes, int cookie,
                  struct picotm_error* error)
{
    socket_tx_write_undo(socket_tx_of_ofd_tx(ofd_tx), fildes, cookie, error);
}

void
fd_tx_write_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    static void (* const write_undo[])(struct ofd_tx*,
                                       int,
                                       int,
                                       struct picotm_error*) = {
        write_undo_chrdev,
        write_undo_fifo,
        write_undo_regfile,
        NULL,
        write_undo_socket
    };

    assert(self);

    write_undo[ofd_tx_file_type(self->ofd_tx)](self->ofd_tx, fildes,
                                               cookie, error);
}
