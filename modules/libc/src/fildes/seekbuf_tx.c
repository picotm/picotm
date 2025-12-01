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

#include "seekbuf_tx.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "fchmodoptab.h"
#include "fcntloptab.h"
#include "iooptab.h"
#include "region.h"
#include "regiontab.h"

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
seekbuf_tx_init(struct seekbuf_tx* self)
{
    assert(self);

    picotm_ref_init(&self->ref, 0);
    picotm_slist_init_item(&self->list_entry);

    self->seekbuf = nullptr;

    self->wrmode = PICOTM_LIBC_WRITE_BACK;

    self->wrbuf = nullptr;
    self->wrbuflen = 0;
    self->wrbufsiz = 0;

    self->wrtab = nullptr;
    self->wrtablen = 0;
    self->wrtabsiz = 0;

    self->rdtab = nullptr;
    self->rdtablen = 0;
    self->rdtabsiz = 0;

    self->file_size = 0;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));

    rwcountermap_init(&self->rwcountermap);

    self->locktab = nullptr;
    self->locktablen = 0;
    self->locktabsiz = 0;
}

void
seekbuf_tx_uninit(struct seekbuf_tx* self)
{
    assert(self);

    free(self->locktab);
    rwcountermap_uninit(&self->rwcountermap);

    iooptab_clear(&self->wrtab, &self->wrtablen);
    iooptab_clear(&self->rdtab, &self->rdtablen);
    free(self->wrbuf);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));

    picotm_slist_uninit_item(&self->list_entry);
}

void
seekbuf_tx_ref_or_set_up(struct seekbuf_tx* self, struct seekbuf *seekbuf,
                         struct picotm_error* error)
{
    assert(self);
    assert(seekbuf);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    filebuf_ref(&seekbuf->base, error);
    if (picotm_error_is_set(error)) {
        goto err_seekbuf_ref;
    }

    self->seekbuf = seekbuf;
    self->wrmode = PICOTM_LIBC_WRITE_BACK;
    self->rdtablen = 0;
    self->wrtablen = 0;
    self->wrbuflen = 0;
    self->file_size = 0;
    self->locktablen = 0;

    return;

err_seekbuf_ref:
    picotm_ref_down(&self->ref);
}

void
seekbuf_tx_ref(struct seekbuf_tx* self)
{
    picotm_ref_up(&self->ref);
}

void
seekbuf_tx_unref(struct seekbuf_tx* self)
{
    assert(self);

    bool final_ref = picotm_ref_down(&self->ref);
    if (!final_ref) {
        return;
    }

    filebuf_unref(&self->seekbuf->base);

    self->seekbuf = nullptr;
}

bool
seekbuf_tx_holds_ref(const struct seekbuf_tx* self)
{
    assert(self);

    return picotm_ref_count(&self->ref) > 0;
}

off_t
seekbuf_tx_get_file_size(struct seekbuf_tx* self, int fildes,
                         struct picotm_error* error)
{
    assert(self);

    enum picotm_rwstate_status lock_status =
        picotm_rwstate_get_status(self->rwstate + SEEKBUF_FIELD_FILE_SIZE);

    if (lock_status != PICOTM_RWSTATE_UNLOCKED) {
        /* fast path: we have already read the file size. */
        return self->file_size;
    }

    /* Read-lock file-size field and ... */
    seekbuf_tx_try_rdlock_field(self, SEEKBUF_FIELD_FILE_SIZE, error);
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
seekbuf_tx_set_file_size(struct seekbuf_tx* self, off_t size,
                         struct picotm_error* error)
{
    assert(self);

    seekbuf_tx_try_wrlock_field(self, SEEKBUF_FIELD_FILE_SIZE, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    self->file_size = size;
}

/*
 * File handling
 */

void
seekbuf_tx_prepare(struct seekbuf_tx* self, struct seekbuf* seekbuf,
                   struct picotm_error* error)
{
    assert(self);

    self->wrmode = PICOTM_LIBC_WRITE_BACK;
    self->file_size = 0;

    self->rdtablen = 0;
    self->wrtablen = 0;
    self->wrbuflen = 0;

    self->locktablen = 0;
}

void
seekbuf_tx_release(struct seekbuf_tx* self)
{ }

void
seekbuf_tx_try_rdlock_field(struct seekbuf_tx* self, enum seekbuf_field field,
                            struct picotm_error* error)
{
    assert(self);

    seekbuf_try_rdlock_field(self->seekbuf, field, self->rwstate + field,
                             error);
}

void
seekbuf_tx_try_wrlock_field(struct seekbuf_tx* self, enum seekbuf_field field,
                            struct picotm_error* error)
{
    assert(self);

    seekbuf_try_wrlock_field(self->seekbuf, field, self->rwstate + field,
                             error);
}

int
seekbuf_tx_try_rdlock_region(struct seekbuf_tx* self, size_t nbyte,
                             off_t offset, struct picotm_error* error)
{
    assert(self);

    seekbuf_try_rdlock_region(self->seekbuf, offset, nbyte,
                              &self->rwcountermap, error);
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

int
seekbuf_tx_try_wrlock_region(struct seekbuf_tx* self, size_t nbyte,
                             off_t offset, struct picotm_error* error)
{
    assert(self);

    seekbuf_try_wrlock_region(self->seekbuf, offset, nbyte,
                              &self->rwcountermap, error);
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

static off_t
append_to_iobuffer(struct seekbuf_tx* self, size_t nbyte, const void* buf,
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
seekbuf_tx_append_to_writeset(struct seekbuf_tx* self, size_t nbyte, off_t offset,
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
seekbuf_tx_append_to_readset(struct seekbuf_tx* self, size_t nbyte, off_t offset,
                             const void* buf, struct picotm_error* error)
{
    assert(self);

    unsigned long res = iooptab_append(&self->rdtab,
                                       &self->rdtablen,
                                       &self->rdtabsiz,
                                       nbyte, offset, 0,
                                       error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return res;
}

static void
unlock_regions(struct region* beg, const struct region* end,
               struct seekbuf* seekbuf, struct rwcountermap* rwcountermap)
{
    while (beg < end) {
        seekbuf_unlock_region(seekbuf, beg->offset, beg->nbyte,
                              rwcountermap);
        ++beg;
    }
}

static void
unlock_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end,
                struct seekbuf* seekbuf)
{
    enum seekbuf_field field = 0;

    while (beg < end) {
        seekbuf_unlock_field(seekbuf, field, beg);
        ++field;
        ++beg;
    }
}

void
seekbuf_tx_finish(struct seekbuf_tx* self)
{
    /* release record locks */
    unlock_regions(self->locktab, self->locktab + self->locktablen,
                   self->seekbuf, &self->rwcountermap);

    /* release reader/writer locks on file state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->seekbuf);
}
