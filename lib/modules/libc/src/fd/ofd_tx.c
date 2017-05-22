/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ofd_tx.h"
#include <assert.h>
#include <errno.h>
#include <picotm/picotm-module.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "errcode.h"
#include "fcntloptab.h"
#include "ioop.h"
#include "iooptab.h"
#include "ofdtab.h"
#include "range.h"
#include "region.h"
#include "regiontab.h"
#include "seekop.h"
#include "seekoptab.h"
#include "fd_event.h"

static int
ofd_tx_2pl_release_locks(struct ofd_tx* self)
{
    struct ofd *ofd;
    struct region *region;
    const struct region *regionend;

    assert(self);

    ofd = ofdtab+self->ofd;

    /* release all locks */

    region = self->modedata.tpl.locktab;
    regionend = region+self->modedata.tpl.locktablen;

    while (region < regionend) {
        ofd_2pl_unlock_region(ofd,
                              region->offset,
                              region->nbyte,
                              &self->modedata.tpl.rwstatemap);
        ++region;
    }

    return 0;
}

int
ofd_tx_init(struct ofd_tx* self)
{
    int err;

    assert(self);

    self->ofd = -1;

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

    self->modedata.tpl.rwstate = RW_NOLOCK;

    if ((err = rwstatemap_init(&self->modedata.tpl.rwstatemap)) < 0) {
        return err;
    }

    self->modedata.tpl.locktab = NULL;
    self->modedata.tpl.locktablen = 0;
    self->modedata.tpl.locktabsiz = 0;

    return 0;
}

void
ofd_tx_uninit(struct ofd_tx* self)
{
    assert(self);

    free(self->modedata.tpl.locktab);
    rwstatemap_uninit(&self->modedata.tpl.rwstatemap);

    fcntloptab_clear(&self->fcntltab, &self->fcntltablen);
    seekoptab_clear(&self->seektab, &self->seektablen);
    iooptab_clear(&self->wrtab, &self->wrtablen);
    iooptab_clear(&self->rdtab, &self->rdtablen);
    free(self->wrbuf);
}

/*
 * Validation
 */

static int
validate_noundo(struct ofd_tx* self, struct picotm_error* error)
{
    return 0;
}

static int
validate_2pl(struct ofd_tx* self, struct picotm_error* error)
{
    assert(self);

    /* Locked regions are ours, so we do not need to validate here. All
     * conflicting transactions will have aborted on encountering our locks.
     *
     * The state of the OFD itself is guarded by ofd::rwlock.
     */
    return 0;
}

int
ofd_tx_validate(struct ofd_tx* self, struct picotm_error* error)
{
    static int (* const validate[])(struct ofd_tx*, struct picotm_error*) = {
        validate_noundo,
        validate_2pl
    };

    if (!ofd_tx_holds_ref(self)) {
        return 0;
    }

    return validate[self->cc_mode](self, error);
}

/*
 * Update CC
 */

static int
update_cc_noundo(struct ofd_tx* self, struct picotm_error* error)
{
    return 0;
}

static int
update_cc_2pl(struct ofd_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_2PL);

    struct ofd *ofd = ofdtab+self->ofd;

    /* release record locks */

    if (self->type == PICOTM_LIBC_FILE_TYPE_REGULAR) {
        ofd_tx_2pl_release_locks(self);
    }

    /* release ofd lock */

    ofd_rwunlock_state(ofd, &self->modedata.tpl.rwstate);

    return 0;
}

int
ofd_tx_update_cc(struct ofd_tx* self, struct picotm_error* error)
{
    static int (* const update_cc[])(struct ofd_tx*, struct picotm_error*) = {
        update_cc_noundo,
        update_cc_2pl
    };

    assert(ofd_tx_holds_ref(self));

    return update_cc[self->cc_mode](self, error);
}

/*
 * Clear CC
 */

static int
clear_cc_noundo(struct ofd_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO);

    return 0;
}

static int
clear_cc_2pl(struct ofd_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_2PL);

    struct ofd *ofd = ofdtab+self->ofd;

    /* release record locks */

    if (self->type == PICOTM_LIBC_FILE_TYPE_REGULAR) {
        ofd_tx_2pl_release_locks(self);
    }

    /* release ofd lock */

    ofd_rwunlock_state(ofd, &self->modedata.tpl.rwstate);

    return 0;
}

int
ofd_tx_clear_cc(struct ofd_tx* self, struct picotm_error* error)
{
    static int (* const clear_cc[])(struct ofd_tx*, struct picotm_error*) = {
        clear_cc_noundo,
        clear_cc_2pl
    };

    assert(ofd_tx_holds_ref(self));

    return clear_cc[self->cc_mode](self, error);
}

/*
 * Referencing
 */

enum error_code
ofd_tx_ref(struct ofd_tx* self, int ofdindex, int fildes, unsigned long flags)
{
    assert(self);
    assert(ofdindex >= 0);
    assert(ofdindex < (ssize_t)(sizeof(ofdtab)/sizeof(ofdtab[0])));

    if (!ofd_tx_holds_ref(self)) {

        /* get reference and status */

        off_t offset;
        enum picotm_libc_file_type type;
        enum picotm_libc_cc_mode cc_mode;

        struct ofd *ofd = ofdtab+ofdindex;

        int err = ofd_ref_state(ofd, fildes, flags, &type, &cc_mode, &offset);

        if (err) {
            return err;
        }

        /* setup fields */

        self->ofd = ofdindex;
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

        self->modedata.tpl.rwstate = RW_NOLOCK;
        self->modedata.tpl.locktablen = 0;
    }

    return 0;
}

void
ofd_tx_unref(struct ofd_tx* self)
{
    assert(self);

    if (!ofd_tx_holds_ref(self)) {
        return;
    }

    struct ofd *ofd = ofdtab+self->ofd;
    ofd_unref(ofd);

    self->ofd = -1;
}

int
ofd_tx_holds_ref(struct ofd_tx* self)
{
    assert(self);

    return (self->ofd >= 0) &&
           (self->ofd < (ssize_t)(sizeof(ofdtab)/sizeof(ofdtab[0])));
}

static off_t
append_to_iobuffer(struct ofd_tx* self, size_t nbyte, const void* buf)
{
    off_t bufoffset;

    assert(self);

    bufoffset = self->wrbuflen;

    if (nbyte && buf) {

        /* resize */
        void* tmp = picotm_tabresize(self->wrbuf,
                                     self->wrbuflen,
                                     self->wrbuflen+nbyte,
                                     sizeof(self->wrbuf[0]));
        if (!tmp) {
            return (off_t)ERR_SYSTEM;
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
                          const void* buf)
{
    off_t bufoffset;

    assert(self);

    bufoffset = append_to_iobuffer(self, nbyte, buf);

    if (!(bufoffset >= 0)) {
        return (int)bufoffset;
    }

    return iooptab_append(&self->wrtab,
                          &self->wrtablen,
                          &self->wrtabsiz, nbyte, offset, bufoffset);
}

int
ofd_tx_append_to_readset(struct ofd_tx* self, size_t nbyte, off_t offset,
                         const void* buf)
{
    off_t bufoffset;

    assert(self);

    bufoffset = append_to_iobuffer(self, nbyte, buf);

    if (!(bufoffset >= 0)) {
        return (int)bufoffset;
    }

    return iooptab_append(&self->rdtab,
                          &self->rdtablen,
                          &self->rdtabsiz, nbyte, offset, bufoffset);
}

/*
 * prepare commit
 */

int
ofd_tx_pre_commit(struct ofd_tx* self)
{
    assert(self);

    return 0;
}

/*
 * cleanup commit
 */

int
ofd_tx_post_commit(struct ofd_tx* self)
{
    assert(self);

    return 0;
}

/*
 * Pessimistic CC
 */

int
ofd_tx_2pl_lock_region(struct ofd_tx* self, size_t nbyte, off_t offset,
                       int write)
{
    int err;

    assert(self);

    err = ofd_2pl_lock_region(ofdtab+self->ofd,
                              offset,
                              nbyte,
                              write,
                             &self->modedata.tpl.rwstatemap);

    if (!err) {
        err = regiontab_append(&self->modedata.tpl.locktab,
                               &self->modedata.tpl.locktablen,
                               &self->modedata.tpl.locktabsiz,
                                nbyte, offset);
    }

    return err;
}

#include <stdio.h>

void
ofd_tx_dump(const struct ofd_tx* self)
{
    fprintf(stderr, "%p: %d %p %zu %p %zu %p %zu %ld\n", (void*)self,
                                                                self->ofd,
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
                 int* cookie)
{
    return bind(sockfd, addr, addrlen);
}

int
ofd_tx_bind_exec(struct ofd_tx* self, int sockfd, const struct sockaddr* addr,
                 socklen_t addrlen, int* cookie, int noundo)
{
    static int (* const bind_exec[][2])(struct ofd_tx*,
                                        int,
                                  const struct sockaddr*,
                                        socklen_t,
                                        int*) = {
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
            return ERR_NOUNDO;
        }
    }

    return bind_exec[self->type][self->cc_mode](self,
                                                  sockfd,
                                                  addr,
                                                  addrlen, cookie);
}

static int
bind_apply_noundo(struct ofd_tx* self, int sockfd,
                  const struct fd_event* event, size_t n,
                  struct picotm_error* error)
{
    return 0;
}

int
ofd_tx_bind_apply(struct ofd_tx* self, int sockfd,
                  const struct fd_event* event, size_t n,
                  struct picotm_error* error)
{
    static int (* const bind_apply[][2])(struct ofd_tx*,
                                         int,
                                         const struct fd_event*,
                                         size_t,
                                         struct picotm_error*) = {
        {bind_apply_noundo, NULL},
        {bind_apply_noundo, NULL},
        {bind_apply_noundo, NULL},
        {bind_apply_noundo, NULL}
    };

    assert(self->type < sizeof(bind_apply)/sizeof(bind_apply[0]));
    assert(bind_apply[self->type]);

    return bind_apply[self->type][self->cc_mode](self, sockfd, event, n,
                                                 error);
}

int
ofd_tx_bind_undo(struct ofd_tx* self, int sockfd, int cookie,
                 struct picotm_error* error)
{
    static int (* const bind_undo[][2])(int, struct picotm_error*) = {
        {NULL, NULL},
        {NULL, NULL},
        {NULL, NULL},
        {NULL, NULL}
    };

    assert(self->type < sizeof(bind_undo)/sizeof(bind_undo[0]));
    assert(bind_undo[self->type]);

    return bind_undo[self->type][self->cc_mode](cookie, error);
}

/*
 * connect()
 */

static int
connect_exec_noundo(struct ofd_tx* self, int sockfd,
                    const struct sockaddr* serv_addr,
                    socklen_t addrlen, int* cookie)
{
    return TEMP_FAILURE_RETRY(connect(sockfd, serv_addr, addrlen));
}

int
ofd_tx_connect_exec(struct ofd_tx* self, int sockfd,
                    const struct sockaddr* serv_addr, socklen_t addrlen,
                    int* cookie, int noundo)
{
    static int (* const connect_exec[][2])(struct ofd_tx*,
                                           int,
                                           const struct sockaddr*,
                                           socklen_t,
                                           int*) = {
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
            return ERR_NOUNDO;
        }
    }

    return connect_exec[self->type][self->cc_mode](self,
                                                   sockfd,
                                                   serv_addr,
                                                   addrlen, cookie);
}

static int
connect_apply_noundo(struct ofd_tx* self, int sockfd,
                     const struct fd_event* event, size_t n,
                     struct picotm_error* error)
{
    return 0;
}

int
ofd_tx_connect_apply(struct ofd_tx* self, int sockfd,
                     const struct fd_event* event, size_t n,
                     struct picotm_error* error)
{
    static int (* const connect_apply[][2])(struct ofd_tx*,
                                            int,
                                            const struct fd_event*,
                                            size_t,
                                            struct picotm_error*) = {
        {connect_apply_noundo, NULL},
        {connect_apply_noundo, NULL},
        {connect_apply_noundo, NULL},
        {connect_apply_noundo, NULL}
    };

    assert(self->type < sizeof(connect_apply)/sizeof(connect_apply[0]));
    assert(connect_apply[self->type]);

    return connect_apply[self->type][self->cc_mode](self, sockfd, event, n,
                                                    error);
}

int
ofd_tx_connect_undo(struct ofd_tx* self, int sockfd, int cookie,
                    struct picotm_error* error)
{
    static int (* const connect_undo[][2])(struct ofd_tx*,
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

    return connect_undo[self->type][self->cc_mode](self, sockfd, cookie,
                                                   error);
}

/*
 * fcntl()
 */

static int
fcntl_exec_noundo(struct ofd_tx* self, int fildes, int cmd,
                  union fcntl_arg* arg, int* cookie)
{
    int res = 0;

    assert(arg);

    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN:

            arg->arg0 = TEMP_FAILURE_RETRY(fcntl(fildes, cmd));

            if (arg->arg0 < 0) {
                res = -1;
            }
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
            res = ERR_SYSTEM;
            break;
    }

    return res;
}

static int
fcntl_exec_2pl(struct ofd_tx* self, int fildes, int cmd,
               union fcntl_arg* arg, int* cookie)
{
    int res = 0;

    assert(arg);

    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN:

            /* Read-lock open file description */
            if ((res = ofd_rdlock_state(ofdtab+self->ofd, &self->modedata.tpl.rwstate)) < 0) {
                return res;
            }

            arg->arg0 = TEMP_FAILURE_RETRY(fcntl(fildes, cmd));
            if (arg->arg0 < 0) {
                res = -1;
            }
            break;
        case F_GETLK:

            /* Read-lock open file description */
            if ((res = ofd_rdlock_state(ofdtab+self->ofd, &self->modedata.tpl.rwstate)) < 0) {
                return res;
            }

            res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg1));
            break;
        case F_SETFL:
        case F_SETFD:
        case F_SETOWN:
        case F_SETLK:
        case F_SETLKW:
            res = ERR_NOUNDO;
            break;
        default:
            errno = EINVAL;
            res = ERR_SYSTEM;
            break;
    }

    if (res < 0) {
        return res;
    }

    return res;
}

int
ofd_tx_fcntl_exec(struct ofd_tx* self, int fildes, int cmd,
                  union fcntl_arg* arg, int* cookie, int noundo)
{
    static int (* const fcntl_exec[][2])(struct ofd_tx*,
                                         int,
                                         int,
                                         union fcntl_arg*, int*) = {
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
            return ERR_NOUNDO;
        }
    }

    return fcntl_exec[self->type][self->cc_mode](self, fildes, cmd, arg, cookie);
}

int
ofd_tx_fcntl_apply(struct ofd_tx* self, int fildes,
                   const struct fd_event* event, size_t n,
                   struct picotm_error* error)
{
    return 0;
}

int
ofd_tx_fcntl_undo(struct ofd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    return 0;
}

/*
 * fsync()
 */

static int
fsync_exec_noundo(int fildes, int* cookie)
{
    return fsync(fildes);
}

static int
fsync_exec_regular_2pl(int fildes, int* cookie)
{
    /* Signal apply/undo */
    *cookie = 0;

    return 0;
}

int
ofd_tx_fsync_exec(struct ofd_tx* self, int fildes, int noundo, int* cookie)
{
    static int (* const fsync_exec[][2])(int, int*) = {
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
            return ERR_NOUNDO;
        }
    }

    return fsync_exec[self->type][self->cc_mode](fildes, cookie);
}

static int
fsync_apply_noundo(int fildes, struct picotm_error* error)
{
    return 0;
}

static int
fsync_apply_regular_2pl(int fildes, struct picotm_error* error)
{
    int res = fsync(fildes);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }
    return 0;
}

int
ofd_tx_fsync_apply(struct ofd_tx* self, int fildes,
                   const struct fd_event* event, size_t n,
                   struct picotm_error* error)
{
    static int (* const fsync_apply[][2])(int, struct picotm_error*) = {
        {fsync_apply_noundo, NULL},
        {fsync_apply_noundo, fsync_apply_regular_2pl},
        {fsync_apply_noundo, NULL},
        {fsync_apply_noundo, NULL}
    };

    assert(self->type < sizeof(fsync_apply)/sizeof(fsync_apply[0]));
    assert(fsync_apply[self->type][self->cc_mode]);

    return fsync_apply[self->type][self->cc_mode](fildes, error);
}

static int
fsync_undo_regular_2pl(int fildes, int cookie, struct picotm_error* error)
{
    return 0;
}

int
ofd_tx_fsync_undo(struct ofd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static int (* const fsync_undo[][2])(int, int, struct picotm_error*) = {
        {NULL, NULL},
        {NULL, fsync_undo_regular_2pl},
        {NULL, NULL},
        {NULL, NULL}
    };

    assert(self->type < sizeof(fsync_undo)/sizeof(fsync_undo[0]));
    assert(fsync_undo[self->type][self->cc_mode]);

    return fsync_undo[self->type][self->cc_mode](fildes, cookie, error);
}

/*
 * listen()
 */

static int
listen_exec_noundo(struct ofd_tx* self, int sockfd, int backlog, int* cookie)
{
    return listen(sockfd, backlog);
}

int
ofd_tx_listen_exec(struct ofd_tx* self, int sockfd, int backlog, int* cookie,
                   int noundo)
{
    static int (* const listen_exec[][2])(struct ofd_tx*,
                                          int,
                                          int,
                                          int*) = {
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
            return ERR_NOUNDO;
        }
    }

    return listen_exec[self->type][self->cc_mode](self, sockfd, backlog, cookie);
}

static int
listen_apply_noundo(struct picotm_error* error)
{
    return 0;
}

int
ofd_tx_listen_apply(struct ofd_tx* self, int sockfd,
                    const struct fd_event* event, size_t n,
                    struct picotm_error* error)
{
    static int (* const listen_apply[][2])(struct picotm_error*) = {
        {listen_apply_noundo, NULL},
        {listen_apply_noundo, NULL},
        {listen_apply_noundo, NULL},
        {listen_apply_noundo, NULL}
    };

    assert(self->type < sizeof(listen_apply)/sizeof(listen_apply[0]));
    assert(listen_apply[self->type]);

    return listen_apply[self->type][self->cc_mode](error);
}

int
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

    return listen_undo[self->type][self->cc_mode](self, sockfd, cookie, error);
}

/*
 * lseek()
 */

static off_t
filesize(int fildes)
{
    struct stat buf;

    if (fstat(fildes, &buf) < 0) {
        return ERR_SYSTEM;
    }

    return buf.st_size;
}

static off_t
lseek_exec_noundo(struct ofd_tx* self, int fildes, off_t offset,
                  int whence, int* cookie)
{
    return TEMP_FAILURE_RETRY(lseek(fildes, offset, whence));
}

static off_t
lseek_exec_regular_2pl(struct ofd_tx* self, int fildes, off_t offset,
                       int whence, int* cookie)
{
    int err;

    struct ofd *ofd = ofdtab+self->ofd;

    /* Read-lock open file description */

    if ((err = ofd_rdlock_state(ofd, &self->modedata.tpl.rwstate)) < 0) {
        return (off_t)err;
    }

	/* Fastpath: Read current position */
	if (!offset && (whence == SEEK_CUR)) {
		return self->offset;
	}

    /* Write-lock open file description to change position */

    if ((err = ofd_wrlock_state(ofd, &self->modedata.tpl.rwstate)) < 0) {
        return (off_t)err;
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
        case SEEK_END:
            {
                const off_t fs = filesize(fildes);

                if (fs == (off_t)ERR_SYSTEM) {
                    pos = (off_t)ERR_SYSTEM;
                    break;
                }

                pos = llmax(self->size, fs)+offset;
            }
            break;
        default:
            pos = -1;
            break;
    }

    if (pos < 0) {
        errno = EINVAL;
        pos = (off_t)ERR_SYSTEM;
    }

    if ((pos == (off_t)-2) || (pos == (off_t)-1)) {
        return pos;
    }

    if (cookie) {
        *cookie = seekoptab_append(&self->seektab,
                                   &self->seektablen,
                                    self->offset, offset, whence);

        if (*cookie < 0) {
            abort();
        }
    }

    self->offset = pos; /* Update file pointer */

    return pos;
}

off_t
ofd_tx_lseek_exec(struct ofd_tx* self, int fildes,  off_t offset, int whence,
                  int* cookie, int noundo)
{
    static off_t (* const lseek_exec[][2])(struct ofd_tx*,
                                           int,
                                           off_t,
                                           int,
                                           int*) = {
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
            return ERR_NOUNDO;
        }
    }

    return lseek_exec[self->type][self->cc_mode](self, fildes, offset, whence, cookie);
}

static int
lseek_apply_noundo(struct ofd_tx* self, int fildes,
                   const struct fd_event* event, size_t n,
                   struct picotm_error* error)
{
    return 0;
}

static int
lseek_apply_regular(struct ofd_tx* self, int fildes,
                    const struct fd_event* event, size_t n,
                    struct picotm_error* error)
{
    while (n) {
        const off_t pos = lseek(fildes, self->seektab[event->cookie].offset,
                                        self->seektab[event->cookie].whence);

        if (pos == (off_t)-1) {
            picotm_error_set_errno(error, errno);
            return ERR_SYSTEM;
        }

        struct ofd *ofd = ofdtab+self->ofd;

        ofd->data.regular.offset = pos;

        --n;
        ++event;
    }

    return 0;
}

int
ofd_tx_lseek_apply(struct ofd_tx* self, int fildes,
                   const struct fd_event* event, size_t n,
                   struct picotm_error* error)
{
    static int (* const lseek_apply[][2])(struct ofd_tx*,
                                          int,
                                          const struct fd_event*,
                                          size_t,
                                          struct picotm_error*) = {
        {lseek_apply_noundo, NULL},
        {lseek_apply_noundo, lseek_apply_regular},
        {lseek_apply_noundo, NULL},
        {lseek_apply_noundo, NULL}
    };

    assert(self->type < sizeof(lseek_apply)/sizeof(lseek_apply[0]));
    assert(lseek_apply[self->type][self->cc_mode]);

    return lseek_apply[self->type][self->cc_mode](self, fildes, event, n, error);
}

static int
lseek_undo_regular(struct picotm_error* error)
{
    return 0;
}

int
ofd_tx_lseek_undo(struct ofd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static int (* const lseek_undo[][2])(struct picotm_error*) = {
        {NULL, NULL},
        {NULL, lseek_undo_regular},
        {NULL, NULL},
        {NULL, NULL}
    };

    assert(self->type < sizeof(lseek_undo)/sizeof(lseek_undo[0]));
    assert(lseek_undo[self->type][self->cc_mode]);

    return lseek_undo[self->type][self->cc_mode](error);
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
                  enum picotm_libc_validation_mode val_mode)
{
    return TEMP_FAILURE_RETRY(pread(fildes, buf, nbyte, offset));
}

static ssize_t
pread_exec_regular_2pl(struct ofd_tx* self, int fildes, void* buf,
                       size_t nbyte, off_t offset, int* cookie,
                       enum picotm_libc_validation_mode val_mode)
{
    int err;
    ssize_t len, len2;

    /* lock region */
    if ((err = ofd_tx_2pl_lock_region(self, nbyte, offset, 0)) < 0) {
        return err;
    }

    /* read from file */
    if ((len = TEMP_FAILURE_RETRY(pread(fildes, buf, nbyte, offset))) < 0) {
        return len;
    }

    /* read from local write set */
    if ((len2 = iooptab_read(self->wrtab,
                             self->wrtablen,
                             buf, nbyte, offset, self->wrbuf)) < 0) {
        return len2;
    }

    return llmax(len, len2);
}

static ssize_t
pread_exec_fifo(struct ofd_tx* self, int fildes, void *buf, size_t nbyte,
                off_t offset, int* cookie,
                enum picotm_libc_validation_mode val_mode)
{
    errno = ESPIPE;
    return ERR_SYSTEM;
}

ssize_t
ofd_tx_pread_exec(struct ofd_tx* self, int fildes, void* buf, size_t nbyte,
                  off_t offset, int* cookie, int noundo,
                  enum picotm_libc_validation_mode val_mode)
{
    static ssize_t (* const pread_exec[][2])(struct ofd_tx*,
                                             int,
                                             void*,
                                             size_t,
                                             off_t,
                                             int*,
                                             enum picotm_libc_validation_mode) = {
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
            return ERR_NOUNDO;
        }
    }

    return pread_exec[self->type][self->cc_mode](self, fildes, buf, nbyte, offset, cookie, val_mode);
}

static ssize_t
pread_apply_any(struct picotm_error* error)
{
    return 0;
}

ssize_t
ofd_tx_pread_apply(struct ofd_tx* self, int fildes,
                   const struct fd_event* event, size_t n,
                   struct picotm_error* error)
{
    static ssize_t (* const pread_apply[][2])(struct picotm_error*) = {
        {pread_apply_any, NULL},
        {pread_apply_any, pread_apply_any},
        {pread_apply_any, NULL},
        {pread_apply_any, NULL}
    };

    assert(self->type < sizeof(pread_apply)/sizeof(pread_apply[0]));
    assert(pread_apply[self->type]);

    int err = 0;

    while (n && !err) {
        err = pread_apply[self->type][self->cc_mode](error);
        --n;
    }

    return err;
}

static int
pread_undo_any(struct picotm_error* error)
{
    return 0;
}

int
ofd_tx_pread_undo(struct ofd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static int (* const pread_undo[][2])(struct picotm_error*) = {
        {NULL, NULL},
        {NULL, pread_undo_any},
        {NULL, NULL},
        {NULL, NULL}
    };

    assert(self->type < sizeof(pread_undo)/sizeof(pread_undo[0]));
    assert(pread_undo[self->type]);

    return pread_undo[self->type][self->cc_mode](error);
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
                   size_t nbyte, off_t offset, int* cookie)
{
    return TEMP_FAILURE_RETRY(pwrite(fildes, buf, nbyte, offset));
}

static ssize_t
pwrite_exec_regular_2pl(struct ofd_tx* self, int fildes,
                        const void* buf, size_t nbyte, off_t offset,
                        int* cookie)
{
    /* register written data */

    if (__builtin_expect(!!cookie, 1)) {

        int err;

        /* lock region */

        if ((err = ofd_tx_2pl_lock_region(self, nbyte, offset, 1)) < 0) {
            return err;
        }

        /* add data to write set */
        if ((*cookie = ofd_tx_append_to_writeset(self,
                                                nbyte,
                                                offset, buf)) < 0) {
            return *cookie;
        }
    }

    return nbyte;
}

ssize_t
ofd_tx_pwrite_exec(struct ofd_tx* self, int fildes, const void* buf,
                   size_t nbyte, off_t offset, int* cookie, int noundo)
{
    static ssize_t (* const pwrite_exec[][2])(struct ofd_tx*,
                                              int,
                                              const void*,
                                              size_t,
                                              off_t,
                                              int*) = {
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
            return ERR_NOUNDO;
        }
    }

    return pwrite_exec[self->type][self->cc_mode](self, fildes, buf, nbyte, offset, cookie);
}

static ssize_t
pwrite_apply_noundo(struct ofd_tx* self, int fildes,
                    const struct fd_event* event, size_t n,
                    struct picotm_error* error)
{
    return 0;
}

static ssize_t
pwrite_apply_regular(struct ofd_tx* self, int fildes,
                     const struct fd_event* event, size_t n,
                     struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(event || !n);

    while (n) {

        /* Combine writes to adjacent locations */

        off_t  off = self->wrtab[event->cookie].off;
        size_t nbyte = self->wrtab[event->cookie].nbyte;
        off_t  bufoff = self->wrtab[event->cookie].bufoff;

        size_t m = 1;

        while ((m < n)
                && (self->wrtab[event[m].cookie].off == (off_t)(off+nbyte))
                && (self->wrtab[event[m].cookie].bufoff == bufoff+nbyte)) {

            nbyte += self->wrtab[event[m].cookie].nbyte;

            ++m;
        }

        ssize_t res = TEMP_FAILURE_RETRY(pwrite(fildes,
                                                self->wrbuf + bufoff,
                                                nbyte, off));
        if (res < 0) {
            picotm_error_set_errno(error, errno);
            return ERR_SYSTEM;
        }

        n -= m;
        event += m;
    }

    return 0;
}

ssize_t
ofd_tx_pwrite_apply(struct ofd_tx* self, int fildes,
                    const struct fd_event* event, size_t n,
                    struct picotm_error* error)
{
    static ssize_t (* const pwrite_apply[][2])(struct ofd_tx*,
                                               int,
                                               const struct fd_event*,
                                               size_t,
                                               struct picotm_error*) = {
        {pwrite_apply_noundo, NULL},
        {pwrite_apply_noundo, pwrite_apply_regular},
        {pwrite_apply_noundo, NULL},
        {pwrite_apply_noundo, NULL}
    };

    assert(self->type < sizeof(pwrite_apply)/sizeof(pwrite_apply[0]));
    assert(pwrite_apply[self->type]);

    return pwrite_apply[self->type][self->cc_mode](self, fildes, event, n,
                                                   error);
}

static int
pwrite_any_undo(struct picotm_error* error)
{
    return 0;
}

int
ofd_tx_pwrite_undo(struct ofd_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{
    static int (* const pwrite_undo[][2])(struct picotm_error*) = {
        {NULL, NULL},
        {NULL, pwrite_any_undo},
        {NULL, NULL},
        {NULL, NULL}
    };

    assert(self->type < sizeof(pwrite_undo)/sizeof(pwrite_undo[0]));
    assert(pwrite_undo[self->type]);

    return pwrite_undo[self->type][self->cc_mode](error);
}

/*
 * read()
 */

static ssize_t
read_exec_noundo(struct ofd_tx* self, int fildes, void* buf,
                 size_t nbyte, int* cookie,
                 enum picotm_libc_validation_mode val_mode)
{
    return TEMP_FAILURE_RETRY(read(fildes, buf, nbyte));
}

static ssize_t
read_exec_regular_2pl(struct ofd_tx* self, int fildes, void* buf,
                      size_t nbyte, int* cookie,
                      enum picotm_libc_validation_mode val_mode)
{
    int err;

    assert(self);

    struct ofd *ofd = ofdtab+self->ofd;

    /* write-lock open file description, because we change the file position */

    if ((err = ofd_wrlock_state(ofd, &self->modedata.tpl.rwstate)) < 0) {
        return err;
    }

    /* read-lock region */

    if ((err = ofd_tx_2pl_lock_region(self, nbyte, self->offset, 0)) < 0) {
        return err;
    }

    /* read from file */

    ssize_t len = TEMP_FAILURE_RETRY(pread(fildes, buf, nbyte, self->offset));

    if (len < 0) {
        return len;
    }

    /* read from local write set */

    ssize_t len2 = iooptab_read(self->wrtab,
                                self->wrtablen, buf, nbyte,
                                self->offset,
                                self->wrbuf);

    if (len2 < 0) {
        return len2;
    }

    ssize_t res = llmax(len, len2);

    /* update file pointer */
    self->offset += res;

    return res;
}

ssize_t
ofd_tx_read_exec(struct ofd_tx* self, int fildes, void* buf, size_t nbyte,
                 int* cookie, int noundo,
                 enum picotm_libc_validation_mode val_mode)
{
    static ssize_t (* const read_exec[][2])(struct ofd_tx*,
                                            int,
                                            void*,
                                            size_t,
                                            int*,
                                            enum picotm_libc_validation_mode) = {
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
            return ERR_NOUNDO;
        }
    }

    return read_exec[self->type][self->cc_mode](self, fildes, buf, nbyte, cookie, val_mode);
}

static ssize_t
read_apply_noundo(struct ofd_tx* self, int fildes,
                  const struct fd_event* event, size_t n,
                  struct picotm_error* error)
{
    return 0;
}

static ssize_t
read_apply_regular(struct ofd_tx* self, int fildes,
                   const struct fd_event* event, size_t n,
                   struct picotm_error* error)
{
    while (n) {
        ofdtab[self->ofd].data.regular.offset += self->rdtab[event->cookie].nbyte;

        off_t res = lseek(fildes, ofdtab[self->ofd].data.regular.offset,
                          SEEK_SET);
        if (res == (off_t)-1) {
            picotm_error_set_errno(error, errno);
            return ERR_SYSTEM;
        }

        --n;
        ++event;
    }

    return 0;
}

ssize_t
ofd_tx_read_apply(struct ofd_tx* self, int fildes,
                  const struct fd_event* event, size_t n,
                  struct picotm_error* error)
{
    static ssize_t (* const read_apply[][2])(struct ofd_tx*,
                                             int,
                                             const struct fd_event*,
                                             size_t,
                                             struct picotm_error*) = {
        {read_apply_noundo, NULL},
        {read_apply_noundo, read_apply_regular},
        {read_apply_noundo, NULL},
        {read_apply_noundo, NULL}
    };

    assert(self->type < sizeof(read_apply)/sizeof(read_apply[0]));
    assert(read_apply[self->type]);

    return read_apply[self->type][self->cc_mode](self, fildes, event, n, error);
}

static off_t
read_any_undo(struct picotm_error* error)
{
    return 0;
}

off_t
ofd_tx_read_undo(struct ofd_tx* self, int fildes, int cookie,
                 struct picotm_error* error)
{
    static off_t (* const read_undo[][2])(struct picotm_error*) = {
        {NULL, NULL},
        {NULL, read_any_undo},
        {NULL, NULL},
        {NULL, NULL}
    };

    assert(self->type < sizeof(read_undo)/sizeof(read_undo[0]));
    assert(read_undo[self->type]);

    return read_undo[self->type][self->cc_mode](error);
}

/*
 * recv()
 */

static ssize_t
recv_exec_noundo(struct ofd_tx* self, int sockfd, void* buffer,
                        size_t length, int flags, int* cookie)
{
    return TEMP_FAILURE_RETRY(recv(sockfd, buffer, length, flags));
}

ssize_t
ofd_tx_recv_exec(struct ofd_tx* self, int sockfd, void* buffer,
                 size_t length, int flags, int* cookie, int noundo)
{
    static ssize_t (* const recv_exec[][2])(struct ofd_tx*,
                                            int,
                                            void*,
                                            size_t,
                                            int,
                                            int*) = {
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
            return ERR_NOUNDO;
        }
    }

    return recv_exec[self->type][self->cc_mode](self, sockfd, buffer, length, flags, cookie);
}

static ssize_t
recv_apply_noundo(struct picotm_error* error)
{
    return 0;
}

int
ofd_tx_recv_apply(struct ofd_tx* self, int sockfd,
                  const struct fd_event* event, size_t n,
                  struct picotm_error* error)
{
    static ssize_t (* const recv_apply[][2])(struct picotm_error*) = {
        {recv_apply_noundo, NULL},
        {recv_apply_noundo, NULL},
        {recv_apply_noundo, NULL},
        {recv_apply_noundo, NULL}
    };

    assert(self->type < sizeof(recv_apply)/sizeof(recv_apply[0]));
    assert(recv_apply[self->type]);

    return recv_apply[self->type][self->cc_mode](error);
}

int
ofd_tx_recv_undo(struct ofd_tx* self, int sockfd, int cookie,
                 struct picotm_error* error)
{
    static int (* const recv_undo[][2])(struct ofd_tx*,
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

    return recv_undo[self->type][self->cc_mode](self, sockfd, cookie, error);
}

/*
 * send()
 */

static ssize_t
send_exec_noundo(struct ofd_tx* self, int sockfd, const void* buffer,
                 size_t length, int flags, int* cookie)
{
    return TEMP_FAILURE_RETRY(send(sockfd, buffer, length, flags));
}

static ssize_t
send_exec_socket_2pl(struct ofd_tx* self, int sockfd, const void* buf,
                     size_t nbyte, int flags, int* cookie)
{
    int err;

    /* Become irrevocable if any flags are selected */
    if (flags) {
        return ERR_NOUNDO;
    }

    struct ofd *ofd = ofdtab+self->ofd;

    /* Write-lock open file description, because we change the file position */

    if ((err = ofd_wrlock_state(ofd, &self->modedata.tpl.rwstate)) < 0) {
        return err;
    }

    /* Register write data */

    if (cookie) {

        if ((*cookie = ofd_tx_append_to_writeset(self, nbyte, 0, buf)) < 0) {
            return *cookie;
        }
    }

    return nbyte;
}

ssize_t
ofd_tx_send_exec(struct ofd_tx* self, int sockfd, const void* buffer,
                 size_t length, int flags, int* cookie, int noundo)
{
    static ssize_t (* const send_exec[][2])(struct ofd_tx*,
                                            int,
                                      const void*,
                                            size_t,
                                            int,
                                            int*) = {
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
            return ERR_NOUNDO;
        }
    }

    return send_exec[self->type][self->cc_mode](self, sockfd, buffer, length, flags, cookie);
}

static int
send_apply_noundo(struct ofd_tx* self, int sockfd, int cookie,
                  struct picotm_error* error)
{
    return 0;
}

static int
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
        return ERR_SYSTEM;
    }

    return 0;
}

int
ofd_tx_send_apply(struct ofd_tx* self, int sockfd,
                  const struct fd_event* event, size_t n,
                  struct picotm_error* error)
{
    static int (* const send_apply[][2])(struct ofd_tx*,
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

    while (n) {
        int res = send_apply[self->type][self->cc_mode](self, sockfd,
                                                        event->cookie,
                                                        error);
        if (res < 0) {
            return res;
        }
        --n;
        ++event;
    }

    return 0;
}

static int
send_undo_socket_2pl(struct picotm_error* error)
{
    return 0;
}

int
ofd_tx_send_undo(struct ofd_tx* self, int sockfd, int cookie,
                 struct picotm_error* error)
{
    static int (* const send_undo[][2])(struct picotm_error*) = {
        {NULL, NULL},
        {NULL, NULL},
        {NULL, NULL},
        {NULL, send_undo_socket_2pl}
    };

    assert(self->type < sizeof(send_undo)/sizeof(send_undo[0]));
    assert(send_undo[self->type][self->cc_mode]);

    return send_undo[self->type][self->cc_mode](error);
}

/*
 * shutdown()
 */

static int
shutdown_exec_noundo(struct ofd_tx* self, int sockfd, int how,
                            int* cookie)
{
    return shutdown(sockfd, how);
}

int
ofd_tx_shutdown_exec(struct ofd_tx* self, int sockfd, int how, int* cookie,
                     int noundo)
{
    static int (* const shutdown_exec[][2])(struct ofd_tx*,
                                            int,
                                            int,
                                            int*) = {
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
            return ERR_NOUNDO;
        }
    }

    return shutdown_exec[self->type][self->cc_mode](self, sockfd, how, cookie);
}

static int
shutdown_apply_noundo(struct ofd_tx* self, int sockfd,
                      const struct fd_event* event, size_t n,
                      struct picotm_error* error)
{
    return 0;
}

int
ofd_tx_shutdown_apply(struct ofd_tx* self, int sockfd,
                      const struct fd_event* event, size_t n,
                      struct picotm_error* error)
{
    static int (* const shutdown_apply[][2])(struct ofd_tx*,
                                             int,
                                             const struct fd_event*,
                                             size_t,
                                             struct picotm_error*) = {
        {shutdown_apply_noundo, NULL},
        {shutdown_apply_noundo, NULL},
        {shutdown_apply_noundo, NULL},
        {shutdown_apply_noundo, NULL}
    };

    assert(self->type < sizeof(shutdown_apply)/sizeof(shutdown_apply[0]));
    assert(shutdown_apply[self->type]);

    return shutdown_apply[self->type][self->cc_mode](self, sockfd, event, n,
                                                     error);
}

int
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

    return shutdown_undo[self->type][self->cc_mode](self, sockfd, cookie,
                                                    error);
}

/*
 * write()
 */

static ssize_t
write_exec_noundo(struct ofd_tx* self, int fildes, const void* buf,
                  size_t nbyte, int* cookie)
{
    return TEMP_FAILURE_RETRY(write(fildes, buf, nbyte));
}

static ssize_t
write_exec_regular_2pl(struct ofd_tx* self, int fildes, const void* buf,
                       size_t nbyte, int* cookie)
{
    /* register written data */

    if (__builtin_expect(!!cookie, 1)) {

        int err;

        struct ofd *ofd = ofdtab+self->ofd;

        /* write-lock open file description, because we change the file position */

        if ((err = ofd_wrlock_state(ofd, &self->modedata.tpl.rwstate)) < 0) {
            return err;
        }

        /* write-lock region */

        if ((err = ofd_tx_2pl_lock_region(self, nbyte, self->offset, 1)) < 0) {
            return err;
        }

        /* add buf to write set */
        if ((*cookie = ofd_tx_append_to_writeset(self,
                                                nbyte,
                                                self->offset, buf)) < 0) {
            return *cookie;
        }
    }

    /* update file pointer */
    self->offset += nbyte;

    return nbyte;
}

static ssize_t
write_exec_fifo_2pl(struct ofd_tx* self, int fildes, const void* buf,
                    size_t nbyte, int* cookie)
{
    int err;

    struct ofd *ofd = ofdtab+self->ofd;

    /* Write-lock open file description, because we change the file position */

    if ((err = ofd_wrlock_state(ofd, &self->modedata.tpl.rwstate)) < 0) {
        return err;
    }

    /* Register write data */

    if (cookie) {
        if ((*cookie = ofd_tx_append_to_writeset(self, nbyte, 0, buf)) < 0) {
            return *cookie;
        }
    }

    return nbyte;
}

static ssize_t
write_exec_socket_2pl(struct ofd_tx* self, int fildes, const void* buf,
                      size_t nbyte, int* cookie)
{
    int err;

    struct ofd *ofd = ofdtab+self->ofd;

    /* Write-lock open file description, because we change the file position */

    if ((err = ofd_wrlock_state(ofd, &self->modedata.tpl.rwstate)) < 0) {
        return err;
    }

    /* Register write data */

    if (cookie) {

        if ((*cookie = ofd_tx_append_to_writeset(self, nbyte, 0, buf)) < 0) {
            return *cookie;
        }
    }

    return nbyte;
}

ssize_t
ofd_tx_write_exec(struct ofd_tx* self, int fildes, const void* buf,
                  size_t nbyte, int* cookie, int noundo)
{
    static ssize_t (* const write_exec[][2])(struct ofd_tx*,
                                             int,
                                             const void*,
                                             size_t,
                                             int*) = {
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
            return ERR_NOUNDO;
        }
    }

    return write_exec[self->type][self->cc_mode](self, fildes, buf, nbyte,
                                                 cookie);
}

static int
write_apply_noundo(struct ofd_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{
    return 0;
}

static int
write_apply_regular_2pl(struct ofd_tx* self, int fildes, int cookie,
                        struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);

    /* FIXME: Use select() to prevent blocking? */

    const ssize_t len = TEMP_FAILURE_RETRY(pwrite(fildes, self->wrbuf+self->wrtab[cookie].bufoff,
                                                          self->wrtab[cookie].nbyte,
                                                          self->wrtab[cookie].off));

    if (len < 0) {
        picotm_error_set_errno(error, errno);
        return ERR_SYSTEM;
    }

    /* Update file position */
    ofdtab[self->ofd].data.regular.offset = self->wrtab[cookie].off+len;

    off_t res = lseek(fildes, ofdtab[self->ofd].data.regular.offset, SEEK_SET);

    if (res == (off_t)-1) {
        picotm_error_set_errno(error, errno);
        return ERR_SYSTEM;
    }

    return 0;
}

static int
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
        return ERR_SYSTEM;
    }

    return 0;
}

static int
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
        return ERR_SYSTEM;
    }

    return 0;
}

ssize_t
ofd_tx_write_apply(struct ofd_tx* self, int fildes,
                   const struct fd_event* event, size_t n,
                   struct picotm_error* error)
{
    static int (* const write_apply[][2])(struct ofd_tx*,
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

    while (n) {
        int res = write_apply[self->type][self->cc_mode](self, fildes,
                                                         event->cookie,
                                                         error);
        if (res < 0) {
            return res;
        }
        --n;
        ++event;
    }

    return 0;
}

static int
write_any_undo(struct picotm_error* error)
{
    return 0;
}

int
ofd_tx_write_undo(struct ofd_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static int (* const write_undo[][2])(struct picotm_error*) = {
        {NULL, NULL},
        {NULL, write_any_undo},
        {NULL, write_any_undo},
        {NULL, write_any_undo}
    };

    assert(self->type < sizeof(write_undo)/sizeof(write_undo[0]));
    assert(write_undo[self->type][self->cc_mode]);

    return write_undo[self->type][self->cc_mode](error);
}
