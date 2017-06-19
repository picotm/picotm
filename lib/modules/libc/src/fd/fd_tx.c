/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "fd_tx.h"
#include <assert.h>
#include <errno.h>
#include <picotm/picotm-error.h>
#include <stdio.h>
#include <stdlib.h>
#include "fcntlop.h"
#include "fcntloptab.h"
#include "fdtab.h"
#include "ofd.h"

void
fd_tx_init(struct fd_tx* self)
{
    assert(self);

    self->fildes = -1;
    self->ofd = -1;
	self->flags = 0;
	self->cc_mode = PICOTM_LIBC_CC_MODE_2PL;

    self->fcntltab = NULL;
    self->fcntltablen = 0;

    self->fdver = 0;
}

void
fd_tx_uninit(struct fd_tx* self)
{
    assert(self);
}

void
fd_tx_ref_or_validate(struct fd_tx* self, int fildes, unsigned long flags,
                      struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(fdtab)/sizeof(fdtab[0])));

    unsigned long fdver;

    struct fd* fd = fdtab + fildes;

    if (fd_tx_holds_ref(self)) {

        /* Validate reference or return error if fd has been closed */

        fd_lock(fd);

        fd_validate(fd, self->fdver, error);
        if (picotm_error_is_set(error)) {
            fd_unlock(fd);
            return;
        }

        fd_unlock(fd);

    } else {

        /* Aquire reference if possible */

        int ofd;
        fd_ref_state(fd, fildes, flags, &ofd, &fdver, error);
        if (picotm_error_is_set(error)) {
            return;
        }

        self->fildes = fildes;
        self->ofd = ofd;
        self->fdver = fdver;
    	self->flags = flags&OFD_FL_WANTNEW ? FDTX_FL_LOCALSTATE : 0;
    }
}

void
fd_tx_ref(struct fd_tx* self, int fildes, unsigned long flags,
          struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(fdtab)/sizeof(fdtab[0])));

    if (fd_tx_holds_ref(self)) {
        return;
    }

    struct fd* fd = fdtab + fildes;

    /* aquire reference if possible */

    unsigned long fdver;
    int ofd;

    fd_ref_state(fd, fildes, flags, &ofd, &fdver, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    self->fildes = fildes;
    self->ofd = ofd;
    self->fdver = fdver;
    self->flags = flags&OFD_FL_WANTNEW ? FDTX_FL_LOCALSTATE : 0;
}

void
fd_tx_unref(struct fd_tx* self)
{
    assert(self);

    if (!fd_tx_holds_ref(self)) {
        return;
    }

    fd_unref(fdtab+self->fildes);

    self->flags = 0;
    self->fildes = -1;
}

int
fd_tx_holds_ref(const struct fd_tx* self)
{
    assert(self);

    return (self->fildes >= 0) &&
           (self->fildes < (ssize_t)(sizeof(fdtab)/sizeof(fdtab[0])));
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
close_exec_noundo(struct fd_tx* self, int fildes, int* cookie,
                  struct picotm_error* error)
{
    *cookie = 0; /* injects event */
    return 0;
}

static int
close_exec_ts(struct fd_tx* self, int fildes, int* cookie,
              struct picotm_error* error)
{
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
    fd_close(fdtab + fildes);
}

static void
close_apply_ts(struct fd_tx* self, int fildes, int cookie,
               struct picotm_error* error)
{
    /* Global data structure 'fdtab' is locked during apply */
    fd_close(fdtab + fildes);
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
 * fcntl()
 */

int
fd_tx_fcntl_exec(struct fd_tx* self, int cmd, union fcntl_arg *arg,
                 int* cookie, int noundo, struct picotm_error* error)
{
    assert(self);
    assert(self->fildes >= 0);
    assert(self->fildes < (ssize_t)(sizeof(fdtab)/sizeof(fdtab[0])));

    union fcntl_arg oldvalue;

    struct fd* fd = fdtab + self->fildes;

    fd_lock(fd);

    fd_validate(fd, self->fdver, error);
    if (picotm_error_is_set(error)) {
        fd_unlock(fd);
        return -1;
    }

    int res = -1;

    switch (cmd) {
        case F_SETFD:
            if ( !noundo ) {
                picotm_error_set_revocable(error);
                return -1;
            }
            res = fd_setfd(fd, arg->arg0, error);
            if (picotm_error_is_set(error)) {
                return -1;
            }
            break;
        case F_GETFD:
            res = fd_getfd(fd, error);
            if (picotm_error_is_set(error)) {
                return -1;
            }
            arg->arg0 = res;
            break;
        default:
            picotm_error_set_errno(error, EINVAL);
            break;
    }

    fd_unlock(fd);

    if (picotm_error_is_set(error)) {
        return -1;
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
}

void
fd_tx_fcntl_apply(struct fd_tx* self, int cookie, bool* next_domain,
                  struct picotm_error* error)
{
    assert(self);
    assert(self->fildes >= 0);
    assert(self->fildes < (ssize_t)(sizeof(fdtab)/sizeof(fdtab[0])));
    assert(cookie < (ssize_t)self->fcntltablen);

    struct fd* fd = fdtab + self->fildes;

    switch (self->fcntltab[cookie].command) {
        case F_SETFD: {
            fd_setfd(fd, self->fcntltab[cookie].value.arg0, error);
            if (picotm_error_is_set(error)) {
                return;
            }
            break;
        }
        case F_GETFD:
            break;
        default:
            /* Caller should try other domain, e.g OFD. */
            *next_domain = true;
            break;
    }
}

void
fd_tx_fcntl_undo(struct fd_tx* self, int cookie, bool* next_domain,
                 struct picotm_error* error)
{
    assert(self);
    assert(self->fildes >= 0);
    assert(self->fildes < (ssize_t)(sizeof(fdtab)/sizeof(fdtab[0])));
    assert(cookie < (ssize_t)self->fcntltablen);

    struct fd* fd = fdtab + self->fildes;

    fd_lock(fd);

    switch (self->fcntltab[cookie].command) {
        case F_SETFD: {
            fd_setfd(fd,
                     self->fcntltab[cookie].oldvalue.arg0,
                     error);
            if (picotm_error_is_set(error)) {
                return;
            }
            break;
        }
        case F_GETFD:
            break;
        default:
            /* Caller should try other domain, e.g OFD. */
            *next_domain = true;
            break;
    }

    fd_unlock(fd);
}

/*
 * Module interface
 */

void
fd_tx_lock(struct fd_tx* self)
{
    assert(self);

    /* unlock file descriptor at the end of commit */

    if (self->flags&FDTX_FL_LOCALSTATE) {
        fd_lock(fdtab+self->fildes);
    }
}

void
fd_tx_unlock(struct fd_tx* self)
{
    assert(self);

    /* file descriptor has local changes */

    if (self->flags&FDTX_FL_LOCALSTATE) {
        fd_unlock(fdtab+self->fildes);
    }
}

void
fd_tx_validate(struct fd_tx* self, struct picotm_error* error)
{
    assert(self);

    if (!fd_tx_holds_ref(self)) {
        return;
    }

	/* file descriptor is still open; previously locked */
	if (!fd_is_open_nl(fdtab+self->fildes)) {
        picotm_error_set_conflicting(error, NULL);
		return;
	}

	/* fastpath: no dependencies to other domains */
	if (!(self->flags&FDTX_FL_LOCALSTATE)) {
		return;
	}

	/* validate version of file descriptor */
    fd_validate(fdtab+self->fildes, self->fdver, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fd_tx_update_cc(struct fd_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(fd_tx_holds_ref(self));
}

void
fd_tx_clear_cc(struct fd_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(fd_tx_holds_ref(self));
}
