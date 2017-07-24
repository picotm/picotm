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

#include "chrdev_tx.h"
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
#include "fcntlop.h"
#include "fcntloptab.h"
#include "ioop.h"
#include "iooptab.h"

struct chrdev_tx*
chrdev_tx_of_file_tx(struct file_tx* file_tx)
{
    assert(file_tx);
    assert(file_tx_file_type(file_tx) == PICOTM_LIBC_FILE_TYPE_CHRDEV);

    return picotm_containerof(file_tx, struct chrdev_tx, base);
}

static void
ref_file_tx(struct file_tx* file_tx)
{
    chrdev_tx_ref(chrdev_tx_of_file_tx(file_tx));
}

static void
unref_file_tx(struct file_tx* file_tx)
{
    chrdev_tx_unref(chrdev_tx_of_file_tx(file_tx));
}

static void
chrdev_tx_try_rdlock_field(struct chrdev_tx* self, enum chrdev_field field,
                           struct picotm_error* error)
{
    assert(self);

    chrdev_try_rdlock_field(self->chrdev, field, self->rwstate + field, error);
}

static void
chrdev_tx_try_wrlock_field(struct chrdev_tx* self, enum chrdev_field field,
                           struct picotm_error* error)
{
    assert(self);

    chrdev_try_wrlock_field(self->chrdev, field, self->rwstate + field, error);
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
                struct chrdev* chrdev)
{
    enum chrdev_field field = 0;

    while (beg < end) {
        chrdev_unlock_field(chrdev, field, beg);
        ++field;
        ++beg;
    }
}

static off_t
append_to_iobuffer(struct chrdev_tx* self, size_t nbyte, const void* buf,
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
chrdev_tx_append_to_writeset(struct chrdev_tx* self, size_t nbyte, off_t offset,
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
chrdev_tx_append_to_readset(struct chrdev_tx* self, size_t nbyte, off_t offset,
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
 * accept()
 */

static int
accept_exec(struct file_tx* base, int sockfd, struct sockaddr* address,
            socklen_t* address_len, bool isnoundo, int* cookie,
            struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

/*
 * bind()
 */

static int
bind_exec(struct file_tx* base, int sockfd, const struct sockaddr* address,
          socklen_t addresslen, bool isnoundo, int* cookie,
          struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

/*
 * connect()
 */

static int
connect_exec(struct file_tx* base, int sockfd,
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
fchmod_exec(struct file_tx* base, int fildes, mode_t mode, bool isnoundo,
            int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, EINVAL);
    return -1;
}

/*
 * fcntl()
 */

static int
fcntl_exec_noundo(struct chrdev_tx* self, int fildes, int cmd,
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
fcntl_exec_2pl(struct chrdev_tx* self, int fildes, int cmd,
               union fcntl_arg* arg, int* cookie, struct picotm_error* error)
{
    assert(arg);

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
            chrdev_tx_try_rdlock_field(self, CHRDEV_FIELD_STATE, error);
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
chrdev_tx_fcntl_exec(struct chrdev_tx* self, int fildes, int cmd,
                     union fcntl_arg* arg, bool isnoundo, int* cookie,
                     struct picotm_error* error)
{
    static int (* const fcntl_exec[2])(struct chrdev_tx*,
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
fcntl_exec(struct file_tx* base, int fildes, int cmd,
           union fcntl_arg* arg, bool isnoundo, int* cookie,
           struct picotm_error* error)
{
    return chrdev_tx_fcntl_exec(chrdev_tx_of_file_tx(base), fildes, cmd, arg,
                                isnoundo, cookie, error);
}

void
chrdev_tx_fcntl_apply(struct chrdev_tx* self, int fildes, int cookie,
                      struct picotm_error* error)
{ }

static void
fcntl_apply(struct file_tx* base, int fildes, int cookie,
            struct picotm_error* error)
{
    chrdev_tx_fcntl_apply(chrdev_tx_of_file_tx(base), fildes, cookie, error);
}

void
chrdev_tx_fcntl_undo(struct chrdev_tx* self, int fildes, int cookie,
                     struct picotm_error* error)
{ }

static void
fcntl_undo(struct file_tx* base, int fildes, int cookie,
           struct picotm_error* error)
{
    chrdev_tx_fcntl_undo(chrdev_tx_of_file_tx(base), fildes, cookie, error);
}

/*
 * fstat()
 */

static int
fstat_exec_noundo(struct chrdev_tx* self, int fildes, struct stat* buf,
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
fstat_exec_2pl(struct chrdev_tx* self, int fildes, struct stat* buf,
               int* cookie, struct picotm_error* error)
{
    assert(self);
    assert(buf);

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

int
chrdev_tx_fstat_exec(struct chrdev_tx* self, int fildes, struct stat* buf,
                     bool isnoundo, int* cookie, struct picotm_error* error)
{
    static int (* const fstat_exec[2])(struct chrdev_tx*,
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
fstat_exec(struct file_tx* base, int fildes, struct stat* buf, bool isnoundo,
           int* cookie, struct picotm_error* error)
{
    return chrdev_tx_fstat_exec(chrdev_tx_of_file_tx(base), fildes, buf,
                                isnoundo, cookie, error);
}

static void
fstat_apply_noundo(struct chrdev_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{ }

static void
fstat_apply_2pl(struct chrdev_tx* self, int fildes, int cookie,
                struct picotm_error* error)
{ }

void
chrdev_tx_fstat_apply(struct chrdev_tx* self, int fildes, int cookie,
                      struct picotm_error* error)
{
    static void (* const fstat_apply[2])(struct chrdev_tx*,
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
fstat_apply(struct file_tx* base, int fildes, int cookie,
            struct picotm_error* error)
{
    chrdev_tx_fstat_apply(chrdev_tx_of_file_tx(base), fildes, cookie, error);
}

static void
fstat_undo_2pl(struct chrdev_tx* self, int fildes, int cookie,
                struct picotm_error* error)
{ }

void
chrdev_tx_fstat_undo(struct chrdev_tx* self, int fildes, int cookie,
                     struct picotm_error* error)
{
    static void (* const fstat_undo[2])(struct chrdev_tx*,
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
fstat_undo(struct file_tx* base, int fildes, int cookie,
           struct picotm_error* error)
{
    chrdev_tx_fstat_undo(chrdev_tx_of_file_tx(base), fildes, cookie, error);
}

/*
 * fsync()
 */

static int
fsync_exec(struct file_tx* base, int fildes, bool isnoundo, int* cookie,
           struct picotm_error* error)
{
    picotm_error_set_errno(error, EINVAL);
    return -1;
}

/*
 * listen()
 */

static int
listen_exec(struct file_tx* base, int sockfd, int backlog, bool isnoundo,
            int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

/*
 * lseek()
 */

static off_t
lseek_exec(struct file_tx* base, int fildes, off_t offset, int whence,
           bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return (off_t)-1;
}

/*
 * pread()
 */

static ssize_t
pread_exec(struct file_tx* base, int fildes, void* buf, size_t nbyte,
           off_t off, bool isnoundo,
           enum picotm_libc_validation_mode val_mode, int* cookie,
           struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return -1;
}

/*
 * pwrite()
 */

static ssize_t
pwrite_exec(struct file_tx* base, int fildes, const void* buf,
            size_t nbyte, off_t off, bool isnoundo, int* cookie,
            struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return -1;
}

/*
 * read()
 */

static ssize_t
read_exec_noundo(struct chrdev_tx* self, int fildes, void* buf,
                 size_t nbyte, enum picotm_libc_validation_mode val_mode,
                 int* cookie, struct picotm_error* error)
{
    ssize_t res = TEMP_FAILURE_RETRY(read(fildes, buf, nbyte));
    if ((res < 0) && (errno != EAGAIN) && (errno != EWOULDBLOCK)) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

ssize_t
chrdev_tx_read_exec(struct chrdev_tx* self, int fildes, void* buf, size_t nbyte,
                    bool isnoundo, enum picotm_libc_validation_mode val_mode,
                    int* cookie, struct picotm_error* error)
{
    static ssize_t (* const read_exec[2])(struct chrdev_tx*,
                                          int,
                                          void*,
                                          size_t,
                                          enum picotm_libc_validation_mode,
                                          int*,
                                          struct picotm_error*) = {
        read_exec_noundo,
        NULL
    };

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

static ssize_t
read_exec(struct file_tx* base, int fildes, void* buf, size_t nbyte,
          bool isnoundo, enum picotm_libc_validation_mode val_mode,
          int* cookie, struct picotm_error* error)
{
    return chrdev_tx_read_exec(chrdev_tx_of_file_tx(base), fildes, buf, nbyte,
                               isnoundo, val_mode, cookie, error);
}

static void
read_apply_noundo(struct chrdev_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{ }

void
chrdev_tx_read_apply(struct chrdev_tx* self, int fildes, int cookie,
                     struct picotm_error* error)
{
    static void (* const read_apply[2])(struct chrdev_tx*,
                                        int,
                                        int,
                                        struct picotm_error*) = {
        read_apply_noundo,
        NULL
    };

    read_apply[self->cc_mode](self, fildes, cookie, error);
}

static void
read_apply(struct file_tx* base, int fildes, int cookie,
           struct picotm_error* error)
{
    chrdev_tx_read_apply(chrdev_tx_of_file_tx(base), fildes, cookie, error);
}

void
chrdev_tx_read_undo(struct chrdev_tx* self, int fildes, int cookie,
                    struct picotm_error* error)
{
    static void (* const read_undo[2])(struct picotm_error*) = {
        NULL,
        NULL
    };

    assert(read_undo[self->cc_mode]);

    read_undo[self->cc_mode](error);
}

static void
read_undo(struct file_tx* base, int fildes, int cookie,
          struct picotm_error* error)
{
    chrdev_tx_read_undo(chrdev_tx_of_file_tx(base), fildes, cookie, error);
}

/*
 * recv()
 */

static ssize_t
recv_exec(struct file_tx* base, int sockfd, void* buffer, size_t length,
          int flags, bool isnoundo, int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

/*
 * send()
 */

static ssize_t
send_exec(struct file_tx* base, int sockfd, const void* buffer,
          size_t length, int flags, bool isnoundo, int* cookie,
          struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

/*
 * shutdown()
 */

static int
shutdown_exec(struct file_tx* base, int sockfd, int how, bool isnoundo,
              int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

/*
 * write()
 */

static ssize_t
write_exec_noundo(struct chrdev_tx* self, int fildes, const void* buf,
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
write_exec_2pl(struct chrdev_tx* self, int fildes, const void* buf,
               size_t nbyte, int* cookie, struct picotm_error* error)
{
    /* Write-lock character device, because we change the file position */
    chrdev_tx_try_wrlock_field(self, CHRDEV_FIELD_STATE, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Register write data */

    if (cookie) {
        *cookie = chrdev_tx_append_to_writeset(self, nbyte, 0, buf, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return nbyte;
}

ssize_t
chrdev_tx_write_exec(struct chrdev_tx* self, int fildes, const void* buf,
                     size_t nbyte, bool isnoundo, int* cookie,
                     struct picotm_error* error)
{
    static ssize_t (* const write_exec[2])(struct chrdev_tx*,
                                           int,
                                           const void*,
                                           size_t,
                                           int*,
                                           struct picotm_error*) = {
        write_exec_noundo,
        write_exec_2pl
    };

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

static ssize_t
write_exec(struct file_tx* base, int fildes, const void* buf, size_t nbyte,
           bool isnoundo, int* cookie, struct picotm_error* error)
{
    return chrdev_tx_write_exec(chrdev_tx_of_file_tx(base), fildes, buf,
                                nbyte, isnoundo, cookie, error);
}

static void
write_apply_noundo(struct chrdev_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{ }

static void
write_apply_2pl(struct chrdev_tx* self, int fildes, int cookie,
                struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);

    /* FIXME: Use select() to prevent blocking? */

    const ssize_t len =
        TEMP_FAILURE_RETRY(write(fildes,
                                 self->wrbuf+self->wrtab[cookie].bufoff,
                                 self->wrtab[cookie].nbyte));
    if (len < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

void
chrdev_tx_write_apply(struct chrdev_tx* self, int fildes, int cookie,
                      struct picotm_error* error)
{
    static void (* const write_apply[2])(struct chrdev_tx*,
                                         int,
                                         int,
                                         struct picotm_error*) = {
        write_apply_noundo,
        write_apply_2pl
    };

    write_apply[self->cc_mode](self, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
write_apply(struct file_tx* base, int fildes, int cookie,
            struct picotm_error* error)
{
    chrdev_tx_write_apply(chrdev_tx_of_file_tx(base), fildes, cookie, error);
}

static void
write_any_undo(struct picotm_error* error)
{ }

void
chrdev_tx_write_undo(struct chrdev_tx* self, int fildes, int cookie,
                     struct picotm_error* error)
{
    static void (* const write_undo[2])(struct picotm_error*) = {
        NULL,
        write_any_undo
    };

    write_undo[self->cc_mode](error);
}

static void
write_undo(struct file_tx* base, int fildes, int cookie,
           struct picotm_error* error)
{
    chrdev_tx_write_undo(chrdev_tx_of_file_tx(base), fildes, cookie, error);
}

/*
 * Public interface
 */

static const struct file_tx_ops chrdev_tx_ops = {
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
    NULL,
    NULL,
    fcntl_exec,
    fcntl_apply,
    fcntl_undo,
    fstat_exec,
    fstat_apply,
    fstat_undo,
    fsync_exec,
    NULL,
    NULL,
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
    read_apply,
    read_undo,
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
    write_apply,
    write_undo
};

void
chrdev_tx_init(struct chrdev_tx* self)
{
    assert(self);

    picotm_ref_init(&self->ref, 0);

    memset(&self->active_list, 0, sizeof(self->active_list));

    file_tx_init(&self->base, PICOTM_LIBC_FILE_TYPE_CHRDEV, &chrdev_tx_ops,
                 ref_file_tx, unref_file_tx);

    self->chrdev = NULL;

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
chrdev_tx_uninit(struct chrdev_tx* self)
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
chrdev_tx_lock(struct chrdev_tx* self)
{
    assert(self);
}

void
chrdev_tx_unlock(struct chrdev_tx* self)
{
    assert(self);
}

static void
validate_noundo(struct chrdev_tx* self, struct picotm_error* error)
{ }

static void
validate_2pl(struct chrdev_tx* self, struct picotm_error* error)
{
    assert(self);
}

void
chrdev_tx_validate(struct chrdev_tx* self, struct picotm_error* error)
{
    static void (* const validate[])(struct chrdev_tx*, struct picotm_error*) = {
        validate_noundo,
        validate_2pl
    };

    if (!chrdev_tx_holds_ref(self)) {
        return;
    }

    validate[self->cc_mode](self, error);
}

/*
 * Update CC
 */

static void
update_cc_noundo(struct chrdev_tx* self, struct picotm_error* error)
{ }

static void
update_cc_2pl(struct chrdev_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_2PL);

    /* release reader/writer locks on character-device state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->chrdev);
}

void
chrdev_tx_update_cc(struct chrdev_tx* self, struct picotm_error* error)
{
    static void (* const update_cc[])(struct chrdev_tx*, struct picotm_error*) = {
        update_cc_noundo,
        update_cc_2pl
    };

    assert(chrdev_tx_holds_ref(self));

    update_cc[self->cc_mode](self, error);
}

/*
 * Clear CC
 */

static void
clear_cc_noundo(struct chrdev_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO);
}

static void
clear_cc_2pl(struct chrdev_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_2PL);

    /* release reader/writer locks on character-device state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->chrdev);
}

void
chrdev_tx_clear_cc(struct chrdev_tx* self, struct picotm_error* error)
{
    static void (* const clear_cc[])(struct chrdev_tx*, struct picotm_error*) = {
        clear_cc_noundo,
        clear_cc_2pl
    };

    assert(chrdev_tx_holds_ref(self));

    clear_cc[self->cc_mode](self, error);
}

/*
 * Referencing
 */

void
chrdev_tx_ref_or_set_up(struct chrdev_tx* self, struct chrdev* chrdev,
                        int fildes, struct picotm_error* error)
{
    assert(self);
    assert(chrdev);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    /* acquire reference on character device */
    chrdev_ref(chrdev);

    /* setup fields */

    self->chrdev = chrdev;
    self->cc_mode = chrdev_get_cc_mode(chrdev);

    self->fcntltablen = 0;
    self->rdtablen = 0;
    self->wrtablen = 0;
    self->wrbuflen = 0;
}

void
chrdev_tx_ref(struct chrdev_tx* self)
{
    picotm_ref_up(&self->ref);
}

void
chrdev_tx_unref(struct chrdev_tx* self)
{
    assert(self);

    bool final_ref = picotm_ref_down(&self->ref);
    if (!final_ref) {
        return;
    }

    chrdev_unref(self->chrdev);
    self->chrdev = NULL;
}

bool
chrdev_tx_holds_ref(struct chrdev_tx* self)
{
    assert(self);

    return picotm_ref_count(&self->ref) > 0;
}
