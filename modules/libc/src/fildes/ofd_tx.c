/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
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
 *
 * SPDX-License-Identifier: MIT
 */

#include "ofd_tx.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "file_tx.h"
#include "seekop.h"
#include "seekoptab.h"

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
                struct ofd* ofd)
{
    enum ofd_field field = 0;

    while (beg < end) {
        ofd_unlock_field(ofd, field, beg);
        ++field;
        ++beg;
    }
}

void
ofd_tx_init(struct ofd_tx* self)
{
    assert(self);

    picotm_ref_init(&self->ref, 0);

    memset(&self->active_list, 0, sizeof(self->active_list));

    self->ofd = NULL;

    self->file_tx = NULL;

    self->seektab = NULL;
    self->seektablen = 0;

    self->offset = 0;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));
}

void
ofd_tx_uninit(struct ofd_tx* self)
{
    assert(self);

    seekoptab_clear(&self->seektab, &self->seektablen);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));
}

bool
ofd_tx_holds_ref(struct ofd_tx* self)
{
    assert(self);

    return !!self->ofd;
}

int
ofd_tx_cmp_with_id(const struct ofd_tx* self, const struct ofd_id* id)
{
    assert(self);
    assert(self->ofd);

    return ofd_id_cmp(&self->ofd->id, id);
}

off_t
ofd_tx_get_file_offset(struct ofd_tx* self, int fildes,
                       struct picotm_error* error)
{
    assert(self);

    enum picotm_rwstate_status lock_status =
        picotm_rwstate_get_status(self->rwstate + OFD_FIELD_FILE_OFFSET);

    if (lock_status != PICOTM_RWSTATE_UNLOCKED) {
        /* fast path: we have already read the file offset. */
        return self->offset;
    }

    ofd_tx_try_rdlock_field(self, OFD_FIELD_FILE_OFFSET, error);
    if (picotm_error_is_set(error)) {
        return (off_t)-1;
    }

    /* read file offset from file descriptor */
    off_t res = lseek(fildes, 0, SEEK_CUR);
    if (res == (off_t)-1) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    self->offset = res;

    return self->offset;
}

void
ofd_tx_set_file_offset(struct ofd_tx* self, off_t offset,
                       struct picotm_error* error)
{
    assert(self);

    ofd_tx_try_wrlock_field(self, OFD_FIELD_FILE_OFFSET, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    self->offset = offset;
}

/*
 * Reference counting
 */

void
ofd_tx_ref_or_set_up(struct ofd_tx* self, struct ofd* ofd,
                     struct file_tx* file_tx, struct picotm_error* error)
{
    assert(self);
    assert(ofd);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    /* get references */

    ofd_ref(ofd, error);
    if (picotm_error_is_set(error)) {
        goto err_ofd_ref;
    }

    file_tx_ref(file_tx, error);
    if (picotm_error_is_set(error)) {
        goto err_file_tx_ref;
    }

    /* setup fields */

    self->ofd = ofd;
    self->file_tx = file_tx;

    self->offset = 0;

    self->seektablen = 0;

    return;

err_file_tx_ref:
    ofd_unref(ofd);
err_ofd_ref:
    picotm_ref_down(&self->ref);
}

void
ofd_tx_ref(struct ofd_tx* self, struct picotm_error* error)
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

    file_tx_unref(self->file_tx);
    ofd_unref(self->ofd);

    self->ofd = NULL;
}

/*
 * Locking
 */

void
ofd_tx_try_rdlock_field(struct ofd_tx* self, enum ofd_field field,
                        struct picotm_error* error)
{
    assert(self);

    ofd_try_rdlock_field(self->ofd, field, self->rwstate + field, error);
}

void
ofd_tx_try_wrlock_field(struct ofd_tx* self, enum ofd_field field,
                        struct picotm_error* error)
{
    assert(self);

    ofd_try_wrlock_field(self->ofd, field, self->rwstate + field, error);
}

/*
 * Module interface
 */

void
ofd_tx_lock(struct ofd_tx* self, struct picotm_error* error)
{
    assert(self);
}

void
ofd_tx_unlock(struct ofd_tx* self, struct picotm_error* error)
{
    assert(self);
}

void
ofd_tx_validate(struct ofd_tx* self, struct picotm_error* error)
{
    /* Locked regions are ours, so we do not need to validate here. All
     * conflicting transactions will have aborted on encountering our locks.
     *
     * The state of the OFD itself is guarded by ofd::rwlock.
     */
}

void
ofd_tx_update_cc(struct ofd_tx* self, struct picotm_error* error)
{
    assert(self);

    /* release reader/writer locks on open-file-description state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->ofd);
}

void
ofd_tx_clear_cc(struct ofd_tx* self, struct picotm_error* error)
{
    assert(self);

    /* release reader/writer locks on open-file-description state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->ofd);
}
