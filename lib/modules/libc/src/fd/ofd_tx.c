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

#include "ofd_tx.h"
#include <assert.h>
#include <errno.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-tab.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "fcntloptab.h"
#include "ioop.h"
#include "iooptab.h"
#include "ofd.h"
#include "range.h"
#include "region.h"
#include "regiontab.h"
#include "seekop.h"
#include "seekoptab.h"
#include "fd_event.h"

static void
ofd_tx_2pl_release_locks(struct ofd_tx* self)
{
    assert(self);

    /* release all locks */

    struct region* region = self->modedata.tpl.locktab;
    const struct region* regionend = region+self->modedata.tpl.locktablen;

    while (region < regionend) {
        ofd_2pl_unlock_region(self->ofd,
                              region->offset,
                              region->nbyte,
                              &self->modedata.tpl.rwcountermap);
        ++region;
    }
}

void
ofd_tx_init(struct ofd_tx* self)
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
    self->type = PICOTM_LIBC_FILE_TYPE_OTHER;
    self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;

    /* PLP */

    picotm_rwstate_init(&self->modedata.tpl.rwstate);

    rwcountermap_init(&self->modedata.tpl.rwcountermap);

    self->modedata.tpl.locktab = NULL;
    self->modedata.tpl.locktablen = 0;
    self->modedata.tpl.locktabsiz = 0;
}

void
ofd_tx_uninit(struct ofd_tx* self)
{
    assert(self);

    free(self->modedata.tpl.locktab);
    rwcountermap_uninit(&self->modedata.tpl.rwcountermap);

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
ofd_tx_lock(struct ofd_tx* self)
{
    assert(self);
}

void
ofd_tx_unlock(struct ofd_tx* self)
{
    assert(self);
}

static void
validate_noundo(struct ofd_tx* self, struct picotm_error* error)
{ }

static void
validate_2pl(struct ofd_tx* self, struct picotm_error* error)
{
    assert(self);

    /* Locked regions are ours, so we do not need to validate here. All
     * conflicting transactions will have aborted on encountering our locks.
     *
     * The state of the OFD itself is guarded by ofd::rwlock.
     */
}

void
ofd_tx_validate(struct ofd_tx* self, struct picotm_error* error)
{
    static void (* const validate[])(struct ofd_tx*, struct picotm_error*) = {
        validate_noundo,
        validate_2pl
    };

    if (!ofd_tx_holds_ref(self)) {
        return;
    }

    validate[self->cc_mode](self, error);
}

/*
 * Update CC
 */

static void
update_cc_noundo(struct ofd_tx* self, struct picotm_error* error)
{ }

static void
update_cc_2pl(struct ofd_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_2PL);

    /* release record locks */

    if (self->type == PICOTM_LIBC_FILE_TYPE_REGULAR) {
        ofd_tx_2pl_release_locks(self);
    }

    /* release ofd lock */

    ofd_unlock_state(self->ofd, &self->modedata.tpl.rwstate);
}

void
ofd_tx_update_cc(struct ofd_tx* self, struct picotm_error* error)
{
    static void (* const update_cc[])(struct ofd_tx*, struct picotm_error*) = {
        update_cc_noundo,
        update_cc_2pl
    };

    assert(ofd_tx_holds_ref(self));

    update_cc[self->cc_mode](self, error);
}

/*
 * Clear CC
 */

static void
clear_cc_noundo(struct ofd_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO);
}

static void
clear_cc_2pl(struct ofd_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_2PL);

    /* release record locks */

    if (self->type == PICOTM_LIBC_FILE_TYPE_REGULAR) {
        ofd_tx_2pl_release_locks(self);
    }

    /* release ofd lock */

    ofd_unlock_state(self->ofd, &self->modedata.tpl.rwstate);
}

void
ofd_tx_clear_cc(struct ofd_tx* self, struct picotm_error* error)
{
    static void (* const clear_cc[])(struct ofd_tx*, struct picotm_error*) = {
        clear_cc_noundo,
        clear_cc_2pl
    };

    assert(ofd_tx_holds_ref(self));

    clear_cc[self->cc_mode](self, error);
}

/*
 * Referencing
 */

void
ofd_tx_ref_or_set_up(struct ofd_tx* self, struct ofd* ofd, int fildes,
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
    self->type = type;
    self->cc_mode = cc_mode;
    self->offset = offset;
    self->size = 0;
    self->flags = 0;

    self->fcntltablen = 0;
    self->seektablen = 0;
    self->rdtablen = 0;
    self->wrtablen = 0;
    self->wrbuflen = 0;

    picotm_rwstate_set_status(&self->modedata.tpl.rwstate,
                              PICOTM_RWSTATE_UNLOCKED);
    self->modedata.tpl.locktablen = 0;

    return;

err_ofd_ref_state:
    picotm_ref_down(&self->ref);
}

void
ofd_tx_ref(struct ofd_tx* self)
{
    picotm_ref_up(&self->ref);
}

void
ofd_tx_unref(struct ofd_tx* self)
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
ofd_tx_holds_ref(struct ofd_tx* self)
{
    assert(self);

    return picotm_ref_count(&self->ref) > 0;
}

static off_t
append_to_iobuffer(struct ofd_tx* self, size_t nbyte, const void* buf,
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
ofd_tx_append_to_writeset(struct ofd_tx* self, size_t nbyte, off_t offset,
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
ofd_tx_append_to_readset(struct ofd_tx* self, size_t nbyte, off_t offset,
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
 * Pessimistic CC
 */

int
ofd_tx_2pl_lock_region(struct ofd_tx* self, size_t nbyte, off_t offset,
                       int write, struct picotm_error* error)
{
    assert(self);

    ofd_2pl_lock_region(self->ofd,
                        offset,
                        nbyte,
                        write,
                        &self->modedata.tpl.rwcountermap,
                        error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int pos = regiontab_append(&self->modedata.tpl.locktab,
                               &self->modedata.tpl.locktablen,
                               &self->modedata.tpl.locktabsiz,
                               nbyte, offset,
                               error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    return pos;
}

#include <stdio.h>

void
ofd_tx_dump(const struct ofd_tx* self)
{
    fprintf(stderr, "%p: %p %p %zu %p %zu %p %zu %ld\n", (void*)self,
                                                         (void*)self->ofd,
                                                         (void*)self->seektab,
                                                                self->seektablen,
                                                         (void*)self->wrtab,
                                                                self->wrtablen,
                                                         (void*)self->rdtab,
                                                                self->rdtablen,
                                                                self->offset);
}

/*
 * bind()
 */

static int
bind_exec_noundo(struct ofd_tx* self, int sockfd,
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
ofd_tx_bind_exec(struct ofd_tx* self, int sockfd, const struct sockaddr* addr,
                 socklen_t addrlen, int* cookie, int noundo,
                 struct picotm_error* error)
{
    static int (* const bind_exec[][2])(struct ofd_tx*,
                                        int,
                                  const struct sockaddr*,
                                        socklen_t,
                                        int*, struct picotm_error* error) = {
        {bind_exec_noundo, NULL},
        {bind_exec_noundo, NULL},
        {bind_exec_noundo, NULL},
        {bind_exec_noundo, NULL}
    };

    assert(self->type < sizeof(bind_exec)/sizeof(bind_exec[0]));
    assert(bind_exec[self->type]);

    if (noundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !bind_exec[self->type][self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return bind_exec[self->type][self->cc_mode](self,
                                                sockfd,
                                                addr,
                                                addrlen,
                                                cookie,
                                                error);
}

static void
bind_apply_noundo(struct ofd_tx* self, int sockfd,
                  const struct fd_event* event,
                  struct picotm_error* error)
{ }

void
ofd_tx_bind_apply(struct ofd_tx* self, int sockfd,
                  const struct fd_event* event,
                  struct picotm_error* error)
{
    static void (* const bind_apply[][2])(struct ofd_tx*,
                                          int,
                                          const struct fd_event*,
                                          struct picotm_error*) = {
        {bind_apply_noundo, NULL},
        {bind_apply_noundo, NULL},
        {bind_apply_noundo, NULL},
        {bind_apply_noundo, NULL}
    };

    assert(self->type < sizeof(bind_apply)/sizeof(bind_apply[0]));
    assert(bind_apply[self->type]);

    bind_apply[self->type][self->cc_mode](self, sockfd, event, error);
}

void
ofd_tx_bind_undo(struct ofd_tx* self, int sockfd, int cookie,
                 struct picotm_error* error)
{
    static void (* const bind_undo[][2])(int, struct picotm_error*) = {
        {NULL, NULL},
        {NULL, NULL},
        {NULL, NULL},
        {NULL, NULL}
    };

    assert(self->type < sizeof(bind_undo)/sizeof(bind_undo[0]));
    assert(bind_undo[self->type]);

    bind_undo[self->type][self->cc_mode](cookie, error);
}

/*
 * connect()
 */

static int
connect_exec_noundo(struct ofd_tx* self, int sockfd,
                    const struct sockaddr* serv_addr,
                    socklen_t addrlen, int* cookie,
                    struct picotm_error* error)
{
    int res = TEMP_FAILURE_RETRY(connect(sockfd, serv_addr, addrlen));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

int
ofd_tx_connect_exec(struct ofd_tx* self, int sockfd,
                    const struct sockaddr* serv_addr, socklen_t addrlen,
                    int* cookie, int noundo, struct picotm_error* error)
{
    static int (* const connect_exec[][2])(struct ofd_tx*,
                                           int,
                                           const struct sockaddr*,
                                           socklen_t,
                                           int*,
                                           struct picotm_error*) = {
        {connect_exec_noundo, NULL},
        {connect_exec_noundo, NULL},
        {connect_exec_noundo, NULL},
        {connect_exec_noundo, NULL}
    };

    assert(self->type < sizeof(connect_exec)/sizeof(connect_exec[0]));
    assert(connect_exec[self->type]);

    if (noundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !connect_exec[self->type][self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return connect_exec[self->type][self->cc_mode](self,
                                                   sockfd,
                                                   serv_addr,
                                                   addrlen,
                                                   cookie,
                                                   error);
}

static void
connect_apply_noundo(struct ofd_tx* self, int sockfd,
                     const struct fd_event* event,
                     struct picotm_error* error)
{ }

void
ofd_tx_connect_apply(struct ofd_tx* self, int sockfd,
                     const struct fd_event* event,
                     struct picotm_error* error)
{
    static void (* const connect_apply[][2])(struct ofd_tx*,
                                             int,
                                             const struct fd_event*,
                                             struct picotm_error*) = {
        {connect_apply_noundo, NULL},
        {connect_apply_noundo, NULL},
        {connect_apply_noundo, NULL},
        {connect_apply_noundo, NULL}
    };

    assert(self->type < sizeof(connect_apply)/sizeof(connect_apply[0]));
    assert(connect_apply[self->type]);

    connect_apply[self->type][self->cc_mode](self, sockfd, event, error);
}

void
ofd_tx_connect_undo(struct ofd_tx* self, int sockfd, int cookie,
                    struct picotm_error* error)
{
    static void (* const connect_undo[][2])(struct ofd_tx*,
                                            int,
                                            int,
                                            struct picotm_error*) = {
        {NULL, NULL},
        {NULL, NULL},
        {NULL, NULL},
        {NULL, NULL}
    };

    assert(self->type < sizeof(connect_undo)/sizeof(connect_undo[0]));
    assert(connect_undo[self->type]);

    connect_undo[self->type][self->cc_mode](self, sockfd, cookie, error);
}

/*
 * fcntl()
 */

static int
fcntl_exec_noundo(struct ofd_tx* self, int fildes, int cmd,
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
fcntl_exec_2pl(struct ofd_tx* self, int fildes, int cmd,
               union fcntl_arg* arg, int* cookie, struct picotm_error* error)
{
    assert(arg);

    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN: {

            /* Read-lock open file description */
            ofd_rdlock_state(self->ofd, &self->modedata.tpl.rwstate, error);
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
            ofd_rdlock_state(self->ofd, &self->modedata.tpl.rwstate, error);
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
ofd_tx_fcntl_exec(struct ofd_tx* self, int fildes, int cmd,
                  union fcntl_arg* arg, int* cookie, int noundo,
                  struct picotm_error* error)
{
    static int (* const fcntl_exec[][2])(struct ofd_tx*,
                                         int,
                                         int,
                                         union fcntl_arg*,
                                         int*,
                                         struct picotm_error*) = {
        {fcntl_exec_noundo, NULL},
        {fcntl_exec_noundo, fcntl_exec_2pl},
        {fcntl_exec_noundo, fcntl_exec_2pl},
        {fcntl_exec_noundo, NULL}
    };

    assert(self->type < sizeof(fcntl_exec)/sizeof(fcntl_exec[0]));
    assert(fcntl_exec[self->type]);

    if (noundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !fcntl_exec[self->type][self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return fcntl_exec[self->type][self->cc_mode](self, fildes, cmd, arg,
                                                 cookie, error);
}

void
ofd_tx_fcntl_apply(struct ofd_tx* self, int fildes,
                   const struct fd_event* event,
                   struct picotm_error* error)
{ }

void
ofd_tx_fcntl_undo(struct ofd_tx* self, int fildes, int cookie,
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
fsync_exec_regular_2pl(int fildes, int* cookie, struct picotm_error* error)
{
    /* Signal apply/undo */
    *cookie = 0;

    return 0;
}

int
ofd_tx_fsync_exec(struct ofd_tx* self, int fildes, int noundo, int* cookie,
                  struct picotm_error* error)
{
    static int (* const fsync_exec[][2])(int, int*, struct picotm_error*) = {
        {fsync_exec_noundo, NULL},
        {fsync_exec_noundo, fsync_exec_regular_2pl},
        {fsync_exec_noundo, NULL},
        {fsync_exec_noundo, NULL}
    };

    assert(self->type < sizeof(fsync_exec)/sizeof(fsync_exec[0]));

    if (noundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !fsync_exec[self->type][self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return fsync_exec[self->type][self->cc_mode](fildes, cookie, error);
}

static void
fsync_apply_noundo(int fildes, struct picotm_error* error)
{ }

static void
fsync_apply_regular_2pl(int fildes, struct picotm_error* error)
{
    int res = fsync(fildes);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

void
ofd_tx_fsync_apply(struct ofd_tx* self, int fildes,
                   const struct fd_event* event,
                   struct picotm_error* error)
{
    static void (* const fsync_apply[][2])(int, struct picotm_error*) = {
        {fsync_apply_noundo, NULL},
        {fsync_apply_noundo, fsync_apply_regular_2pl},
        {fsync_apply_noundo, NULL},
        {fsync_apply_noundo, NULL}
    };

    assert(self->type < sizeof(fsync_apply)/sizeof(fsync_apply[0]));
    assert(fsync_apply[self->type][self->cc_mode]);

    fsync_apply[self->type][self->cc_mode](fildes, error);
}

static void
fsync_undo_regular_2pl(int fildes, int cookie, struct picotm_error* error)
{ }

void
ofd_tx_fsync_undo(struct ofd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static void (* const fsync_undo[][2])(int, int, struct picotm_error*) = {
        {NULL, NULL},
        {NULL, fsync_undo_regular_2pl},
        {NULL, NULL},
        {NULL, NULL}
    };

    assert(self->type < sizeof(fsync_undo)/sizeof(fsync_undo[0]));
    assert(fsync_undo[self->type][self->cc_mode]);

    fsync_undo[self->type][self->cc_mode](fildes, cookie, error);
}

/*
 * listen()
 */

static int
listen_exec_noundo(struct ofd_tx* self, int sockfd, int backlog, int* cookie,
                   struct picotm_error* error)
{
    int res = listen(sockfd, backlog);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

int
ofd_tx_listen_exec(struct ofd_tx* self, int sockfd, int backlog, int* cookie,
                   int noundo, struct picotm_error* error)
{
    static int (* const listen_exec[][2])(struct ofd_tx*,
                                          int,
                                          int,
                                          int*,
                                          struct picotm_error*) = {
        {listen_exec_noundo, NULL},
        {listen_exec_noundo, NULL},
        {listen_exec_noundo, NULL},
        {listen_exec_noundo, NULL}
    };

    assert(self->type < sizeof(listen_exec)/sizeof(listen_exec[0]));
    assert(listen_exec[self->type]);

    if (noundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !listen_exec[self->type][self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return listen_exec[self->type][self->cc_mode](self, sockfd, backlog,
                                                  cookie, error);
}

static void
listen_apply_noundo(struct picotm_error* error)
{ }

void
ofd_tx_listen_apply(struct ofd_tx* self, int sockfd,
                    const struct fd_event* event,
                    struct picotm_error* error)
{
    static void (* const listen_apply[][2])(struct picotm_error*) = {
        {listen_apply_noundo, NULL},
        {listen_apply_noundo, NULL},
        {listen_apply_noundo, NULL},
        {listen_apply_noundo, NULL}
    };

    assert(self->type < sizeof(listen_apply)/sizeof(listen_apply[0]));
    assert(listen_apply[self->type]);

    listen_apply[self->type][self->cc_mode](error);
}

void
ofd_tx_listen_undo(struct ofd_tx* self, int sockfd, int cookie,
                   struct picotm_error* error)
{
    static int (* const listen_undo[][2])(struct ofd_tx*,
                                          int,
                                          int,
                                          struct picotm_error*) = {
        {NULL, NULL},
        {NULL, NULL},
        {NULL, NULL},
        {NULL, NULL}
    };

    assert(self->type < sizeof(listen_undo)/sizeof(listen_undo[0]));
    assert(listen_undo[self->type]);

    listen_undo[self->type][self->cc_mode](self, sockfd, cookie, error);
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
lseek_exec_noundo(struct ofd_tx* self, int fildes, off_t offset,
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
lseek_exec_regular_2pl(struct ofd_tx* self, int fildes, off_t offset,
                       int whence, int* cookie, struct picotm_error* error)
{
    /* Read-lock open file description */

    ofd_rdlock_state(self->ofd, &self->modedata.tpl.rwstate, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

	/* Fastpath: Read current position */
	if (!offset && (whence == SEEK_CUR)) {
		return self->offset;
	}

    /* Write-lock open file description to change position */

    ofd_wrlock_state(self->ofd, &self->modedata.tpl.rwstate, error);
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
ofd_tx_lseek_exec(struct ofd_tx* self, int fildes,  off_t offset, int whence,
                  int* cookie, int noundo, struct picotm_error* error)
{
    static off_t (* const lseek_exec[][2])(struct ofd_tx*,
                                           int,
                                           off_t,
                                           int,
                                           int*,
                                           struct picotm_error*) = {
        {lseek_exec_noundo, NULL},
        {lseek_exec_noundo, lseek_exec_regular_2pl},
        {lseek_exec_noundo, NULL},
        {lseek_exec_noundo, NULL}
    };

    assert(self->type < sizeof(lseek_exec)/sizeof(lseek_exec[0]));
    assert(lseek_exec[self->type]);

    if (noundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !lseek_exec[self->type][self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return lseek_exec[self->type][self->cc_mode](self, fildes, offset,
                                                 whence, cookie, error);
}

static void
lseek_apply_noundo(struct ofd_tx* self, int fildes,
                   const struct fd_event* event,
                   struct picotm_error* error)
{ }

static void
lseek_apply_regular(struct ofd_tx* self, int fildes,
                    const struct fd_event* event,
                    struct picotm_error* error)
{
    const off_t pos = lseek(fildes, self->seektab[event->cookie].offset,
                                    self->seektab[event->cookie].whence);

    if (pos == (off_t)-1) {
        picotm_error_set_errno(error, errno);
        return;
    }

    self->ofd->data.regular.offset = pos;
}

void
ofd_tx_lseek_apply(struct ofd_tx* self, int fildes,
                   const struct fd_event* event,
                   struct picotm_error* error)
{
    static void (* const lseek_apply[][2])(struct ofd_tx*,
                                           int,
                                           const struct fd_event*,
                                           struct picotm_error*) = {
        {lseek_apply_noundo, NULL},
        {lseek_apply_noundo, lseek_apply_regular},
        {lseek_apply_noundo, NULL},
        {lseek_apply_noundo, NULL}
    };

    assert(self->type < sizeof(lseek_apply)/sizeof(lseek_apply[0]));
    assert(lseek_apply[self->type][self->cc_mode]);

    lseek_apply[self->type][self->cc_mode](self, fildes, event, error);
}

static void
lseek_undo_regular(struct picotm_error* error)
{ }

void
ofd_tx_lseek_undo(struct ofd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static void (* const lseek_undo[][2])(struct picotm_error*) = {
        {NULL, NULL},
        {NULL, lseek_undo_regular},
        {NULL, NULL},
        {NULL, NULL}
    };

    assert(self->type < sizeof(lseek_undo)/sizeof(lseek_undo[0]));
    assert(lseek_undo[self->type][self->cc_mode]);

    lseek_undo[self->type][self->cc_mode](error);
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
pread_exec_noundo(struct ofd_tx* self, int fildes, void* buf,
                  size_t nbyte, off_t offset, int *cookie,
                  enum picotm_libc_validation_mode val_mode,
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
pread_exec_regular_2pl(struct ofd_tx* self, int fildes, void* buf,
                       size_t nbyte, off_t offset, int* cookie,
                       enum picotm_libc_validation_mode val_mode,
                       struct picotm_error* error)
{
    /* lock region */
    ofd_tx_2pl_lock_region(self, nbyte, offset, 0, error);
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
pread_exec_fifo(struct ofd_tx* self, int fildes, void *buf, size_t nbyte,
                off_t offset, int* cookie,
                enum picotm_libc_validation_mode val_mode,
                struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return -1;
}

ssize_t
ofd_tx_pread_exec(struct ofd_tx* self, int fildes, void* buf, size_t nbyte,
                  off_t offset, int* cookie, int noundo,
                  enum picotm_libc_validation_mode val_mode,
                  struct picotm_error* error)
{
    static ssize_t (* const pread_exec[][2])(struct ofd_tx*,
                                             int,
                                             void*,
                                             size_t,
                                             off_t,
                                             int*,
                                             enum picotm_libc_validation_mode,
                                             struct picotm_error*) = {
        {pread_exec_noundo, NULL},
        {pread_exec_noundo, pread_exec_regular_2pl},
        {pread_exec_noundo, pread_exec_fifo},
        {pread_exec_noundo, NULL}
    };

    assert(self->type < sizeof(pread_exec)/sizeof(pread_exec[0]));
    assert(pread_exec[self->type]);

    if (noundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !pread_exec[self->type][self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return pread_exec[self->type][self->cc_mode](self, fildes, buf, nbyte,
                                                 offset, cookie, val_mode,
                                                 error);
}

static void
pread_apply_any(struct picotm_error* error)
{ }

void
ofd_tx_pread_apply(struct ofd_tx* self, int fildes,
                   const struct fd_event* event,
                   struct picotm_error* error)
{
    static void (* const pread_apply[][2])(struct picotm_error*) = {
        {pread_apply_any, NULL},
        {pread_apply_any, pread_apply_any},
        {pread_apply_any, NULL},
        {pread_apply_any, NULL}
    };

    assert(self->type < sizeof(pread_apply)/sizeof(pread_apply[0]));
    assert(pread_apply[self->type]);

    pread_apply[self->type][self->cc_mode](error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
pread_undo_any(struct picotm_error* error)
{ }

void
ofd_tx_pread_undo(struct ofd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static void (* const pread_undo[][2])(struct picotm_error*) = {
        {NULL, NULL},
        {NULL, pread_undo_any},
        {NULL, NULL},
        {NULL, NULL}
    };

    assert(self->type < sizeof(pread_undo)/sizeof(pread_undo[0]));
    assert(pread_undo[self->type]);

    pread_undo[self->type][self->cc_mode](error);
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
pwrite_exec_noundo(struct ofd_tx* self, int fildes, const void* buf,
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
pwrite_exec_regular_2pl(struct ofd_tx* self, int fildes,
                        const void* buf, size_t nbyte, off_t offset,
                        int* cookie, struct picotm_error* error)
{
    /* register written data */

    if (__builtin_expect(!!cookie, 1)) {

        /* lock region */

        ofd_tx_2pl_lock_region(self, nbyte, offset, 1, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }

        /* add data to write set */
        *cookie = ofd_tx_append_to_writeset(self, nbyte, offset, buf, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return nbyte;
}

ssize_t
ofd_tx_pwrite_exec(struct ofd_tx* self, int fildes, const void* buf,
                   size_t nbyte, off_t offset, int* cookie, int noundo,
                   struct picotm_error* error)
{
    static ssize_t (* const pwrite_exec[][2])(struct ofd_tx*,
                                              int,
                                              const void*,
                                              size_t,
                                              off_t,
                                              int*,
                                              struct picotm_error*) = {
        {pwrite_exec_noundo, NULL},
        {pwrite_exec_noundo, pwrite_exec_regular_2pl},
        {pwrite_exec_noundo, NULL},
        {pwrite_exec_noundo, NULL}
    };

    assert(self->type < sizeof(pwrite_exec)/sizeof(pwrite_exec[0]));
    assert(pwrite_exec[self->type]);

    if (noundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !pwrite_exec[self->type][self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return pwrite_exec[self->type][self->cc_mode](self, fildes, buf, nbyte,
                                                  offset, cookie, error);
}

static void
pwrite_apply_noundo(struct ofd_tx* self, int fildes,
                    const struct fd_event* event,
                    struct picotm_error* error)
{ }

static void
pwrite_apply_regular(struct ofd_tx* self, int fildes,
                     const struct fd_event* event,
                     struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(event);

    off_t  off = self->wrtab[event->cookie].off;
    size_t nbyte = self->wrtab[event->cookie].nbyte;
    off_t  bufoff = self->wrtab[event->cookie].bufoff;

    ssize_t res = TEMP_FAILURE_RETRY(pwrite(fildes,
                                            self->wrbuf + bufoff,
                                            nbyte, off));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

void
ofd_tx_pwrite_apply(struct ofd_tx* self, int fildes,
                    const struct fd_event* event,
                    struct picotm_error* error)
{
    static void (* const pwrite_apply[][2])(struct ofd_tx*,
                                            int,
                                            const struct fd_event*,
                                            struct picotm_error*) = {
        {pwrite_apply_noundo, NULL},
        {pwrite_apply_noundo, pwrite_apply_regular},
        {pwrite_apply_noundo, NULL},
        {pwrite_apply_noundo, NULL}
    };

    assert(self->type < sizeof(pwrite_apply)/sizeof(pwrite_apply[0]));
    assert(pwrite_apply[self->type]);

    pwrite_apply[self->type][self->cc_mode](self, fildes, event, error);
}

static void
pwrite_any_undo(struct picotm_error* error)
{ }

void
ofd_tx_pwrite_undo(struct ofd_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{
    static void (* const pwrite_undo[][2])(struct picotm_error*) = {
        {NULL, NULL},
        {NULL, pwrite_any_undo},
        {NULL, NULL},
        {NULL, NULL}
    };

    assert(self->type < sizeof(pwrite_undo)/sizeof(pwrite_undo[0]));
    assert(pwrite_undo[self->type]);

    pwrite_undo[self->type][self->cc_mode](error);
}

/*
 * read()
 */

static ssize_t
read_exec_noundo(struct ofd_tx* self, int fildes, void* buf,
                 size_t nbyte, int* cookie,
                 enum picotm_libc_validation_mode val_mode,
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
read_exec_regular_2pl(struct ofd_tx* self, int fildes, void* buf,
                      size_t nbyte, int* cookie,
                      enum picotm_libc_validation_mode val_mode,
                      struct picotm_error* error)
{
    assert(self);

    /* write-lock open file description, because we change the file position */

    ofd_wrlock_state(self->ofd, &self->modedata.tpl.rwstate, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* read-lock region */

    ofd_tx_2pl_lock_region(self, nbyte, self->offset, 0, error);
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
ofd_tx_read_exec(struct ofd_tx* self, int fildes, void* buf, size_t nbyte,
                 int* cookie, int noundo,
                 enum picotm_libc_validation_mode val_mode,
                 struct picotm_error* error)
{
    static ssize_t (* const read_exec[][2])(struct ofd_tx*,
                                            int,
                                            void*,
                                            size_t,
                                            int*,
                                            enum picotm_libc_validation_mode,
                                            struct picotm_error*) = {
        {read_exec_noundo, NULL},
        {read_exec_noundo, read_exec_regular_2pl},
        {read_exec_noundo, NULL},
        {read_exec_noundo, NULL}
    };

    assert(self->type < sizeof(read_exec)/sizeof(read_exec[0]));
    assert(read_exec[self->type]);

    if (noundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !read_exec[self->type][self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return read_exec[self->type][self->cc_mode](self, fildes, buf, nbyte,
                                                cookie, val_mode, error);
}

static void
read_apply_noundo(struct ofd_tx* self, int fildes,
                  const struct fd_event* event,
                  struct picotm_error* error)
{ }

static void
read_apply_regular(struct ofd_tx* self, int fildes,
                   const struct fd_event* event,
                   struct picotm_error* error)
{
    self->ofd->data.regular.offset += self->rdtab[event->cookie].nbyte;

    off_t res = lseek(fildes, self->ofd->data.regular.offset, SEEK_SET);
    if (res == (off_t)-1) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

void
ofd_tx_read_apply(struct ofd_tx* self, int fildes,
                  const struct fd_event* event,
                  struct picotm_error* error)
{
    static void (* const read_apply[][2])(struct ofd_tx*,
                                          int,
                                          const struct fd_event*,
                                          struct picotm_error*) = {
        {read_apply_noundo, NULL},
        {read_apply_noundo, read_apply_regular},
        {read_apply_noundo, NULL},
        {read_apply_noundo, NULL}
    };

    assert(self->type < sizeof(read_apply)/sizeof(read_apply[0]));
    assert(read_apply[self->type]);

    read_apply[self->type][self->cc_mode](self, fildes, event, error);
}

static void
read_any_undo(struct picotm_error* error)
{ }

void
ofd_tx_read_undo(struct ofd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    static void (* const read_undo[][2])(struct picotm_error*) = {
        {NULL, NULL},
        {NULL, read_any_undo},
        {NULL, NULL},
        {NULL, NULL}
    };

    assert(self->type < sizeof(read_undo)/sizeof(read_undo[0]));
    assert(read_undo[self->type]);

    read_undo[self->type][self->cc_mode](error);
}

/*
 * recv()
 */

static ssize_t
recv_exec_noundo(struct ofd_tx* self, int sockfd, void* buffer,
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
ofd_tx_recv_exec(struct ofd_tx* self, int sockfd, void* buffer,
                 size_t length, int flags, int* cookie, int noundo,
                 struct picotm_error* error)
{
    static ssize_t (* const recv_exec[][2])(struct ofd_tx*,
                                            int,
                                            void*,
                                            size_t,
                                            int,
                                            int*,
                                            struct picotm_error*) = {
        {recv_exec_noundo, NULL},
        {recv_exec_noundo, NULL},
        {recv_exec_noundo, NULL},
        {recv_exec_noundo, NULL}
    };

    assert(self->type < sizeof(recv_exec)/sizeof(recv_exec[0]));
    assert(recv_exec[self->type]);

    if (noundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !recv_exec[self->type][self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return recv_exec[self->type][self->cc_mode](self, sockfd, buffer, length,
                                                flags, cookie, error);
}

static void
recv_apply_noundo(struct picotm_error* error)
{ }

void
ofd_tx_recv_apply(struct ofd_tx* self, int sockfd,
                  const struct fd_event* event,
                  struct picotm_error* error)
{
    static void (* const recv_apply[][2])(struct picotm_error*) = {
        {recv_apply_noundo, NULL},
        {recv_apply_noundo, NULL},
        {recv_apply_noundo, NULL},
        {recv_apply_noundo, NULL}
    };

    assert(self->type < sizeof(recv_apply)/sizeof(recv_apply[0]));
    assert(recv_apply[self->type]);

    recv_apply[self->type][self->cc_mode](error);
}

void
ofd_tx_recv_undo(struct ofd_tx* self, int sockfd, int cookie,
                 struct picotm_error* error)
{
    static void (* const recv_undo[][2])(struct ofd_tx*,
                                         int,
                                         int,
                                         struct picotm_error*) = {
        {NULL, NULL},
        {NULL, NULL},
        {NULL, NULL},
        {NULL, NULL}
    };

    assert(self->type < sizeof(recv_undo)/sizeof(recv_undo[0]));
    assert(recv_undo[self->type]);

    recv_undo[self->type][self->cc_mode](self, sockfd, cookie, error);
}

/*
 * send()
 */

static ssize_t
send_exec_noundo(struct ofd_tx* self, int sockfd, const void* buffer,
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
send_exec_socket_2pl(struct ofd_tx* self, int sockfd, const void* buf,
                     size_t nbyte, int flags, int* cookie,
                     struct picotm_error* error)
{
    /* Become irrevocable if any flags are selected */
    if (flags) {
        picotm_error_set_revocable(error);
        return -1;
    }

    /* Write-lock open file description, because we change the file position */

    ofd_wrlock_state(self->ofd, &self->modedata.tpl.rwstate, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Register write data */

    if (cookie) {
        *cookie = ofd_tx_append_to_writeset(self, nbyte, 0, buf, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return nbyte;
}

ssize_t
ofd_tx_send_exec(struct ofd_tx* self, int sockfd, const void* buffer,
                 size_t length, int flags, int* cookie, int noundo,
                 struct picotm_error* error)
{
    static ssize_t (* const send_exec[][2])(struct ofd_tx*,
                                            int,
                                      const void*,
                                            size_t,
                                            int,
                                            int*,
                                            struct picotm_error*) = {
        {send_exec_noundo, NULL},
        {send_exec_noundo, NULL},
        {send_exec_noundo, NULL},
        {send_exec_noundo, send_exec_socket_2pl}
    };

    assert(self->type < sizeof(send_exec)/sizeof(send_exec[0]));
    assert(send_exec[self->type]);

    if (noundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !send_exec[self->type][self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return send_exec[self->type][self->cc_mode](self, sockfd, buffer, length,
                                                flags, cookie, error);
}

static void
send_apply_noundo(struct ofd_tx* self, int sockfd, int cookie,
                  struct picotm_error* error)
{ }

static void
send_apply_socket_2pl(struct ofd_tx* self, int sockfd, int cookie,
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
ofd_tx_send_apply(struct ofd_tx* self, int sockfd,
                  const struct fd_event* event,
                  struct picotm_error* error)
{
    static void (* const send_apply[][2])(struct ofd_tx*,
                                          int,
                                          int,
                                          struct picotm_error*) = {
        {send_apply_noundo, NULL},
        {send_apply_noundo, NULL},
        {send_apply_noundo, NULL},
        {send_apply_noundo, send_apply_socket_2pl}
    };

    assert(self->type < sizeof(send_apply)/sizeof(send_apply[0]));
    assert(send_apply[self->type][self->cc_mode]);

    send_apply[self->type][self->cc_mode](self, sockfd,
                                          event->cookie,
                                          error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
send_undo_socket_2pl(struct picotm_error* error)
{ }

void
ofd_tx_send_undo(struct ofd_tx* self, int sockfd, int cookie,
                 struct picotm_error* error)
{
    static void (* const send_undo[][2])(struct picotm_error*) = {
        {NULL, NULL},
        {NULL, NULL},
        {NULL, NULL},
        {NULL, send_undo_socket_2pl}
    };

    assert(self->type < sizeof(send_undo)/sizeof(send_undo[0]));
    assert(send_undo[self->type][self->cc_mode]);

    send_undo[self->type][self->cc_mode](error);
}

/*
 * shutdown()
 */

static int
shutdown_exec_noundo(struct ofd_tx* self, int sockfd, int how,
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
ofd_tx_shutdown_exec(struct ofd_tx* self, int sockfd, int how, int* cookie,
                     int noundo, struct picotm_error* error)
{
    static int (* const shutdown_exec[][2])(struct ofd_tx*,
                                            int,
                                            int,
                                            int*,
                                            struct picotm_error*) = {
        {shutdown_exec_noundo, NULL},
        {shutdown_exec_noundo, NULL},
        {shutdown_exec_noundo, NULL},
        {shutdown_exec_noundo, NULL}
    };

    assert(self->type < sizeof(shutdown_exec)/sizeof(shutdown_exec[0]));
    assert(shutdown_exec[self->type]);

    if (noundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !shutdown_exec[self->type][self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return shutdown_exec[self->type][self->cc_mode](self, sockfd, how,
                                                    cookie, error);
}

static void
shutdown_apply_noundo(struct ofd_tx* self, int sockfd,
                      const struct fd_event* event,
                      struct picotm_error* error)
{ }

void
ofd_tx_shutdown_apply(struct ofd_tx* self, int sockfd,
                      const struct fd_event* event,
                      struct picotm_error* error)
{
    static void (* const shutdown_apply[][2])(struct ofd_tx*,
                                              int,
                                              const struct fd_event*,
                                              struct picotm_error*) = {
        {shutdown_apply_noundo, NULL},
        {shutdown_apply_noundo, NULL},
        {shutdown_apply_noundo, NULL},
        {shutdown_apply_noundo, NULL}
    };

    assert(self->type < sizeof(shutdown_apply)/sizeof(shutdown_apply[0]));
    assert(shutdown_apply[self->type]);

    shutdown_apply[self->type][self->cc_mode](self, sockfd, event, error);
}

void
ofd_tx_shutdown_undo(struct ofd_tx* self, int sockfd, int cookie,
                     struct picotm_error* error)
{
    static int (* const shutdown_undo[][2])(struct ofd_tx*,
                                            int,
                                            int,
                                            struct picotm_error*) = {
        {NULL, NULL},
        {NULL, NULL},
        {NULL, NULL},
        {NULL, NULL}
    };

    assert(self->type < sizeof(shutdown_undo)/sizeof(shutdown_undo[0]));
    assert(shutdown_undo[self->type]);

    shutdown_undo[self->type][self->cc_mode](self, sockfd, cookie, error);
}

/*
 * write()
 */

static ssize_t
write_exec_noundo(struct ofd_tx* self, int fildes, const void* buf,
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
write_exec_regular_2pl(struct ofd_tx* self, int fildes, const void* buf,
                       size_t nbyte, int* cookie, struct picotm_error* error)
{
    /* register written data */

    if (__builtin_expect(!!cookie, 1)) {

        /* write-lock open file description, because we change the file position */

        ofd_wrlock_state(self->ofd, &self->modedata.tpl.rwstate, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }

        /* write-lock region */

        ofd_tx_2pl_lock_region(self, nbyte, self->offset, 1, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }

        /* add buf to write set */
        *cookie = ofd_tx_append_to_writeset(self, nbyte, self->offset, buf,
                                            error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    /* update file pointer */
    self->offset += nbyte;

    return nbyte;
}

static ssize_t
write_exec_fifo_2pl(struct ofd_tx* self, int fildes, const void* buf,
                    size_t nbyte, int* cookie, struct picotm_error* error)
{
    /* Write-lock open file description, because we change the file position */

    ofd_wrlock_state(self->ofd, &self->modedata.tpl.rwstate, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Register write data */

    if (cookie) {
        *cookie = ofd_tx_append_to_writeset(self, nbyte, 0, buf, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return nbyte;
}

static ssize_t
write_exec_socket_2pl(struct ofd_tx* self, int fildes, const void* buf,
                      size_t nbyte, int* cookie, struct picotm_error* error)
{
    /* Write-lock open file description, because we change the file position */

    ofd_wrlock_state(self->ofd, &self->modedata.tpl.rwstate, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Register write data */

    if (cookie) {
        *cookie = ofd_tx_append_to_writeset(self, nbyte, 0, buf, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return nbyte;
}

ssize_t
ofd_tx_write_exec(struct ofd_tx* self, int fildes, const void* buf,
                  size_t nbyte, int* cookie, int noundo,
                  struct picotm_error* error)
{
    static ssize_t (* const write_exec[][2])(struct ofd_tx*,
                                             int,
                                             const void*,
                                             size_t,
                                             int*,
                                             struct picotm_error*) = {
        {write_exec_noundo, NULL},
        {write_exec_noundo, write_exec_regular_2pl},
        {write_exec_noundo, write_exec_fifo_2pl},
        {write_exec_noundo, write_exec_socket_2pl}
    };

    assert(self->type < sizeof(write_exec)/sizeof(write_exec[0]));
    assert(write_exec[self->type]);

    if (noundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !write_exec[self->type][self->cc_mode]) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    return write_exec[self->type][self->cc_mode](self, fildes, buf, nbyte,
                                                 cookie, error);
}

static void
write_apply_noundo(struct ofd_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{ }

static void
write_apply_regular_2pl(struct ofd_tx* self, int fildes, int cookie,
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

static void
write_apply_fifo(struct ofd_tx* self, int fildes, int cookie,
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

static void
write_apply_socket(struct ofd_tx* self, int sockfd, int cookie,
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
ofd_tx_write_apply(struct ofd_tx* self, int fildes,
                   const struct fd_event* event,
                   struct picotm_error* error)
{
    static void (* const write_apply[][2])(struct ofd_tx*,
                                           int,
                                           int,
                                           struct picotm_error*) = {
        {write_apply_noundo, NULL},
        {write_apply_noundo, write_apply_regular_2pl},
        {write_apply_noundo, write_apply_fifo},
        {write_apply_noundo, write_apply_socket}
    };

    assert(self->type < sizeof(write_apply)/sizeof(write_apply[0]));
    assert(write_apply[self->type][self->cc_mode]);

    write_apply[self->type][self->cc_mode](self, fildes, event->cookie,
                                           error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
write_any_undo(struct picotm_error* error)
{ }

void
ofd_tx_write_undo(struct ofd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static void (* const write_undo[][2])(struct picotm_error*) = {
        {NULL, NULL},
        {NULL, write_any_undo},
        {NULL, write_any_undo},
        {NULL, write_any_undo}
    };

    assert(self->type < sizeof(write_undo)/sizeof(write_undo[0]));
    assert(write_undo[self->type][self->cc_mode]);

    write_undo[self->type][self->cc_mode](error);
}
