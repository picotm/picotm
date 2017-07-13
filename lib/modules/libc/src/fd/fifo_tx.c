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

#include "fifo_tx.h"
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

struct fifo_tx*
fifo_tx_of_ofd_tx(struct ofd_tx* ofd_tx)
{
    assert(ofd_tx);
    assert(ofd_tx_file_type(ofd_tx) == PICOTM_LIBC_FILE_TYPE_FIFO);

    return picotm_containerof(ofd_tx, struct fifo_tx, base);
}

static void
ref_ofd_tx(struct ofd_tx* ofd_tx)
{
    fifo_tx_ref(fifo_tx_of_ofd_tx(ofd_tx));
}

static void
unref_ofd_tx(struct ofd_tx* ofd_tx)
{
    fifo_tx_unref(fifo_tx_of_ofd_tx(ofd_tx));
}

static void
fifo_tx_try_rdlock_field(struct fifo_tx* self, enum fifo_field field,
                         struct picotm_error* error)
{
    assert(self);

    fifo_try_rdlock_field(self->fifo, field, self->rwstate + field, error);
}

static void
fifo_tx_try_wrlock_field(struct fifo_tx* self, enum fifo_field field,
                         struct picotm_error* error)
{
    assert(self);

    fifo_try_wrlock_field(self->fifo, field, self->rwstate + field, error);
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
                struct fifo* fifo)
{
    enum fifo_field field = 0;

    while (beg < end) {
        fifo_unlock_field(fifo, field, beg);
        ++field;
        ++beg;
    }
}

void
fifo_tx_init(struct fifo_tx* self)
{
    assert(self);

    picotm_ref_init(&self->ref, 0);

    memset(&self->active_list, 0, sizeof(self->active_list));

    ofd_tx_init(&self->base, PICOTM_LIBC_FILE_TYPE_FIFO,
                ref_ofd_tx, unref_ofd_tx);

    self->fifo = NULL;

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
fifo_tx_uninit(struct fifo_tx* self)
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
fifo_tx_lock(struct fifo_tx* self)
{
    assert(self);
}

void
fifo_tx_unlock(struct fifo_tx* self)
{
    assert(self);
}

static void
validate_noundo(struct fifo_tx* self, struct picotm_error* error)
{ }

static void
validate_2pl(struct fifo_tx* self, struct picotm_error* error)
{
    assert(self);
}

void
fifo_tx_validate(struct fifo_tx* self, struct picotm_error* error)
{
    static void (* const validate[])(struct fifo_tx*, struct picotm_error*) = {
        validate_noundo,
        validate_2pl
    };

    if (!fifo_tx_holds_ref(self)) {
        return;
    }

    validate[self->cc_mode](self, error);
}

/*
 * Update CC
 */

static void
update_cc_noundo(struct fifo_tx* self, struct picotm_error* error)
{ }

static void
update_cc_2pl(struct fifo_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_2PL);

    /* release reader/writer locks on FIFO state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->fifo);
}

void
fifo_tx_update_cc(struct fifo_tx* self, struct picotm_error* error)
{
    static void (* const update_cc[])(struct fifo_tx*, struct picotm_error*) = {
        update_cc_noundo,
        update_cc_2pl
    };

    assert(fifo_tx_holds_ref(self));

    update_cc[self->cc_mode](self, error);
}

/*
 * Clear CC
 */

static void
clear_cc_noundo(struct fifo_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO);
}

static void
clear_cc_2pl(struct fifo_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_2PL);

    /* release reader/writer locks on FIFO state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->fifo);
}

void
fifo_tx_clear_cc(struct fifo_tx* self, struct picotm_error* error)
{
    static void (* const clear_cc[])(struct fifo_tx*, struct picotm_error*) = {
        clear_cc_noundo,
        clear_cc_2pl
    };

    assert(fifo_tx_holds_ref(self));

    clear_cc[self->cc_mode](self, error);
}

/*
 * Referencing
 */

void
fifo_tx_ref_or_set_up(struct fifo_tx* self, struct fifo* fifo, int fildes,
                     unsigned long flags, struct picotm_error* error)
{
    assert(self);
    assert(fifo);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    /* get reference on FIFO */
    fifo_ref(fifo);

    /* setup fields */

    self->fifo = fifo;
    self->cc_mode = fifo_get_cc_mode(fifo);
    self->flags = 0;

    self->fcntltablen = 0;
    self->rdtablen = 0;
    self->wrtablen = 0;
    self->wrbuflen = 0;
}

void
fifo_tx_ref(struct fifo_tx* self)
{
    picotm_ref_up(&self->ref);
}

void
fifo_tx_unref(struct fifo_tx* self)
{
    assert(self);

    bool final_ref = picotm_ref_down(&self->ref);
    if (!final_ref) {
        return;
    }

    fifo_unref(self->fifo);
    self->fifo = NULL;
}

bool
fifo_tx_holds_ref(struct fifo_tx* self)
{
    assert(self);

    return picotm_ref_count(&self->ref) > 0;
}

static off_t
append_to_iobuffer(struct fifo_tx* self, size_t nbyte, const void* buf,
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
fifo_tx_append_to_writeset(struct fifo_tx* self, size_t nbyte, off_t offset,
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
fifo_tx_append_to_readset(struct fifo_tx* self, size_t nbyte, off_t offset,
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
 * fcntl()
 */

static int
fcntl_exec_noundo(struct fifo_tx* self, int fildes, int cmd,
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
fcntl_exec_2pl(struct fifo_tx* self, int fildes, int cmd,
               union fcntl_arg* arg, int* cookie, struct picotm_error* error)
{
    assert(arg);

    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN: {

            /* Read-lock FIFO */
            fifo_tx_try_rdlock_field(self, FIFO_FIELD_STATE, error);
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

            /* Read-lock FIFO */
            fifo_tx_try_rdlock_field(self, FIFO_FIELD_STATE, error);
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
fifo_tx_fcntl_exec(struct fifo_tx* self, int fildes, int cmd,
                   union fcntl_arg* arg, bool isnoundo, int* cookie,
                   struct picotm_error* error)
{
    static int (* const fcntl_exec[2])(struct fifo_tx*,
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

void
fifo_tx_fcntl_apply(struct fifo_tx* self, int fildes, int cookie,
                    struct picotm_error* error)
{ }

void
fifo_tx_fcntl_undo(struct fifo_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{ }

/*
 * fstat()
 */

static int
fstat_exec_noundo(struct fifo_tx* self, int fildes, struct stat* buf,
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
fstat_exec_2pl(struct fifo_tx* self, int fildes, struct stat* buf,
               int* cookie, struct picotm_error* error)
{
    assert(self);
    assert(buf);

    /* Acquire file-mode reader lock. */
    fifo_tx_try_rdlock_field(self, FIFO_FIELD_FILE_MODE, error);
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
fifo_tx_fstat_exec(struct fifo_tx* self, int fildes, struct stat* buf,
                   bool isnoundo, int* cookie, struct picotm_error* error)
{
    static int (* const fstat_exec[2])(struct fifo_tx*,
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

static void
fstat_apply_noundo(struct fifo_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{ }

static void
fstat_apply_2pl(struct fifo_tx* self, int fildes, int cookie,
                struct picotm_error* error)
{ }

void
fifo_tx_fstat_apply(struct fifo_tx* self, int fildes, int cookie,
                    struct picotm_error* error)
{
    static void (* const fstat_apply[2])(struct fifo_tx*,
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
fstat_undo_2pl(struct fifo_tx* self, int fildes, int cookie,
               struct picotm_error* error)
{ }

void
fifo_tx_fstat_undo(struct fifo_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{
    static void (* const fstat_undo[2])(struct fifo_tx*,
                                        int,
                                        int,
                                        struct picotm_error*) = {
        NULL,
        fstat_undo_2pl
    };

    assert(fstat_undo[self->cc_mode]);

    fstat_undo[self->cc_mode](self, fildes, cookie, error);
}

/*
 * read()
 */

static ssize_t
read_exec_noundo(struct fifo_tx* self, int fildes, void* buf,
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
fifo_tx_read_exec(struct fifo_tx* self, int fildes, void* buf, size_t nbyte,
                  bool isnoundo, enum picotm_libc_validation_mode val_mode,
                  int* cookie, struct picotm_error* error)
{
    static ssize_t (* const read_exec[2])(struct fifo_tx*,
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

static void
read_apply_noundo(struct fifo_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{ }

void
fifo_tx_read_apply(struct fifo_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{
    static void (* const read_apply[2])(struct fifo_tx*,
                                        int,
                                        int,
                                        struct picotm_error*) = {
        read_apply_noundo,
        NULL
    };

    read_apply[self->cc_mode](self, fildes, cookie, error);
}

void
fifo_tx_read_undo(struct fifo_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static void (* const read_undo[2])(struct picotm_error*) = {
        NULL,
        NULL
    };

    assert(read_undo[self->cc_mode]);

    read_undo[self->cc_mode](error);
}

/*
 * write()
 */

static ssize_t
write_exec_noundo(struct fifo_tx* self, int fildes, const void* buf,
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
write_exec_2pl(struct fifo_tx* self, int fildes, const void* buf,
               size_t nbyte, int* cookie, struct picotm_error* error)
{
    /* Write-lock FIFO, because we change the file position */
    fifo_tx_try_wrlock_field(self, FIFO_FIELD_STATE, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Register write data */

    if (cookie) {
        *cookie = fifo_tx_append_to_writeset(self, nbyte, 0, buf, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return nbyte;
}

ssize_t
fifo_tx_write_exec(struct fifo_tx* self, int fildes, const void* buf,
                   size_t nbyte, bool isnoundo, int* cookie,
                   struct picotm_error* error)
{
    static ssize_t (* const write_exec[2])(struct fifo_tx*,
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

static void
write_apply_noundo(struct fifo_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{ }

static void
write_apply_2pl(struct fifo_tx* self, int fildes, int cookie,
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
fifo_tx_write_apply(struct fifo_tx* self, int fildes, int cookie,
                    struct picotm_error* error)
{
    static void (* const write_apply[2])(struct fifo_tx*,
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
write_any_undo(struct picotm_error* error)
{ }

void
fifo_tx_write_undo(struct fifo_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{
    static void (* const write_undo[2])(struct picotm_error*) = {
        NULL,
        write_any_undo
    };

    write_undo[self->cc_mode](error);
}
