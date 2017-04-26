/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "fd_tx.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "errcode.h"
#include "fcntlop.h"
#include "fcntloptab.h"
#include "fdtab.h"
#include "ofd.h"

int
fd_tx_init(struct fd_tx* self)
{
    assert(self);

    self->fildes = -1;
    self->ofd = -1;
	self->flags = 0;
	self->cc_mode = PICOTM_LIBC_CC_MODE_TS;

    self->fcntltab = NULL;
    self->fcntltablen = 0;

    self->fdver = 0;

    return 0;
}

void
fd_tx_uninit(struct fd_tx* self)
{
    assert(self);
}

enum error_code
fd_tx_ref_or_validate(struct fd_tx* self, int fildes, unsigned long flags)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < sizeof(fdtab)/sizeof(fdtab[0]));

    count_type fdver;

    struct fd* fd = fdtab + fildes;

    if (fd_tx_holds_ref(self)) {

        /* Validate reference or return error if fd has been closed */

        fd_lock(fd);

        if (fd_is_open_nl(fd)) {
            fdver = fd_get_version_nl(fd);
        } else {
            fd_unlock(fd);
            return ERR_CONFLICT;
        }

        fd_unlock(fd);

        if (fdver > self->fdver) {
            return ERR_CONFLICT;
        }

    } else {

        /* Aquire reference if possible */

        int ofd;

        int err = fd_ref_state(fd, fildes, flags, &ofd, &fdver);

        if (err) {
            return err;
        }

        self->fildes = fildes;
        self->ofd = ofd;
        self->fdver = fdver;
    	self->flags = flags&OFD_FL_WANTNEW ? FDTX_FL_LOCALSTATE : 0;
    }

    return 0;
}


enum error_code
fd_tx_ref(struct fd_tx* self, int fildes, unsigned long flags)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < sizeof(fdtab)/sizeof(fdtab[0]));

    if (fd_tx_holds_ref(self)) {
        return 0;
    }

    struct fd* fd = fdtab + fildes;

    /* aquire reference if possible */

    count_type fdver;
    int ofd;

    int err = fd_ref_state(fd, fildes, flags, &ofd, &fdver);

    if (err) {
        return err;
    }

    self->fildes = fildes;
    self->ofd = ofd;
    self->fdver = fdver;
    self->flags = flags&OFD_FL_WANTNEW ? FDTX_FL_LOCALSTATE : 0;

    return 0;
}

void
fd_tx_unref(struct fd_tx* self)
{
    assert(self);

    if (!fd_tx_holds_ref(self)) {
        return;
    }

    fd_unref(fdtab+self->fildes, self->fildes);

    self->flags = 0;
    self->fildes = -1;
}

int
fd_tx_holds_ref(const struct fd_tx* self)
{
    assert(self);

    return (self->fildes >= 0) &&
           (self->fildes < sizeof(fdtab)/sizeof(fdtab[0]));
}

int
fd_tx_pre_commit(struct fd_tx* self)
{
    assert(self);

    /* file descriptor has local changes */

    if (self->flags&FDTX_FL_LOCALSTATE) {
        fd_lock(fdtab+self->fildes);
    }

    return 0;
}

int
fd_tx_post_commit(struct fd_tx* self)
{
    assert(self);

    /* unlock file descriptor at the end of commit */

    if (self->flags&FDTX_FL_LOCALSTATE) {
        fd_unlock(fdtab+self->fildes);
    }

    return 0;
}

void
fd_tx_signal_close(struct fd_tx* self)
{
    assert(self);

    fd_close(fdtab+self->fildes);
}

void
fd_tx_dump(const struct fd_tx* self)
{
    fprintf(stderr, "%p: %d %p %zu\n", (void*)self,
                                              self->fildes,
                                       (void*)self->fcntltab,
                                              self->fcntltablen);
}

/*
 * close()
 */

static int
close_exec_noundo(struct fd_tx* self, int fildes, int* cookie)
{
    return 0;
}

static int
close_exec_ts(struct fd_tx* self, int fildes, int* cookie)
{
    return 0;
}

int
fd_tx_close_exec(struct fd_tx* self, int fildes, int* cookie, int noundo)
{
    static int (* const close_exec[2])(struct fd_tx*, int, int*) = {
        close_exec_noundo,
        close_exec_ts
    };

    assert(self->cc_mode < sizeof(close_exec)/sizeof(close_exec[0]));

    if (noundo) {
        /* TX irrevokable */
        self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !close_exec[self->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return close_exec[self->cc_mode](self, fildes, cookie);
}

static int
close_apply_noundo(struct fd_tx* self, int fildes, int cookie)
{
    fd_close(fdtab+fildes);
    return 0;
}

static int
close_apply_ts(struct fd_tx* self, int fildes, int cookie)
{
    /* Global data structure 'fdtab' is locked during apply */
    fd_close(fdtab+fildes);
    return 0;
}

int
fd_tx_close_apply(struct fd_tx* self, int fildes, int cookie)
{
    static int (* const close_apply[2])(struct fd_tx*, int, int) = {
        close_apply_noundo,
        close_apply_ts
    };

    assert(self->cc_mode < sizeof(close_apply)/sizeof(close_apply[0]));

    return close_apply[self->cc_mode](self, fildes, cookie);
}

static int
close_undo_ts(struct fd_tx* self, int fildes, int cookie)
{
    return 0;
}

int
fd_tx_close_undo(struct fd_tx* self, int fildes, int cookie)
{
    static int (* const close_undo[2])(struct fd_tx*, int, int) = {
        NULL,
        close_undo_ts
    };

    assert(self->cc_mode < sizeof(close_undo)/sizeof(close_undo[0]));
    assert(close_undo[self->cc_mode]);

    return close_undo[self->cc_mode](self, fildes, cookie);
}

/*
 * fcntl()
 */

int
fd_tx_fcntl_exec(struct fd_tx* self, int cmd, union fcntl_arg *arg,
                 int* cookie, int noundo)
{
    assert(self);
    assert(self->fildes >= 0);
    assert(self->fildes < sizeof(fdtab)/sizeof(fdtab[0]));

    union fcntl_arg oldvalue;

    fd_lock(fdtab+self->fildes);
    int res = fd_fcntl_exec(fdtab+self->fildes,
                                  self->fildes, cmd, arg, &oldvalue,
                                  self->fdver, noundo);
    fd_unlock(fdtab+self->fildes);

    if (res < 0) {
        return res;
    }

    /* register fcntl */

    if (cookie) {
        *cookie = fcntloptab_append(&self->fcntltab,
                                    &self->fcntltablen, cmd, arg, &oldvalue);

        if (*cookie < 0) {
            abort();
        }
    }

	self->flags |= FDTX_FL_LOCALSTATE;

    return res;
}

int
fd_tx_fcntl_apply(struct fd_tx* self, int cookie)
{
    assert(self);
    assert(self->fildes >= 0);
    assert(self->fildes < sizeof(fdtab)/sizeof(fdtab[0]));
    assert(cookie < self->fcntltablen);

    return fd_fcntl_apply(fdtab+self->fildes,
                                self->fildes, self->fcntltab[cookie].command,
                                             &self->fcntltab[cookie].value,
                                              self->cc_mode);
}

int
fd_tx_fcntl_undo(struct fd_tx* self, int cookie)
{
    assert(self);
    assert(self->fildes >= 0);
    assert(self->fildes < sizeof(fdtab)/sizeof(fdtab[0]));
    assert(cookie < self->fcntltablen);

    fd_lock(fdtab+self->fildes);
    int res = fd_fcntl_undo(fdtab+self->fildes,
                                  self->fildes, self->fcntltab[cookie].command,
                                               &self->fcntltab[cookie].oldvalue,
                                                self->cc_mode);
    fd_unlock(fdtab+self->fildes);

    return res;
}

/*
 * Module interface
 */

int
fd_tx_validate(struct fd_tx* self)
{
	int res;

    assert(self);

    if (!fd_tx_holds_ref(self)) {
        return 0;
    }

	/* file descriptor is still open; previously locked */
	if (!fd_is_open_nl(fdtab+self->fildes)) {
		return ERR_CONFLICT;
	}

	/* fastpath: no dependencies to other domains */
	if (!(self->flags&FDTX_FL_LOCALSTATE)) {
		return 0;
	}

	/* validate version of file descriptor */
    if ((res = fd_validate(fdtab+self->fildes, self->fdver)) < 0) {
        return res;
    }

    return 0;
}

int
fd_tx_update_cc(struct fd_tx* self)
{
    assert(self);
    assert(fd_tx_holds_ref(self));

    return 0;
}

int
fd_tx_clear_cc(struct fd_tx* self)
{
    assert(self);
    assert(fd_tx_holds_ref(self));

    return 0;
}
