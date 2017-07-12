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

#include "dir_tx.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-ptr.h>
#include <picotm/picotm-lib-tab.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "dir.h"
#include "fcntlop.h"
#include "fcntloptab.h"

struct dir_tx*
dir_tx_of_ofd_tx(struct ofd_tx* ofd_tx)
{
    assert(ofd_tx);
    assert(ofd_tx_file_type(ofd_tx) == PICOTM_LIBC_FILE_TYPE_DIR);

    return picotm_containerof(ofd_tx, struct dir_tx, base);
}

static void
ref_ofd_tx(struct ofd_tx* ofd_tx)
{
    dir_tx_ref(dir_tx_of_ofd_tx(ofd_tx));
}

static void
unref_ofd_tx(struct ofd_tx* ofd_tx)
{
    dir_tx_unref(dir_tx_of_ofd_tx(ofd_tx));
}

void
dir_tx_init(struct dir_tx* self)
{
    assert(self);

    picotm_ref_init(&self->ref, 0);

    memset(&self->active_list, 0, sizeof(self->active_list));

    ofd_tx_init(&self->base, PICOTM_LIBC_FILE_TYPE_DIR,
                ref_ofd_tx, unref_ofd_tx);

    self->dir = NULL;

    self->flags = 0;

    self->fcntltab = NULL;
    self->fcntltablen = 0;

    self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;

    picotm_rwstate_init(&self->rwstate);
}

void
dir_tx_uninit(struct dir_tx* self)
{
    assert(self);

    fcntloptab_clear(&self->fcntltab, &self->fcntltablen);
}

/*
 * Validation
 */

void
dir_tx_lock(struct dir_tx* self)
{
    assert(self);
}

void
dir_tx_unlock(struct dir_tx* self)
{
    assert(self);
}

static void
validate_noundo(struct dir_tx* self, struct picotm_error* error)
{ }

static void
validate_2pl(struct dir_tx* self, struct picotm_error* error)
{
    assert(self);
}

void
dir_tx_validate(struct dir_tx* self, struct picotm_error* error)
{
    static void (* const validate[])(struct dir_tx*, struct picotm_error*) = {
        validate_noundo,
        validate_2pl
    };

    if (!dir_tx_holds_ref(self)) {
        return;
    }

    validate[self->cc_mode](self, error);
}

/*
 * Update CC
 */

static void
update_cc_noundo(struct dir_tx* self, struct picotm_error* error)
{ }

static void
update_cc_2pl(struct dir_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_2PL);

    /* release lock on directory */
    dir_unlock_state(self->dir, &self->rwstate);
}

void
dir_tx_update_cc(struct dir_tx* self, struct picotm_error* error)
{
    static void (* const update_cc[])(struct dir_tx*, struct picotm_error*) = {
        update_cc_noundo,
        update_cc_2pl
    };

    assert(dir_tx_holds_ref(self));

    update_cc[self->cc_mode](self, error);
}

/*
 * Clear CC
 */

static void
clear_cc_noundo(struct dir_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO);
}

static void
clear_cc_2pl(struct dir_tx* self, struct picotm_error* error)
{
    assert(self);
    assert(self->cc_mode == PICOTM_LIBC_CC_MODE_2PL);

    /* release lock on directory */
    dir_unlock_state(self->dir, &self->rwstate);
}

void
dir_tx_clear_cc(struct dir_tx* self, struct picotm_error* error)
{
    static void (* const clear_cc[])(struct dir_tx*, struct picotm_error*) = {
        clear_cc_noundo,
        clear_cc_2pl
    };

    assert(dir_tx_holds_ref(self));

    clear_cc[self->cc_mode](self, error);
}

/*
 * Referencing
 */

void
dir_tx_ref_or_set_up(struct dir_tx* self, struct dir* dir,
                        int fildes, unsigned long flags,
                        struct picotm_error* error)
{
    assert(self);
    assert(dir);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    /* acquire reference on directory */
    dir_ref(dir);

    /* setup fields */

    self->dir = dir;
    self->cc_mode = dir_get_cc_mode(dir);
    self->flags = 0;

    self->fcntltablen = 0;

    picotm_rwstate_set_status(&self->rwstate, PICOTM_RWSTATE_UNLOCKED);
}

void
dir_tx_ref(struct dir_tx* self)
{
    picotm_ref_up(&self->ref);
}

void
dir_tx_unref(struct dir_tx* self)
{
    assert(self);

    bool final_ref = picotm_ref_down(&self->ref);
    if (!final_ref) {
        return;
    }

    dir_unref(self->dir);
    self->dir = NULL;
}

bool
dir_tx_holds_ref(struct dir_tx* self)
{
    assert(self);

    return picotm_ref_count(&self->ref) > 0;
}

/*
 * fcntl()
 */

static int
fcntl_exec_noundo(struct dir_tx* self, int fildes, int cmd,
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
fcntl_exec_2pl(struct dir_tx* self, int fildes, int cmd,
               union fcntl_arg* arg, int* cookie, struct picotm_error* error)
{
    assert(arg);

    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN: {

            /* Read-lock directory */
            dir_try_rdlock_state(self->dir, &self->rwstate, error);
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

            /* Read-lock directory */
            dir_try_rdlock_state(self->dir, &self->rwstate, error);
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
dir_tx_fcntl_exec(struct dir_tx* self, int fildes, int cmd,
                  union fcntl_arg* arg, bool isnoundo, int* cookie,
                  struct picotm_error* error)
{
    static int (* const fcntl_exec[2])(struct dir_tx*,
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
dir_tx_fcntl_apply(struct dir_tx* self, int fildes, int cookie,
                   struct picotm_error* error)
{ }

void
dir_tx_fcntl_undo(struct dir_tx* self, int fildes, int cookie,
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
dir_tx_fsync_exec(struct dir_tx* self, int fildes, bool isnoundo, int* cookie,
                  struct picotm_error* error)
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
dir_tx_fsync_apply(struct dir_tx* self, int fildes, int cookie,
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
dir_tx_fsync_undo(struct dir_tx* self, int fildes, int cookie,
                  struct picotm_error* error)
{
    static void (* const fsync_undo[2])(int, int, struct picotm_error*) = {
        NULL,
        fsync_undo_2pl
    };

    assert(fsync_undo[self->cc_mode]);

    fsync_undo[self->cc_mode](fildes, cookie, error);
}
