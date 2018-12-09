/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "fchmodoptab.h"
#include "fcntloptab.h"
#include "iooptab.h"
#include "regfile_tx_ops.h"

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

/*
 * Public interface
 */

void
regfile_tx_init(struct regfile_tx* self)
{
    assert(self);

    picotm_ref_init(&self->ref, 0);

    file_tx_init(&self->base, &regfile_tx_ops);

    self->regfile = NULL;

    self->wrmode = PICOTM_LIBC_WRITE_BACK;

    self->wrbuf = NULL;
    self->wrbuflen = 0;
    self->wrbufsiz = 0;

    self->wrtab = NULL;
    self->wrtablen = 0;
    self->wrtabsiz = 0;

    self->rdtab = NULL;
    self->rdtablen = 0;
    self->rdtabsiz = 0;

    self->fchmodtab = NULL;
    self->fchmodtablen = 0;

    self->fcntltab = NULL;
    self->fcntltablen = 0;

    self->file_size = 0;

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
    iooptab_clear(&self->wrtab, &self->wrtablen);
    iooptab_clear(&self->rdtab, &self->rdtablen);
    free(self->wrbuf);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));
}

off_t
regfile_tx_get_file_size(struct regfile_tx* self, int fildes,
                      struct picotm_error* error)
{
    assert(self);

    enum picotm_rwstate_status lock_status =
        picotm_rwstate_get_status(self->rwstate + REGFILE_FIELD_FILE_SIZE);

    if (lock_status != PICOTM_RWSTATE_UNLOCKED) {
        /* fast path: we have already read the file size. */
        return self->file_size;
    }

    /* Read-lock file-size field and ... */
    regfile_tx_try_rdlock_field(self, REGFILE_FIELD_FILE_SIZE, error);
    if (picotm_error_is_set(error)) {
        return (off_t)-1;
    }

    /* ...get current file buffer size. */
    struct stat buf;
    int res = fstat(fildes, &buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return (off_t)res;
    }
    self->file_size = buf.st_size;

    return self->file_size;
}

void
regfile_tx_set_file_size(struct regfile_tx* self, off_t size,
                         struct picotm_error* error)
{
    assert(self);

    regfile_tx_try_wrlock_field(self, REGFILE_FIELD_FILE_SIZE, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    self->file_size = size;
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
    regfile_ref(regfile, error);
    if (picotm_error_is_set(error)) {
        goto err_regfile_ref;
    }

    /* setup fields */

    self->regfile = regfile;

    self->wrmode = PICOTM_LIBC_WRITE_BACK;

    self->file_size = 0;

    self->fchmodtablen = 0;
    self->fcntltablen = 0;
    self->rdtablen = 0;
    self->wrtablen = 0;
    self->wrbuflen = 0;

    self->locktablen = 0;

    return;

err_regfile_ref:
    picotm_ref_down(&self->ref);
}

void
regfile_tx_ref(struct regfile_tx* self, struct picotm_error* error)
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
