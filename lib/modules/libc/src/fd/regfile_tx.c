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

#include "regfile_tx.h"
#include <assert.h>
#include <errno.h>
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
#include "ioop.h"
#include "iooptab.h"
#include "range.h"
#include "region.h"
#include "regiontab.h"
#include "seekop.h"
#include "seekoptab.h"

static void
regfile_tx_2pl_release_locks(struct regfile_tx* self)
{
    assert(self);

    /* release all locks */

    struct region* region = self->locktab;
    const struct region* regionend = region+self->locktablen;

    while (region < regionend) {
        regfile_unlock_region(self->regfile,
                              region->offset,
                              region->nbyte,
                              &self->rwcountermap);
        ++region;
    }
}

static void
regfile_tx_try_rdlock_field(struct regfile_tx* self, enum regfile_field field,
                            struct picotm_error* error)
{
    assert(self);

    regfile_try_rdlock_field(self->regfile, field, self->rwstate + field,
                             error);
}

static void
regfile_tx_try_wrlock_field(struct regfile_tx* self, enum regfile_field field,
                            struct picotm_error* error)
{
    assert(self);

    regfile_try_wrlock_field(self->regfile, field, self->rwstate + field,
                             error);
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
                struct regfile* regfile)
{
    enum regfile_field field = 0;

    while (beg < end) {
        regfile_unlock_field(regfile, field, beg);
        ++field;
        ++beg;
    }
}

static struct regfile_tx*
regfile_tx_of_file_tx(struct file_tx* file_tx)
{
    assert(file_tx);
    assert(file_tx_file_type(file_tx) == PICOTM_LIBC_FILE_TYPE_REGULAR);

    return picotm_containerof(file_tx, struct regfile_tx, base);
}

static void
ref_file_tx(struct file_tx* file_tx)
{
    regfile_tx_ref(regfile_tx_of_file_tx(file_tx));
}

static void
unref_file_tx(struct file_tx* file_tx)
{
    regfile_tx_unref(regfile_tx_of_file_tx(file_tx));
}

static off_t
append_to_iobuffer(struct regfile_tx* self, size_t nbyte, const void* buf,
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

static int
regfile_tx_append_to_writeset(struct regfile_tx* self, size_t nbyte, off_t offset,
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

/*
 * Concurrency Control
 */

static int
regfile_tx_2pl_lock_region(struct regfile_tx* self, size_t nbyte, off_t offset,
                           bool iswrite, struct picotm_error* error)
{
    void (* const try_lock_region[])(struct regfile*,
                                     off_t,
                                     size_t,
                                     struct rwcountermap*,
                                     struct picotm_error*) = {
        regfile_try_rdlock_region,
        regfile_try_wrlock_region
    };

    assert(self);

    try_lock_region[iswrite](self->regfile,
                             offset,
                             nbyte,
                             &self->rwcountermap,
                             error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int pos = regiontab_append(&self->locktab,
                               &self->locktablen,
                               &self->locktabsiz,
                               nbyte, offset,
                               error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    return pos;
}

/*
 * accept()
 */

static int
accept_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
            struct sockaddr* address, socklen_t* address_len,
            bool isnoundo, int* cookie, struct picotm_error* error)
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
fchmod_exec_noundo(struct regfile_tx* self, int fildes, mode_t mode,
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
fchmod_exec_2pl(struct regfile_tx* self, int fildes, mode_t mode,
                int* cookie, struct picotm_error* error)
{
    assert(self);
    assert(cookie);

    /* Acquire file-mode lock. */
    regfile_tx_try_wrlock_field(self, REGFILE_FIELD_FILE_MODE, error);
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
regfile_tx_fchmod_exec(struct regfile_tx* self, int fildes, mode_t mode,
                       bool isnoundo, int* cookie, struct picotm_error* error)
{
    static int (* const fchmod_exec[2])(struct regfile_tx*,
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
    return regfile_tx_fchmod_exec(regfile_tx_of_file_tx(base), fildes, mode,
                                  isnoundo, cookie, error);
}

static void
fchmod_apply_noundo(int fildes, struct picotm_error* error)
{ }

static void
fchmod_apply_2pl(int fildes, struct picotm_error* error)
{ }

static void
regfile_tx_fchmod_apply(struct regfile_tx* self, int fildes, int cookie,
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
    regfile_tx_fchmod_apply(regfile_tx_of_file_tx(base), fildes, cookie,
                            error);
}

static void
fchmod_undo_2pl(struct regfile_tx* self, int fildes, int cookie,
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
regfile_tx_fchmod_undo(struct regfile_tx* self, int fildes, int cookie,
                       struct picotm_error* error)
{
    static void (* const fchmod_undo[2])(struct regfile_tx*,
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
    regfile_tx_fchmod_undo(regfile_tx_of_file_tx(base), fildes, cookie,
                           error);
}

/*
 * fcntl()
 */

static int
fcntl_exec_noundo(struct regfile_tx* self, int fildes, int cmd,
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
fcntl_exec_2pl(struct regfile_tx* self, int fildes, int cmd,
               union fcntl_arg* arg, int* cookie, struct picotm_error* error)
{
    assert(arg);

    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN: {

            /* Read-lock open file description */
            regfile_tx_try_rdlock_field(self, REGFILE_FIELD_STATE, error);
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
            regfile_tx_try_rdlock_field(self, REGFILE_FIELD_STATE, error);
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
regfile_tx_fcntl_exec(struct regfile_tx* self, int fildes, int cmd,
                      union fcntl_arg* arg, bool isnoundo, int* cookie,
                      struct picotm_error* error)
{
    static int (* const fcntl_exec[2])(struct regfile_tx*,
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
    return regfile_tx_fcntl_exec(regfile_tx_of_file_tx(base), fildes, cmd,
                                 arg, isnoundo, cookie, error);
}

static void
fcntl_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            int cookie, struct picotm_error* error)
{ }

static void
fcntl_undo(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           int cookie, struct picotm_error* error)
{ }

/*
 * fstat()
 */

static int
fstat_exec_noundo(struct regfile_tx* self, int fildes, struct stat* buf,
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
fstat_exec_2pl(struct regfile_tx* self, int fildes, struct stat* buf,
               int* cookie, struct picotm_error* error)
{
    assert(self);
    assert(buf);

    /* Acquire file-mode reader lock. */
    regfile_tx_try_wrlock_field(self, REGFILE_FIELD_FILE_MODE, error);
    if (picotm_error_is_set(error)) {
        picotm_error_set_errno(error, errno);
        return -1;
    }

    /* Acquire file-size reader lock. */
    regfile_tx_try_rdlock_field(self, REGFILE_FIELD_FILE_SIZE, error);
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
regfile_tx_fstat_exec(struct regfile_tx* self, int fildes, struct stat* buf,
                      bool isnoundo, int* cookie, struct picotm_error* error)
{
    static int (* const fstat_exec[2])(struct regfile_tx*,
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
    return regfile_tx_fstat_exec(regfile_tx_of_file_tx(base), fildes, buf,
                                 isnoundo, cookie, error);
}

static void
fstat_apply_noundo(struct regfile_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{ }

static void
fstat_apply_2pl(struct regfile_tx* self, int fildes, int cookie,
                struct picotm_error* error)
{ }

static void
regfile_tx_fstat_apply(struct regfile_tx* self, int fildes, int cookie,
                       struct picotm_error* error)
{
    static void (* const fstat_apply[2])(struct regfile_tx*,
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
    regfile_tx_fstat_apply(regfile_tx_of_file_tx(base), fildes, cookie, error);
}

static void
fstat_undo_2pl(struct regfile_tx* self, int fildes, int cookie,
               struct picotm_error* error)
{ }

static void
regfile_tx_fstat_undo(struct regfile_tx* self, int fildes, int cookie,
                      struct picotm_error* error)
{
    static void (* const fstat_undo[2])(struct regfile_tx*,
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
    regfile_tx_fstat_undo(regfile_tx_of_file_tx(base), fildes, cookie, error);
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
regfile_tx_fsync_exec(struct regfile_tx* self, int fildes, bool isnoundo,
                      int* cookie, struct picotm_error* error)
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
    return regfile_tx_fsync_exec(regfile_tx_of_file_tx(base), fildes,
                                 isnoundo, cookie, error);
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
regfile_tx_fsync_apply(struct regfile_tx* self, int fildes, int cookie,
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
    regfile_tx_fsync_apply(regfile_tx_of_file_tx(base), fildes, cookie,
                           error);
}

static void
fsync_undo_2pl(int fildes, int cookie, struct picotm_error* error)
{ }

static void
regfile_tx_fsync_undo(struct regfile_tx* self, int fildes, int cookie,
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
    regfile_tx_fsync_undo(regfile_tx_of_file_tx(base), fildes, cookie,
                          error);
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
filesize(int fildes, struct picotm_error* error)
{
    struct stat buf;

    int res = fstat(fildes, &buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return (off_t)-1;
    }

    return buf.st_size;
}

static off_t
lseek_exec_noundo(struct regfile_tx* self, int fildes, off_t offset,
                  int whence, int* cookie, struct picotm_error* error)
{
    off_t res = TEMP_FAILURE_RETRY(lseek(fildes, offset, whence));
    if (res == (off_t)-1) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

static off_t
lseek_exec_2pl(struct regfile_tx* self, int fildes, off_t offset,
               int whence, int* cookie, struct picotm_error* error)
{
    /* Read-lock open file description */
    regfile_tx_try_rdlock_field(self, REGFILE_FIELD_STATE, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

	/* Fastpath: Read current position */
	if (!offset && (whence == SEEK_CUR)) {
		return self->offset;
	}

    /* Write-lock open file description to change position */
    regfile_tx_try_wrlock_field(self, REGFILE_FIELD_STATE, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Compute absolute position */

    self->size = llmax(self->offset, self->size);

    off_t pos;

    switch (whence) {
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_CUR:
            pos = self->offset + offset;
            break;
        case SEEK_END: {
            const off_t fs = filesize(fildes, error);
            if (picotm_error_is_set(error)) {
                break;
            }
            pos = llmax(self->size, fs)+offset;
            break;
        }
        default:
            pos = -1;
            break;
    }

    if (picotm_error_is_set(error)) {
        return -1;
    }

    if (cookie) {
        *cookie = seekoptab_append(&self->seektab,
                                   &self->seektablen,
                                    self->offset, offset, whence,
                                    error);
        if (picotm_error_is_set(error)) {
            abort();
        }
    }

    self->offset = pos; /* Update file pointer */

    return pos;
}

static off_t
regfile_tx_lseek_exec(struct regfile_tx* self, int fildes, off_t offset,
                      int whence, bool isnoundo, int* cookie,
                      struct picotm_error* error)
{
    static off_t (* const lseek_exec[2])(struct regfile_tx*,
                                         int,
                                         off_t,
                                         int,
                                         int*,
                                         struct picotm_error*) = {
        lseek_exec_noundo,
        lseek_exec_2pl
    };

    if (isnoundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !lseek_exec[self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return lseek_exec[self->cc_mode](self, fildes, offset, whence,
                                     cookie, error);
}

static off_t
lseek_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           off_t offset, int whence, bool isnoundo, int* cookie,
           struct picotm_error* error)
{
    return regfile_tx_lseek_exec(regfile_tx_of_file_tx(base), fildes, offset,
                                 whence, isnoundo, cookie, error);
}

static void
lseek_apply_noundo(struct regfile_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{ }

static void
lseek_apply_2pl(struct regfile_tx* self, int fildes, int cookie,
                struct picotm_error* error)
{
    const off_t pos = lseek(fildes, self->seektab[cookie].offset,
                                    self->seektab[cookie].whence);

    if (pos == (off_t)-1) {
        picotm_error_set_errno(error, errno);
        return;
    }

    self->regfile->offset = pos;
}

static void
regfile_tx_lseek_apply(struct regfile_tx* self, int fildes, int cookie,
                       struct picotm_error* error)
{
    static void (* const lseek_apply[2])(struct regfile_tx*,
                                         int,
                                         int,
                                         struct picotm_error*) = {
        lseek_apply_noundo,
        lseek_apply_2pl
    };

    assert(lseek_apply[self->cc_mode]);

    lseek_apply[self->cc_mode](self, fildes, cookie, error);
}

static void
lseek_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            int cookie, struct picotm_error* error)
{
    regfile_tx_lseek_apply(regfile_tx_of_file_tx(base), fildes, cookie, error);
}

static void
lseek_undo_2pl(struct picotm_error* error)
{ }

static void
regfile_tx_lseek_undo(struct regfile_tx* self, int fildes, int cookie,
                      struct picotm_error* error)
{
    static void (* const lseek_undo[2])(struct picotm_error*) = {
        NULL,
        lseek_undo_2pl,
    };

    assert(lseek_undo[self->cc_mode]);

    lseek_undo[self->cc_mode](error);
}

static void
lseek_undo(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           int cookie, struct picotm_error* error)
{
    regfile_tx_lseek_undo(regfile_tx_of_file_tx(base), fildes, cookie, error);
}

/*
 * pread()
 */

/*

Busy-wait instead of syscall

typedef unsigned long long ticks;
static __inline__ ticks getticks(void)
{
     unsigned a, d;
     __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));
     return ((ticks)a) | (((ticks)d) << 32);
}

static ssize_t
pread_wait(int fildes, void *buf, size_t nbyte, off_t off)
{
    ticks t0 = getticks();
    memset(buf, 0, nbyte);
    ticks t1 = getticks();

    if (!nbyte) {
        while ( (t1-t0) < 631 ) {
            t1 = getticks();
        }
    } else {

        // approximation of single-thread cycles
        long limit = 1020 + (339*nbyte)/1000;

        while ( (t1-t0) < limit ) {
            t1 = getticks();
        }
    }

    return nbyte;
}*/

static ssize_t
pread_exec_noundo(struct regfile_tx* self, int fildes, void* buf,
                  size_t nbyte, off_t offset,
                  enum picotm_libc_validation_mode val_mode, int* cookie,
                  struct picotm_error* error)
{
    ssize_t res = TEMP_FAILURE_RETRY(pread(fildes, buf, nbyte, offset));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

static ssize_t
pread_exec_2pl(struct regfile_tx* self, int fildes, void* buf,
               size_t nbyte, off_t offset,
               enum picotm_libc_validation_mode val_mode, int* cookie,
               struct picotm_error* error)
{
    /* lock region */
    regfile_tx_2pl_lock_region(self, nbyte, offset, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    size_t len = 0;
    uint8_t* buf8 = buf;

    /* read from file */

    while (len < nbyte) {

        ssize_t res = TEMP_FAILURE_RETRY(pread(fildes,
                                               buf8 + len, nbyte - len,
                                               offset));
        if (res < 0) {
            picotm_error_set_errno(error, errno);
            return res;
        } else if (!res) {
            break; /* EOF */
        }

        len += res;
    }

    /* read from local write set */
    size_t len2 = iooptab_read(self->wrtab,
                               self->wrtablen,
                               buf, nbyte, offset, self->wrbuf);

    return llmax(len, len2);
}

static ssize_t
regfile_tx_pread_exec(struct regfile_tx* self, int fildes, void* buf,
                      size_t nbyte, off_t offset, bool isnoundo,
                      enum picotm_libc_validation_mode val_mode, int* cookie,
                      struct picotm_error* error)
{
    static ssize_t (* const pread_exec[2])(struct regfile_tx*,
                                           int,
                                           void*,
                                           size_t,
                                           off_t,
                                           enum picotm_libc_validation_mode,
                                           int*,
                                           struct picotm_error*) = {
        pread_exec_noundo,
        pread_exec_2pl
    };

    if (isnoundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !pread_exec[self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return pread_exec[self->cc_mode](self, fildes, buf, nbyte, offset,
                                     val_mode, cookie, error);
}

static ssize_t
pread_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes, void* buf,
           size_t nbyte, off_t offset, bool isnoundo,
           enum picotm_libc_validation_mode val_mode, int* cookie,
           struct picotm_error* error)
{
    return regfile_tx_pread_exec(regfile_tx_of_file_tx(base), fildes, buf,
                                 nbyte, offset, isnoundo, val_mode, cookie,
                                 error);
}

static void
pread_apply_noundo(struct picotm_error* error)
{ }

static void
pread_apply_2pl(struct picotm_error* error)
{ }

static void
regfile_tx_pread_apply(struct regfile_tx* self, int fildes, int cookie,
                       struct picotm_error* error)
{
    static void (* const pread_apply[2])(struct picotm_error*) = {
        pread_apply_noundo,
        pread_apply_2pl
    };

    assert(pread_apply[self->cc_mode]);

    pread_apply[self->cc_mode](error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
pread_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            int cookie, struct picotm_error* error)
{
    regfile_tx_pread_apply(regfile_tx_of_file_tx(base), fildes, cookie, error);
}

static void
pread_undo_2pl(struct picotm_error* error)
{ }

static void
regfile_tx_pread_undo(struct regfile_tx* self, int fildes, int cookie,
                      struct picotm_error* error)
{
    static void (* const pread_undo[2])(struct picotm_error*) = {
        NULL,
        pread_undo_2pl,
    };

    assert(pread_undo[self->cc_mode]);

    pread_undo[self->cc_mode](error);
}

static void
pread_undo(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           int cookie, struct picotm_error* error)
{
    regfile_tx_pread_undo(regfile_tx_of_file_tx(base), fildes, cookie, error);
}

/*
 * pwrite()
 */

/*

Busy-wait instead of syscall

typedef unsigned long long ticks;
static __inline__ ticks getticks(void)
{
     unsigned a, d;
     __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));
     return ((ticks)a) | (((ticks)d) << 32);
}

static ssize_t
pwrite_wait(int fildes, const void *buf, size_t nbyte, off_t off)
{
    ticks t0 = getticks();
    ticks t1 = getticks();

    if (!nbyte) {
        while ( (t1-t0) < 765 ) {
            t1 = getticks();
        }
    } else {

        // approximation of single-thread cycles
        long limit = 1724 + (1139*nbyte)/1000;

        while ( (t1-t0) < limit ) {
            t1 = getticks();
        }
    }

    return nbyte;
}*/

static ssize_t
pwrite_exec_noundo(struct regfile_tx* self, int fildes, const void* buf,
                   size_t nbyte, off_t offset, int* cookie,
                   struct picotm_error* error)
{
    ssize_t res = TEMP_FAILURE_RETRY(pwrite(fildes, buf, nbyte, offset));
    if (res) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

static ssize_t
pwrite_exec_2pl(struct regfile_tx* self, int fildes,
                const void* buf, size_t nbyte, off_t offset,
                int* cookie, struct picotm_error* error)
{
    /* register written data */

    if (__builtin_expect(!!cookie, 1)) {

        /* lock region */

        regfile_tx_2pl_lock_region(self, nbyte, offset, 1, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }

        /* add data to write set */
        *cookie = regfile_tx_append_to_writeset(self, nbyte, offset, buf, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return nbyte;
}

static ssize_t
regfile_tx_pwrite_exec(struct regfile_tx* self, int fildes, const void* buf,
                       size_t nbyte, off_t offset, bool isnoundo, int* cookie,
                       struct picotm_error* error)
{
    static ssize_t (* const pwrite_exec[2])(struct regfile_tx*,
                                            int,
                                            const void*,
                                            size_t,
                                            off_t,
                                            int*,
                                            struct picotm_error*) = {
        pwrite_exec_noundo,
        pwrite_exec_2pl,
    };

    if (isnoundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !pwrite_exec[self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return pwrite_exec[self->cc_mode](self, fildes, buf, nbyte, offset,
                                      cookie, error);
}

static ssize_t
pwrite_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            const void* buf, size_t nbyte, off_t offset, bool isnoundo,
            int* cookie, struct picotm_error* error)
{
    return regfile_tx_pwrite_exec(regfile_tx_of_file_tx(base), fildes, buf,
                                  nbyte, offset, isnoundo, cookie, error);
}

static void
pwrite_apply_noundo(struct regfile_tx* self, int fildes, int cookie,
                    struct picotm_error* error)
{ }

static void
pwrite_apply_2pl(struct regfile_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);

    off_t  off = self->wrtab[cookie].off;
    size_t nbyte = self->wrtab[cookie].nbyte;
    off_t  bufoff = self->wrtab[cookie].bufoff;

    ssize_t res = TEMP_FAILURE_RETRY(pwrite(fildes,
                                            self->wrbuf + bufoff,
                                            nbyte, off));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

static void
regfile_tx_pwrite_apply(struct regfile_tx* self, int fildes, int cookie,
                        struct picotm_error* error)
{
    static void (* const pwrite_apply[2])(struct regfile_tx*,
                                          int,
                                          int,
                                          struct picotm_error*) = {
        pwrite_apply_noundo,
        pwrite_apply_2pl
    };

    assert(pwrite_apply[self->cc_mode]);

    pwrite_apply[self->cc_mode](self, fildes, cookie, error);
}

static void
pwrite_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
             int cookie, struct picotm_error* error)
{
    regfile_tx_pwrite_apply(regfile_tx_of_file_tx(base), fildes, cookie,
                            error);
}

static void
pwrite_undo_2pl(struct picotm_error* error)
{ }

static void
regfile_tx_pwrite_undo(struct regfile_tx* self, int fildes, int cookie,
                       struct picotm_error* error)
{
    static void (* const pwrite_undo[2])(struct picotm_error*) = {
        NULL,
        pwrite_undo_2pl
    };

    assert(pwrite_undo[self->cc_mode]);

    pwrite_undo[self->cc_mode](error);
}

static void
pwrite_undo(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            int cookie, struct picotm_error* error)
{
    regfile_tx_pwrite_undo(regfile_tx_of_file_tx(base), fildes, cookie,
                           error);
}

/*
 * read()
 */

static ssize_t
read_exec_noundo(struct regfile_tx* self, int fildes, void* buf, size_t nbyte,
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

static ssize_t
read_exec_2pl(struct regfile_tx* self, int fildes, void* buf, size_t nbyte,
              enum picotm_libc_validation_mode val_mode, int* cookie,
              struct picotm_error* error)
{
    assert(self);

    /* write-lock open file description, because we change the file position */
    regfile_tx_try_wrlock_field(self, REGFILE_FIELD_STATE, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* read-lock region */

    regfile_tx_2pl_lock_region(self, nbyte, self->offset, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* read from file */

    ssize_t len = TEMP_FAILURE_RETRY(pread(fildes, buf, nbyte, self->offset));
    if (len < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }

    /* read from local write set */

    ssize_t len2 = iooptab_read(self->wrtab,
                                self->wrtablen, buf, nbyte,
                                self->offset,
                                self->wrbuf);

    ssize_t res = llmax(len, len2);

    /* update file pointer */
    self->offset += res;

    return res;
}

ssize_t
regfile_tx_read_exec(struct regfile_tx* self, int fildes, void* buf,
                     size_t nbyte, bool isnoundo,
                     enum picotm_libc_validation_mode val_mode, int* cookie,
                     struct picotm_error* error)
{
    static ssize_t (* const read_exec[2])(struct regfile_tx*,
                                          int,
                                          void*,
                                          size_t,
                                          enum picotm_libc_validation_mode,
                                          int*,
                                          struct picotm_error*) = {
        read_exec_noundo,
        read_exec_2pl
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
read_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes, void* buf,
          size_t nbyte, bool isnoundo,
          enum picotm_libc_validation_mode val_mode, int* cookie,
          struct picotm_error* error)
{
    return regfile_tx_read_exec(regfile_tx_of_file_tx(base), fildes, buf,
                                nbyte, isnoundo, val_mode, cookie, error);
}

static void
read_apply_noundo(struct regfile_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{ }

static void
read_apply_2pl(struct regfile_tx* self, int fildes, int cookie,
               struct picotm_error* error)
{
    self->regfile->offset += self->rdtab[cookie].nbyte;

    off_t res = lseek(fildes, self->regfile->offset, SEEK_SET);
    if (res == (off_t)-1) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

static void
regfile_tx_read_apply(struct regfile_tx* self, int fildes, int cookie,
                      struct picotm_error* error)
{
    static void (* const read_apply[2])(struct regfile_tx*,
                                        int,
                                        int,
                                        struct picotm_error*) = {
        read_apply_noundo,
        read_apply_2pl
    };

    assert(read_apply[self->cc_mode]);

    read_apply[self->cc_mode](self, fildes, cookie, error);
}

static void
read_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           int cookie, struct picotm_error* error)
{
    regfile_tx_read_apply(regfile_tx_of_file_tx(base), fildes, cookie,
                          error);
}

static void
read_undo_2pl(struct picotm_error* error)
{ }

static void
regfile_tx_read_undo(struct regfile_tx* self, int fildes, int cookie,
                     struct picotm_error* error)
{
    static void (* const read_undo[2])(struct picotm_error*) = {
        NULL,
        read_undo_2pl
    };

    assert(read_undo[self->cc_mode]);

    read_undo[self->cc_mode](error);
}

static void
read_undo(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes, int cookie,
          struct picotm_error* error)
{
    regfile_tx_read_undo(regfile_tx_of_file_tx(base), fildes, cookie,
                         error);
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
write_exec_noundo(struct regfile_tx* self, int fildes, const void* buf,
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
write_exec_2pl(struct regfile_tx* self, int fildes, const void* buf,
               size_t nbyte, int* cookie, struct picotm_error* error)
{
    /* register written data */

    if (__builtin_expect(!!cookie, 1)) {

        /* write-lock open file description, because we change the file position */
        regfile_tx_try_wrlock_field(self, REGFILE_FIELD_STATE, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }

        /* write-lock region */

        regfile_tx_2pl_lock_region(self, nbyte, self->offset, 1, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }

        /* add buf to write set */
        *cookie = regfile_tx_append_to_writeset(self, nbyte, self->offset,
                                                buf, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    /* update file pointer */
    self->offset += nbyte;

    return nbyte;
}

static ssize_t
regfile_tx_write_exec(struct regfile_tx* self, int fildes, const void* buf,
                      size_t nbyte, bool isnoundo, int* cookie,
                      struct picotm_error* error)
{
    static ssize_t (* const write_exec[2])(struct regfile_tx*,
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
write_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           const void* buf, size_t nbyte, bool isnoundo, int* cookie,
           struct picotm_error* error)
{
    return regfile_tx_write_exec(regfile_tx_of_file_tx(base), fildes, buf,
                                 nbyte, isnoundo, cookie, error);
}

static void
write_apply_noundo(struct regfile_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{ }

static void
write_apply_2pl(struct regfile_tx* self, int fildes, int cookie,
                struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);

    /* FIXME: Use select() to prevent blocking? */

    const ssize_t len =
        TEMP_FAILURE_RETRY(pwrite(fildes,
                                  self->wrbuf+self->wrtab[cookie].bufoff,
                                  self->wrtab[cookie].nbyte,
                                  self->wrtab[cookie].off));
    if (len < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }

    /* Update file position */
    self->regfile->offset = self->wrtab[cookie].off+len;

    off_t res = lseek(fildes, self->regfile->offset, SEEK_SET);
    if (res == (off_t)-1) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

static void
regfile_tx_write_apply(struct regfile_tx* self, int fildes, int cookie,
                       struct picotm_error* error)
{
    static void (* const write_apply[2])(struct regfile_tx*,
                                         int,
                                         int,
                                         struct picotm_error*) = {
        write_apply_noundo,
        write_apply_2pl
    };

    assert(write_apply[self->cc_mode]);

    write_apply[self->cc_mode](self, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
write_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            int cookie, struct picotm_error* error)
{
    regfile_tx_write_apply(regfile_tx_of_file_tx(base), fildes, cookie,
                           error);
}

static void
write_undo_2pl(struct picotm_error* error)
{ }

static void
regfile_tx_write_undo(struct regfile_tx* self, int fildes, int cookie,
                      struct picotm_error* error)
{
    static void (* const write_undo[2])(struct picotm_error*) = {
        NULL,
        write_undo_2pl
    };

    assert(write_undo[self->cc_mode]);

    write_undo[self->cc_mode](error);
}

static void
write_undo(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           int cookie, struct picotm_error* error)
{
    regfile_tx_write_undo(regfile_tx_of_file_tx(base), fildes, cookie,
                          error);
}

/*
 * Public interface
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
validate_noundo(struct regfile_tx* self, struct picotm_error* error)
{ }

static void
validate_2pl(struct regfile_tx* self, struct picotm_error* error)
{
    assert(self);

    /* Locked regions are ours, so we do not need to validate here. All
     * conflicting transactions will have aborted on encountering our locks.
     *
     * The state of the OFD itself is guarded by regfile::rwlock.
     */
}

static void
regfile_tx_validate(struct regfile_tx* self, struct picotm_error* error)
{
    static void (* const validate[])(struct regfile_tx*, struct picotm_error*) = {
        validate_noundo,
        validate_2pl
    };

    if (!regfile_tx_holds_ref(self)) {
        return;
    }

    validate[self->cc_mode](self, error);
}

static void
validate_file_tx(struct file_tx* base, struct picotm_error* error)
{
    regfile_tx_validate(regfile_tx_of_file_tx(base), error);
}

/* Update CC
 */

static void
update_cc_noundo(struct regfile_tx* self, struct picotm_error* error)
{ }

static void
update_cc_2pl(struct regfile_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_2PL);

    /* release record locks */
    regfile_tx_2pl_release_locks(self);

    /* release reader/writer locks on file state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->regfile);
}

static void
regfile_tx_update_cc(struct regfile_tx* self, struct picotm_error* error)
{
    static void (* const update_cc[])(struct regfile_tx*, struct picotm_error*) = {
        update_cc_noundo,
        update_cc_2pl
    };

    assert(regfile_tx_holds_ref(self));

    update_cc[self->cc_mode](self, error);
}

static void
update_cc_file_tx(struct file_tx* base, struct picotm_error* error)
{
    regfile_tx_update_cc(regfile_tx_of_file_tx(base), error);
}

/* Clear CC
 */

static void
clear_cc_noundo(struct regfile_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO);
}

static void
clear_cc_2pl(struct regfile_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_2PL);

    /* release record locks */
    regfile_tx_2pl_release_locks(self);

    /* release reader/writer locks on file state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->regfile);
}

static void
regfile_tx_clear_cc(struct regfile_tx* self, struct picotm_error* error)
{
    static void (* const clear_cc[])(struct regfile_tx*, struct picotm_error*) = {
        clear_cc_noundo,
        clear_cc_2pl
    };

    assert(regfile_tx_holds_ref(self));

    clear_cc[self->cc_mode](self, error);
}

static void
clear_cc_file_tx(struct file_tx* base, struct picotm_error* error)
{
    regfile_tx_clear_cc(regfile_tx_of_file_tx(base), error);
}

/*
 * Public interface
 */

static const struct file_tx_ops regfile_tx_ops = {
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
    lseek_apply,
    lseek_undo,
    pread_exec,
    pread_apply,
    pread_undo,
    pwrite_exec,
    pwrite_apply,
    pwrite_undo,
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
regfile_tx_init(struct regfile_tx* self)
{
    assert(self);

    picotm_ref_init(&self->ref, 0);

    file_tx_init(&self->base, PICOTM_LIBC_FILE_TYPE_REGULAR, &regfile_tx_ops);

    self->regfile = NULL;

    self->wrbuf = NULL;
    self->wrbuflen = 0;
    self->wrbufsiz = 0;

    self->wrtab = NULL;
    self->wrtablen = 0;
    self->wrtabsiz = 0;

    self->rdtab = NULL;
    self->rdtablen = 0;
    self->rdtabsiz = 0;

    self->seektab = NULL;
    self->seektablen = 0;

    self->fchmodtab = NULL;
    self->fchmodtablen = 0;

    self->fcntltab = NULL;
    self->fcntltablen = 0;

    self->offset = 0;
    self->size = 0;
    self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));

    rwcountermap_init(&self->rwcountermap);

    self->locktab = NULL;
    self->locktablen = 0;
    self->locktabsiz = 0;
}

void
regfile_tx_uninit(struct regfile_tx* self)
{
    assert(self);

    free(self->locktab);
    rwcountermap_uninit(&self->rwcountermap);

    fcntloptab_clear(&self->fcntltab, &self->fcntltablen);
    fchmodoptab_clear(&self->fchmodtab, &self->fchmodtablen);
    seekoptab_clear(&self->seektab, &self->seektablen);
    iooptab_clear(&self->wrtab, &self->wrtablen);
    iooptab_clear(&self->rdtab, &self->rdtablen);
    free(self->wrbuf);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));
}

/*
 * Referencing
 */

void
regfile_tx_ref_or_set_up(struct regfile_tx* self, struct regfile* regfile,
                         struct picotm_error* error)
{
    assert(self);
    assert(regfile);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    /* get reference on file */
    regfile_ref(regfile);

    /* setup fields */

    self->regfile = regfile;
    self->cc_mode = regfile_get_cc_mode(regfile);
    self->offset = regfile_get_offset(regfile);
    self->size = 0;

    self->fchmodtablen = 0;
    self->fcntltablen = 0;
    self->seektablen = 0;
    self->rdtablen = 0;
    self->wrtablen = 0;
    self->wrbuflen = 0;

    self->locktablen = 0;
}

void
regfile_tx_ref(struct regfile_tx* self)
{
    picotm_ref_up(&self->ref);
}

void
regfile_tx_unref(struct regfile_tx* self)
{
    assert(self);

    bool final_ref = picotm_ref_down(&self->ref);
    if (!final_ref) {
        return;
    }

    regfile_unref(self->regfile);
    self->regfile = NULL;
}

bool
regfile_tx_holds_ref(struct regfile_tx* self)
{
    assert(self);

    return picotm_ref_count(&self->ref) > 0;
}
