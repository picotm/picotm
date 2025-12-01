/*
 * picotm - A system-level transaction manager
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "regfile_tx.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"
#include <errno.h>
#include <unistd.h>
#include "fchmodoptab.h"
#include "fcntloptab.h"
#include "regfile_tx_ops.h"
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

/*
 * Public interface
 */

void
regfile_tx_init(struct regfile_tx* self)
{
    assert(self);

    file_tx_init(&self->base, &regfile_tx_ops);

    self->seekbuf_tx = NULL;

    self->fchmodtab = NULL;
    self->fchmodtablen = 0;

    self->fcntltab = NULL;
    self->fcntltablen = 0;

    self->seektab = NULL;
    self->seektablen = 0;

    self->offset = 0;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));
}

void
regfile_tx_uninit(struct regfile_tx* self)
{
    assert(self);

    seekoptab_clear(&self->seektab, &self->seektablen);
    fcntloptab_clear(&self->fcntltab, &self->fcntltablen);
    fchmodoptab_clear(&self->fchmodtab, &self->fchmodtablen);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));
}

void
regfile_tx_prepare(struct regfile_tx* self, struct regfile* regfile,
                   struct picotm_error* error)
{
    assert(self);

    self->seekbuf_tx = NULL;
    self->fchmodtablen = 0;
    self->fcntltablen = 0;
    self->seektablen = 0;
    self->offset = 0;
}

void
regfile_tx_release(struct regfile_tx* self)
{ }

void
regfile_tx_try_rdlock_field(struct regfile_tx* self, enum regfile_field field,
                            struct picotm_error* error)
{
    assert(self);

    regfile_try_rdlock_field(regfile_of_base(self->base.file), field,
                             self->rwstate + field, error);
}

void
regfile_tx_try_wrlock_field(struct regfile_tx* self, enum regfile_field field,
                            struct picotm_error* error)
{
    assert(self);

    regfile_try_wrlock_field(regfile_of_base(self->base.file), field,
                             self->rwstate + field, error);
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

void
regfile_tx_finish(struct regfile_tx* self)
{
    /* release reader/writer locks on file state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    regfile_of_base(self->base.file));
}

off_t
regfile_tx_get_file_offset(struct regfile_tx* self, int fildes,
                           struct picotm_error* error)
{
    assert(self);

    enum picotm_rwstate_status lock_status =
        picotm_rwstate_get_status(self->rwstate + REGFILE_FIELD_FILE_OFFSET);
    if (lock_status != PICOTM_RWSTATE_UNLOCKED)
        return self->offset; /* we have already read the file offset. */

    regfile_tx_try_rdlock_field(self, REGFILE_FIELD_FILE_OFFSET, error);
    if (picotm_error_is_set(error))
        return (off_t)-1;

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
regfile_tx_set_file_offset(struct regfile_tx* self, off_t offset,
                           struct picotm_error* error)
{
    assert(self);

    regfile_tx_try_wrlock_field(self, REGFILE_FIELD_FILE_OFFSET, error);
    if (picotm_error_is_set(error))
        return;

    self->offset = offset;
}
