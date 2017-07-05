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
#include <picotm/picotm-lib-tab.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "fcntlop.h"
#include "fcntloptab.h"
#include "ioop.h"
#include "iooptab.h"
#include "ofd.h"
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
        ofd_2pl_unlock_region(self->ofd,
                              region->offset,
                              region->nbyte,
                              &self->rwcountermap);
        ++region;
    }
}

void
regfile_tx_init(struct regfile_tx* self)
{
    assert(self);

    picotm_ref_init(&self->ref, 0);

    memset(&self->active_list, 0, sizeof(self->active_list));

    self->ofd = NULL;

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

    self->seektab = NULL;
    self->seektablen = 0;

    self->fcntltab = NULL;
    self->fcntltablen = 0;

    self->offset = 0;
    self->size = 0;
    self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;

    /* PLP */

    picotm_rwstate_init(&self->rwstate);

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
    seekoptab_clear(&self->seektab, &self->seektablen);
    iooptab_clear(&self->wrtab, &self->wrtablen);
    iooptab_clear(&self->rdtab, &self->rdtablen);
    free(self->wrbuf);
}

/*
 * Validation
 */

void
regfile_tx_lock(struct regfile_tx* self)
{
    assert(self);
}

void
regfile_tx_unlock(struct regfile_tx* self)
{
    assert(self);
}

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
     * The state of the OFD itself is guarded by ofd::rwlock.
     */
}

void
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

/*
 * Update CC
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

    /* release ofd lock */

    ofd_unlock_state(self->ofd, &self->rwstate);
}

void
regfile_tx_update_cc(struct regfile_tx* self, struct picotm_error* error)
{
    static void (* const update_cc[])(struct regfile_tx*, struct picotm_error*) = {
        update_cc_noundo,
        update_cc_2pl
    };

    assert(regfile_tx_holds_ref(self));

    update_cc[self->cc_mode](self, error);
}

/*
 * Clear CC
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

    /* release ofd lock */

    ofd_unlock_state(self->ofd, &self->rwstate);
}

void
regfile_tx_clear_cc(struct regfile_tx* self, struct picotm_error* error)
{
    static void (* const clear_cc[])(struct regfile_tx*, struct picotm_error*) = {
        clear_cc_noundo,
        clear_cc_2pl
    };

    assert(regfile_tx_holds_ref(self));

    clear_cc[self->cc_mode](self, error);
}

/*
 * Referencing
 */

void
regfile_tx_ref_or_set_up(struct regfile_tx* self, struct ofd* ofd, int fildes,
                         unsigned long flags, struct picotm_error* error)
{
    assert(self);
    assert(ofd);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    /* get reference and status */

    off_t offset;
    enum picotm_libc_file_type type;
    enum picotm_libc_cc_mode cc_mode;
    ofd_ref_state(ofd, &type, &cc_mode, &offset);
    if (picotm_error_is_set(error)) {
        goto err_ofd_ref_state;
    }

    /* setup fields */

    self->ofd = ofd;
    self->cc_mode = cc_mode;
    self->offset = offset;
    self->size = 0;
    self->flags = 0;

    self->fcntltablen = 0;
    self->seektablen = 0;
    self->rdtablen = 0;
    self->wrtablen = 0;
    self->wrbuflen = 0;

    picotm_rwstate_set_status(&self->rwstate, PICOTM_RWSTATE_UNLOCKED);
    self->locktablen = 0;

    return;

err_ofd_ref_state:
    picotm_ref_down(&self->ref);
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

    ofd_unref(self->ofd);
    self->ofd = NULL;
}

bool
regfile_tx_holds_ref(struct regfile_tx* self)
{
    assert(self);

    return picotm_ref_count(&self->ref) > 0;
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

int
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

int
regfile_tx_append_to_readset(struct regfile_tx* self, size_t nbyte, off_t offset,
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
 * Concurrency Control
 */

int
regfile_tx_2pl_lock_region(struct regfile_tx* self, size_t nbyte, off_t offset,
                           bool iswrite, struct picotm_error* error)
{
    assert(self);

    ofd_2pl_lock_region(self->ofd,
                        offset,
                        nbyte,
                        iswrite,
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
            ofd_rdlock_state(self->ofd, &self->rwstate, error);
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
            ofd_rdlock_state(self->ofd, &self->rwstate, error);
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

void
regfile_tx_fcntl_apply(struct regfile_tx* self, int fildes, int cookie,
                       struct picotm_error* error)
{ }

void
regfile_tx_fcntl_undo(struct regfile_tx* self, int fildes, int cookie,
                      struct picotm_error* error)
{ }

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

int
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

void
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
fsync_undo_2pl(int fildes, int cookie, struct picotm_error* error)
{ }

void
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

    ofd_rdlock_state(self->ofd, &self->rwstate, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

	/* Fastpath: Read current position */
	if (!offset && (whence == SEEK_CUR)) {
		return self->offset;
	}

    /* Write-lock open file description to change position */

    ofd_wrlock_state(self->ofd, &self->rwstate, error);
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

off_t
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

    self->ofd->data.regular.offset = pos;
}

void
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
lseek_undo_2pl(struct picotm_error* error)
{ }

void
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

ssize_t
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

static void
pread_apply_noundo(struct picotm_error* error)
{ }

static void
pread_apply_2pl(struct picotm_error* error)
{ }

void
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
pread_undo_2pl(struct picotm_error* error)
{ }

void
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

ssize_t
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

void
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
pwrite_undo_2pl(struct picotm_error* error)
{ }

void
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

    ofd_wrlock_state(self->ofd, &self->rwstate, error);
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

static void
read_apply_noundo(struct regfile_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{ }

static void
read_apply_2pl(struct regfile_tx* self, int fildes, int cookie,
               struct picotm_error* error)
{
    self->ofd->data.regular.offset += self->rdtab[cookie].nbyte;

    off_t res = lseek(fildes, self->ofd->data.regular.offset, SEEK_SET);
    if (res == (off_t)-1) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

void
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
read_undo_2pl(struct picotm_error* error)
{ }

void
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

        ofd_wrlock_state(self->ofd, &self->rwstate, error);
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

ssize_t
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
    self->ofd->data.regular.offset = self->wrtab[cookie].off+len;

    off_t res = lseek(fildes, self->ofd->data.regular.offset, SEEK_SET);
    if (res == (off_t)-1) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

void
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
write_undo_2pl(struct picotm_error* error)
{ }

void
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
