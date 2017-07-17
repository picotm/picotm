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

#include "socket_tx.h"
#include <assert.h>
#include <errno.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-ptr.h>
#include <picotm/picotm-lib-tab.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "fcntlop.h"
#include "fcntloptab.h"
#include "ioop.h"
#include "iooptab.h"

struct socket_tx*
socket_tx_of_ofd_tx(struct ofd_tx* ofd_tx)
{
    assert(ofd_tx);
    assert(ofd_tx_file_type(ofd_tx) == PICOTM_LIBC_FILE_TYPE_SOCKET);

    return picotm_containerof(ofd_tx, struct socket_tx, base);
}

static void
ref_ofd_tx(struct ofd_tx* ofd_tx)
{
    socket_tx_ref(socket_tx_of_ofd_tx(ofd_tx));
}

static void
unref_ofd_tx(struct ofd_tx* ofd_tx)
{
    socket_tx_unref(socket_tx_of_ofd_tx(ofd_tx));
}

static void
socket_tx_try_rdlock_field(struct socket_tx* self, enum socket_field field,
                           struct picotm_error* error)
{
    assert(self);

    socket_try_rdlock_field(self->socket, field, self->rwstate + field, error);
}

static void
socket_tx_try_wrlock_field(struct socket_tx* self, enum socket_field field,
                           struct picotm_error* error)
{
    assert(self);

    socket_try_wrlock_field(self->socket, field, self->rwstate + field, error);
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
                struct socket* socket)
{
    enum socket_field field = 0;

    while (beg < end) {
        socket_unlock_field(socket, field, beg);
        ++field;
        ++beg;
    }
}

void
socket_tx_init(struct socket_tx* self)
{
    assert(self);

    picotm_ref_init(&self->ref, 0);

    memset(&self->active_list, 0, sizeof(self->active_list));

    ofd_tx_init(&self->base, PICOTM_LIBC_FILE_TYPE_SOCKET,
                ref_ofd_tx, unref_ofd_tx);

    self->socket = NULL;

    self->flags = 0;

    self->wrbuf = NULL;
    self->wrbuflen = 0;
    self->wrbufsiz = 0;

    self->wrtab = NULL;
    self->wrtablen = 0;
    self->wrtabsiz = 0;

    self->rdtab = NULL;
    self->rdtablen = 0;
    self->rdtabsiz = 0;

    self->fcntltab = NULL;
    self->fcntltablen = 0;

    self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));
}

void
socket_tx_uninit(struct socket_tx* self)
{
    assert(self);

    fcntloptab_clear(&self->fcntltab, &self->fcntltablen);
    iooptab_clear(&self->wrtab, &self->wrtablen);
    iooptab_clear(&self->rdtab, &self->rdtablen);
    free(self->wrbuf);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));
}

/*
 * Validation
 */

void
socket_tx_lock(struct socket_tx* self)
{
    assert(self);
}

void
socket_tx_unlock(struct socket_tx* self)
{
    assert(self);
}

static void
validate_noundo(struct socket_tx* self, struct picotm_error* error)
{ }

static void
validate_2pl(struct socket_tx* self, struct picotm_error* error)
{ }

void
socket_tx_validate(struct socket_tx* self, struct picotm_error* error)
{
    static void (* const validate[])(struct socket_tx*, struct picotm_error*) = {
        validate_noundo,
        validate_2pl
    };

    assert(self);

    if (!socket_tx_holds_ref(self)) {
        return;
    }

    validate[self->cc_mode](self, error);
}

/*
 * Update CC
 */

static void
update_cc_noundo(struct socket_tx* self, struct picotm_error* error)
{ }

static void
update_cc_2pl(struct socket_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_2PL);

    /* release reader/writer locks on socket state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->socket);
}

void
socket_tx_update_cc(struct socket_tx* self, struct picotm_error* error)
{
    static void (* const update_cc[])(struct socket_tx*, struct picotm_error*) = {
        update_cc_noundo,
        update_cc_2pl
    };

    assert(socket_tx_holds_ref(self));

    update_cc[self->cc_mode](self, error);
}

/*
 * Clear CC
 */

static void
clear_cc_noundo(struct socket_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO);
}

static void
clear_cc_2pl(struct socket_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_2PL);

    /* release reader/writer locks on socket state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->socket);
}

void
socket_tx_clear_cc(struct socket_tx* self, struct picotm_error* error)
{
    static void (* const clear_cc[])(struct socket_tx*, struct picotm_error*) = {
        clear_cc_noundo,
        clear_cc_2pl
    };

    assert(socket_tx_holds_ref(self));

    clear_cc[self->cc_mode](self, error);
}

/*
 * Referencing
 */

void
socket_tx_ref_or_set_up(struct socket_tx* self, struct socket* socket,
                        int fildes, unsigned long flags,
                        struct picotm_error* error)
{
    assert(self);
    assert(socket);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    /* get reference on socket */
    socket_ref(socket);

    /* setup fields */

    self->socket = socket;
    self->cc_mode = socket_get_cc_mode(socket);
    self->flags = 0;

    self->fcntltablen = 0;
    self->rdtablen = 0;
    self->wrtablen = 0;
    self->wrbuflen = 0;
}

void
socket_tx_ref(struct socket_tx* self)
{
    picotm_ref_up(&self->ref);
}

void
socket_tx_unref(struct socket_tx* self)
{
    assert(self);

    bool final_ref = picotm_ref_down(&self->ref);
    if (!final_ref) {
        return;
    }

    socket_unref(self->socket);
    self->socket = NULL;
}

bool
socket_tx_holds_ref(struct socket_tx* self)
{
    assert(self);

    return picotm_ref_count(&self->ref) > 0;
}

static off_t
append_to_iobuffer(struct socket_tx* self, size_t nbyte, const void* buf,
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

int
socket_tx_append_to_writeset(struct socket_tx* self, size_t nbyte, off_t offset,
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
socket_tx_append_to_readset(struct socket_tx* self, size_t nbyte, off_t offset,
                            const void* buf, struct picotm_error* error)
{
    assert(self);

    off_t bufoffset = append_to_iobuffer(self, nbyte, buf, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    unsigned long res = iooptab_append(&self->rdtab,
                                       &self->rdtablen,
                                       &self->rdtabsiz,
                                       nbyte, offset, bufoffset,
                                       error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return res;
}

/*
 * bind()
 */

static int
bind_exec_noundo(struct socket_tx* self, int sockfd,
                 const struct sockaddr* addr, socklen_t addrlen,
                 int* cookie, struct picotm_error* error)
{
    int res = bind(sockfd, addr, addrlen);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

int
socket_tx_bind_exec(struct socket_tx* self, int sockfd,
                    const struct sockaddr* address, socklen_t addresslen,
                    bool isnoundo, int* cookie, struct picotm_error* error)
{
    static int (* const bind_exec[2])(struct socket_tx*,
                                      int,
                                      const struct sockaddr*,
                                      socklen_t,
                                      int*,
                                      struct picotm_error* error) = {
        bind_exec_noundo,
        NULL
    };

    if (isnoundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !bind_exec[self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return bind_exec[self->cc_mode](self, sockfd, address, addresslen,
                                    cookie, error);
}

static void
bind_apply_noundo(struct socket_tx* self, int sockfd, int cookie,
                  struct picotm_error* error)
{ }

void
socket_tx_bind_apply(struct socket_tx* self, int sockfd, int cookie,
                     struct picotm_error* error)
{
    static void (* const bind_apply[2])(struct socket_tx*,
                                        int,
                                        int,
                                        struct picotm_error*) = {
        bind_apply_noundo,
        NULL
    };

    assert(bind_apply[self->cc_mode]);

    bind_apply[self->cc_mode](self, sockfd, cookie, error);
}

void
socket_tx_bind_undo(struct socket_tx* self, int sockfd, int cookie,
                    struct picotm_error* error)
{
    static void (* const bind_undo[2])(int, struct picotm_error*) = {
        NULL,
        NULL
    };

    assert(self->cc_mode < sizeof(bind_undo)/sizeof(bind_undo[0]));
    assert(bind_undo[self->cc_mode]);

    bind_undo[self->cc_mode](cookie, error);
}

/*
 * connect()
 */

static int
connect_exec_noundo(struct socket_tx* self, int sockfd,
                    const struct sockaddr* address, socklen_t addresslen,
                    int* cookie, struct picotm_error* error)
{
    int res = TEMP_FAILURE_RETRY(connect(sockfd, address, addresslen));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

int
socket_tx_connect_exec(struct socket_tx* self, int sockfd,
                       const struct sockaddr* address, socklen_t addresslen,
                       bool isnoundo, int* cookie, struct picotm_error* error)
{
    static int (* const connect_exec[2])(struct socket_tx*,
                                         int,
                                         const struct sockaddr*,
                                         socklen_t,
                                         int*,
                                         struct picotm_error*) = {
        connect_exec_noundo,
        NULL
    };

    assert(self->cc_mode < sizeof(connect_exec)/sizeof(connect_exec[0]));
    assert(connect_exec[self->cc_mode]);

    if (isnoundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !connect_exec[self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return connect_exec[self->cc_mode](self, sockfd, address, addresslen,
                                       cookie, error);
}

static void
connect_apply_noundo(struct socket_tx* self, int sockfd, int cookie,
                     struct picotm_error* error)
{ }

void
socket_tx_connect_apply(struct socket_tx* self, int sockfd, int cookie,
                        struct picotm_error* error)
{
    static void (* const connect_apply[2])(struct socket_tx*,
                                           int,
                                           int,
                                           struct picotm_error*) = {
        connect_apply_noundo,
        NULL
    };

    assert(self->cc_mode < sizeof(connect_apply)/sizeof(connect_apply[0]));
    assert(connect_apply[self->cc_mode]);

    connect_apply[self->cc_mode](self, sockfd, cookie, error);
}

void
socket_tx_connect_undo(struct socket_tx* self, int sockfd, int cookie,
                       struct picotm_error* error)
{
    static void (* const connect_undo[2])(struct socket_tx*,
                                          int,
                                          int,
                                          struct picotm_error*) = {
        NULL,
        NULL
    };

    assert(self->cc_mode < sizeof(connect_undo)/sizeof(connect_undo[0]));
    assert(connect_undo[self->cc_mode]);

    connect_undo[self->cc_mode](self, sockfd, cookie, error);
}

/*
 * fcntl()
 */

static int
fcntl_exec_noundo(struct socket_tx* self, int fildes, int cmd,
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
fcntl_exec_2pl(struct socket_tx* self, int fildes, int cmd,
               union fcntl_arg* arg, int* cookie, struct picotm_error* error)
{
    assert(arg);

    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN: {

            /* Read-lock socket */
            socket_tx_try_rdlock_field(self, SOCKET_FIELD_STATE, error);
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

            /* Read-lock open file description */
            socket_tx_try_rdlock_field(self, SOCKET_FIELD_STATE, error);
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

int
socket_tx_fcntl_exec(struct socket_tx* self, int fildes, int cmd,
                  union fcntl_arg* arg, bool isnoundo, int* cookie,
                  struct picotm_error* error)
{
    static int (* const fcntl_exec[2])(struct socket_tx*,
                                       int,
                                       int,
                                       union fcntl_arg*,
                                       int*,
                                       struct picotm_error*) = {
        fcntl_exec_noundo,
        fcntl_exec_2pl
    };

    assert(self->cc_mode < sizeof(fcntl_exec)/sizeof(fcntl_exec[0]));
    assert(fcntl_exec[self->cc_mode]);

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

void
socket_tx_fcntl_apply(struct socket_tx* self, int fildes, int cookie,
                      struct picotm_error* error)
{ }

void
socket_tx_fcntl_undo(struct socket_tx* self, int fildes, int cookie,
                     struct picotm_error* error)
{ }

/*
 * listen()
 */

static int
listen_exec_noundo(struct socket_tx* self, int sockfd, int backlog,
                   int* cookie, struct picotm_error* error)
{
    int res = listen(sockfd, backlog);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

int
socket_tx_listen_exec(struct socket_tx* self, int sockfd, int backlog,
                      bool isnoundo, int* cookie, struct picotm_error* error)
{
    static int (* const listen_exec[2])(struct socket_tx*,
                                        int,
                                        int,
                                        int*,
                                        struct picotm_error*) = {
        listen_exec_noundo,
        NULL
    };

    assert(self->cc_mode < sizeof(listen_exec)/sizeof(listen_exec[0]));
    assert(listen_exec[self->cc_mode]);

    if (isnoundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !listen_exec[self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return listen_exec[self->cc_mode](self, sockfd, backlog, cookie, error);
}

static void
listen_apply_noundo(struct picotm_error* error)
{ }

void
socket_tx_listen_apply(struct socket_tx* self, int sockfd, int cookie,
                       struct picotm_error* error)
{
    static void (* const listen_apply[2])(struct picotm_error*) = {
        listen_apply_noundo,
        NULL
    };

    assert(self->cc_mode < sizeof(listen_apply)/sizeof(listen_apply[0]));
    assert(listen_apply[self->cc_mode]);

    listen_apply[self->cc_mode](error);
}

void
socket_tx_listen_undo(struct socket_tx* self, int sockfd, int cookie,
                      struct picotm_error* error)
{
    static int (* const listen_undo[2])(struct socket_tx*,
                                        int,
                                        int,
                                        struct picotm_error*) = {
        NULL,
        NULL
    };

    assert(self->cc_mode < sizeof(listen_undo)/sizeof(listen_undo[0]));
    assert(listen_undo[self->cc_mode]);

    listen_undo[self->cc_mode](self, sockfd, cookie, error);
}

/*
 * read()
 */

static ssize_t
read_exec_noundo(struct socket_tx* self, int fildes, void* buf, size_t nbyte,
                 enum picotm_libc_validation_mode val_mode, int* cookie,
                 struct picotm_error* error)
{
    ssize_t res = TEMP_FAILURE_RETRY(read(fildes, buf, nbyte));
    if ((res < 0) && (errno != EAGAIN) && (errno != EWOULDBLOCK)) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

ssize_t
socket_tx_read_exec(struct socket_tx* self, int fildes, void* buf,
                    size_t nbyte, bool isnoundo,
                    enum picotm_libc_validation_mode val_mode,
                    int* cookie, struct picotm_error* error)
{
    static ssize_t (* const read_exec[2])(struct socket_tx*,
                                          int,
                                          void*,
                                          size_t,
                                          enum picotm_libc_validation_mode,
                                          int*,
                                          struct picotm_error*) = {
        read_exec_noundo,
        NULL
    };

    assert(self->cc_mode < sizeof(read_exec)/sizeof(read_exec[0]));
    assert(read_exec[self->cc_mode]);

    if (isnoundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !read_exec[self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return read_exec[self->cc_mode](self, fildes, buf, nbyte, val_mode,
                                    cookie, error);
}

static void
read_apply_noundo(struct socket_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{ }

void
socket_tx_read_apply(struct socket_tx* self, int fildes, int cookie,
                     struct picotm_error* error)
{
    static void (* const read_apply[2])(struct socket_tx*,
                                        int,
                                        int,
                                        struct picotm_error*) = {
        read_apply_noundo,
        NULL
    };

    assert(self->cc_mode < sizeof(read_apply)/sizeof(read_apply[0]));
    assert(read_apply[self->cc_mode]);

    read_apply[self->cc_mode](self, fildes, cookie, error);
}

void
socket_tx_read_undo(struct socket_tx* self, int fildes, int cookie,
                    struct picotm_error* error)
{
    static void (* const read_undo[2])(struct picotm_error*) = {
        NULL,
        NULL
    };

    assert(self->cc_mode < sizeof(read_undo)/sizeof(read_undo[0]));
    assert(read_undo[self->cc_mode]);

    read_undo[self->cc_mode](error);
}

/*
 * recv()
 */

static ssize_t
recv_exec_noundo(struct socket_tx* self, int sockfd, void* buffer,
                 size_t length, int flags, int* cookie,
                 struct picotm_error* error)
{
    ssize_t res = TEMP_FAILURE_RETRY(recv(sockfd, buffer, length, flags));
    if ((res < 0) && (errno != EAGAIN) && (errno != EWOULDBLOCK)) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

ssize_t
socket_tx_recv_exec(struct socket_tx* self, int sockfd, void* buffer,
                    size_t length, int flags, bool isnoundo, int* cookie,
                    struct picotm_error* error)
{
    static ssize_t (* const recv_exec[2])(struct socket_tx*,
                                          int,
                                          void*,
                                          size_t,
                                          int,
                                          int*,
                                          struct picotm_error*) = {
        recv_exec_noundo,
        NULL
    };

    assert(self->cc_mode < sizeof(recv_exec)/sizeof(recv_exec[0]));
    assert(recv_exec[self->cc_mode]);

    if (isnoundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !recv_exec[self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return recv_exec[self->cc_mode](self, sockfd, buffer, length, flags,
                                    cookie, error);
}

static void
recv_apply_noundo(struct picotm_error* error)
{ }

void
socket_tx_recv_apply(struct socket_tx* self, int sockfd, int cookie,
                     struct picotm_error* error)
{
    static void (* const recv_apply[2])(struct picotm_error*) = {
        recv_apply_noundo,
        NULL
    };

    assert(self->cc_mode < sizeof(recv_apply)/sizeof(recv_apply[0]));
    assert(recv_apply[self->cc_mode]);

    recv_apply[self->cc_mode](error);
}

void
socket_tx_recv_undo(struct socket_tx* self, int sockfd, int cookie,
                    struct picotm_error* error)
{
    static void (* const recv_undo[2])(struct socket_tx*,
                                       int,
                                       int,
                                       struct picotm_error*) = {
        NULL,
        NULL
    };

    assert(self->cc_mode < sizeof(recv_undo)/sizeof(recv_undo[0]));
    assert(recv_undo[self->cc_mode]);

    recv_undo[self->cc_mode](self, sockfd, cookie, error);
}

/*
 * send()
 */

static ssize_t
send_exec_noundo(struct socket_tx* self, int sockfd, const void* buffer,
                 size_t length, int flags, int* cookie,
                 struct picotm_error* error)
{
    ssize_t res = TEMP_FAILURE_RETRY(send(sockfd, buffer, length, flags));
    if ((res < 0) && (errno != EAGAIN) && (errno != EWOULDBLOCK)) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

static ssize_t
send_exec_2pl(struct socket_tx* self, int sockfd, const void* buf,
              size_t nbyte, int flags, int* cookie,
              struct picotm_error* error)
{
    /* Become irrevocable if any flags are selected */
    if (flags) {
        picotm_error_set_revocable(error);
        return -1;
    }

    /* Write-lock socket */
    socket_tx_try_wrlock_field(self, SOCKET_FIELD_STATE, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Register write data */

    if (cookie) {
        *cookie = socket_tx_append_to_writeset(self, nbyte, 0, buf, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return nbyte;
}

ssize_t
socket_tx_send_exec(struct socket_tx* self, int sockfd, const void* buffer,
                    size_t length, int flags, bool isnoundo, int* cookie,
                    struct picotm_error* error)
{
    static ssize_t (* const send_exec[2])(struct socket_tx*,
                                          int,
                                          const void*,
                                          size_t,
                                          int,
                                          int*,
                                          struct picotm_error*) = {
        send_exec_noundo,
        send_exec_2pl
    };

    assert(self->cc_mode < sizeof(send_exec)/sizeof(send_exec[0]));
    assert(send_exec[self->cc_mode]);

    if (isnoundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !send_exec[self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return send_exec[self->cc_mode](self, sockfd, buffer, length, flags,
                                    cookie, error);
}

static void
send_apply_noundo(struct socket_tx* self, int sockfd, int cookie,
                  struct picotm_error* error)
{ }

static void
send_apply_2pl(struct socket_tx* self, int sockfd, int cookie,
               struct picotm_error* error)
{
    assert(self);
    assert(sockfd >= 0);

    /* FIXME: Use select() to prevent blocking? */

    const ssize_t len =
        TEMP_FAILURE_RETRY(send(sockfd,
                                self->wrbuf+self->wrtab[cookie].bufoff,
                                self->wrtab[cookie].nbyte, 0));

    if (len < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

void
socket_tx_send_apply(struct socket_tx* self, int sockfd, int cookie,
                     struct picotm_error* error)
{
    static void (* const send_apply[2])(struct socket_tx*,
                                        int,
                                        int,
                                        struct picotm_error*) = {
        send_apply_noundo,
        send_apply_2pl
    };

    assert(self->cc_mode < sizeof(send_apply)/sizeof(send_apply[0]));
    assert(send_apply[self->cc_mode]);

    send_apply[self->cc_mode](self, sockfd, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
send_undo_2pl(struct picotm_error* error)
{ }

void
socket_tx_send_undo(struct socket_tx* self, int sockfd, int cookie,
                    struct picotm_error* error)
{
    static void (* const send_undo[2])(struct picotm_error*) = {
        NULL,
        send_undo_2pl
    };

    assert(self->cc_mode < sizeof(send_undo)/sizeof(send_undo[0]));
    assert(send_undo[self->cc_mode]);

    send_undo[self->cc_mode](error);
}

/*
 * shutdown()
 */

static int
shutdown_exec_noundo(struct socket_tx* self, int sockfd, int how,
                     int* cookie, struct picotm_error* error)
{
    int res = shutdown(sockfd, how);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

int
socket_tx_shutdown_exec(struct socket_tx* self, int sockfd, int how,
                        bool isnoundo, int* cookie,
                        struct picotm_error* error)
{
    static int (* const shutdown_exec[2])(struct socket_tx*,
                                          int,
                                          int,
                                          int*,
                                          struct picotm_error*) = {
        shutdown_exec_noundo,
        NULL
    };

    assert(self->cc_mode < sizeof(shutdown_exec)/sizeof(shutdown_exec[0]));
    assert(shutdown_exec[self->cc_mode]);

    if (isnoundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !shutdown_exec[self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return shutdown_exec[self->cc_mode](self, sockfd, how, cookie, error);
}

static void
shutdown_apply_noundo(struct socket_tx* self, int sockfd, int cookie,
                      struct picotm_error* error)
{ }

void
socket_tx_shutdown_apply(struct socket_tx* self, int sockfd, int cookie,
                         struct picotm_error* error)
{
    static void (* const shutdown_apply[2])(struct socket_tx*,
                                            int,
                                            int,
                                            struct picotm_error*) = {
        shutdown_apply_noundo,
        NULL
    };

    assert(self->cc_mode < sizeof(shutdown_apply)/sizeof(shutdown_apply[0]));
    assert(shutdown_apply[self->cc_mode]);

    shutdown_apply[self->cc_mode](self, sockfd, cookie, error);
}

void
socket_tx_shutdown_undo(struct socket_tx* self, int sockfd, int cookie,
                        struct picotm_error* error)
{
    static int (* const shutdown_undo[2])(struct socket_tx*,
                                          int,
                                          int,
                                          struct picotm_error*) = {
        NULL,
        NULL
    };

    assert(self->cc_mode < sizeof(shutdown_undo)/sizeof(shutdown_undo[0]));
    assert(shutdown_undo[self->cc_mode]);

    shutdown_undo[self->cc_mode](self, sockfd, cookie, error);
}

/*
 * write()
 */

static ssize_t
write_exec_noundo(struct socket_tx* self, int fildes, const void* buf,
                  size_t nbyte, int* cookie, struct picotm_error* error)
{
    ssize_t res = TEMP_FAILURE_RETRY(write(fildes, buf, nbyte));
    if ((res < 0) && (errno != EAGAIN) && (errno != EWOULDBLOCK)) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

static ssize_t
write_exec_2pl(struct socket_tx* self, int fildes, const void* buf,
               size_t nbyte, int* cookie, struct picotm_error* error)
{
    /* Write-lock socket */
    socket_tx_try_wrlock_field(self, SOCKET_FIELD_STATE, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Register write data */

    if (cookie) {
        *cookie = socket_tx_append_to_writeset(self, nbyte, 0, buf, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return nbyte;
}

ssize_t
socket_tx_write_exec(struct socket_tx* self, int fildes, const void* buf,
                     size_t nbyte, bool isnoundo, int* cookie,
                     struct picotm_error* error)
{
    static ssize_t (* const write_exec[2])(struct socket_tx*,
                                           int,
                                           const void*,
                                           size_t,
                                           int*,
                                           struct picotm_error*) = {
        write_exec_noundo,
        write_exec_2pl
    };

    assert(self->cc_mode < sizeof(write_exec)/sizeof(write_exec[0]));
    assert(write_exec[self->cc_mode]);

    if (isnoundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !write_exec[self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return write_exec[self->cc_mode](self, fildes, buf, nbyte, cookie, error);
}

static void
write_apply_noundo(struct socket_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{ }

static void
write_apply_2pl(struct socket_tx* self, int sockfd, int cookie,
                struct picotm_error* error)
{
    assert(self);
    assert(sockfd >= 0);

    /* FIXME: Use select() to prevent blocking? */

    const ssize_t len =
        TEMP_FAILURE_RETRY(write(sockfd,
                                 self->wrbuf+self->wrtab[cookie].bufoff,
                                 self->wrtab[cookie].nbyte));
    if (len < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

void
socket_tx_write_apply(struct socket_tx* self, int fildes, int cookie,
                      struct picotm_error* error)
{
    static void (* const write_apply[2])(struct socket_tx*,
                                         int,
                                         int,
                                         struct picotm_error*) = {
        write_apply_noundo,
        write_apply_2pl
    };

    assert(self->cc_mode < sizeof(write_apply)/sizeof(write_apply[0]));
    assert(write_apply[self->cc_mode]);

    write_apply[self->cc_mode](self, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
write_any_undo(struct picotm_error* error)
{ }

void
socket_tx_write_undo(struct socket_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static void (* const write_undo[2])(struct picotm_error*) = {
        NULL,
        write_any_undo
    };

    assert(self->cc_mode < sizeof(write_undo)/sizeof(write_undo[0]));
    assert(write_undo[self->cc_mode]);

    write_undo[self->cc_mode](error);
}
