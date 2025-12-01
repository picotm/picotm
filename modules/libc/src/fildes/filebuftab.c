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

#include "filebuftab.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-tab.h"
#include "picotm/picotm-module.h"

void
fildes_filebuftab_init(struct fildes_filebuftab self[static 1],
                       const struct fildes_filebuftab_ops ops[static 1],
                       size_t siz, unsigned char tab[siz], size_t stride,
                       struct picotm_error error[static 1])
{
    self->ops = ops;
    self->len = 0;
    self->siz = siz;
    self->tab = tab;
    self->stride = stride;

    int err = pthread_rwlock_init(&self->rwlock, NULL);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static size_t
filebuf_uninit_walk_cb(void* data0, void* data1,
                       struct picotm_error error[static 1])
{
    struct filebuf* filebuf = data0;
    struct fildes_filebuftab* filebuftab = data1;

    filebuftab->ops->uninit_filebuf(filebuf);

    return 1;
}

void
fildes_filebuftab_uninit(struct fildes_filebuftab self[static 1])
{
    struct picotm_error ignored = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_2(self->tab, self->len, self->stride,
                     filebuf_uninit_walk_cb, self, &ignored);

    pthread_rwlock_destroy(&self->rwlock);
}

static void
rdlock_filebuftab(struct fildes_filebuftab filebuftab[static 1],
                  struct picotm_error error[static 1])
{
    int err = pthread_rwlock_rdlock(&filebuftab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
wrlock_filebuftab(struct fildes_filebuftab filebuftab[static 1],
                  struct picotm_error error[static 1])
{
    int err = pthread_rwlock_wrlock(&filebuftab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
unlock_filebuftab(struct fildes_filebuftab filebuftab[static 1])
{
    do {
        int err = pthread_rwlock_unlock(&filebuftab->rwlock);
        if (err) {
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_errno(&error, err);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
            continue;
        }
        break;
    } while (true);
}

static struct filebuf*
filebuftab_at(const struct fildes_filebuftab filebuftab[static 1], size_t i)
{
    return (struct filebuf*)(filebuftab->tab + (i * filebuftab->stride));
}

static struct filebuf*
filebuftab_begin(const struct fildes_filebuftab filebuftab[static 1])
{
    return filebuftab_at(filebuftab, 0);
}

static const struct filebuf*
filebuftab_end(const struct fildes_filebuftab filebuftab[static 1])
{
    return filebuftab_at(filebuftab, filebuftab->len);
}

static struct filebuf*
filebuftab_next(struct fildes_filebuftab filebuftab[static 1],
                struct filebuf filebuf[static 1])
{
    return (struct filebuf*)((unsigned char*)filebuf + filebuftab->stride);
}

/* requires a writer lock */
static struct filebuf*
filebuftab_append(struct fildes_filebuftab filebuftab[static 1],
                  struct picotm_error error[static 1])
{
    if ((filebuftab->len * filebuftab->stride) >= filebuftab->siz) {
        /* Return error if not enough ids available */
        picotm_error_set_conflicting(error, NULL);
        return NULL;
    }

    struct filebuf* filebuf = filebuftab_at(filebuftab, filebuftab->len);

    filebuftab->ops->init_filebuf(filebuf, error);
    if (picotm_error_is_set(error))
        return NULL;

    ++filebuftab->len;

    return filebuf;
}

/* requires reader lock */
static struct filebuf*
find_by_id(struct fildes_filebuftab filebuftab[static 1],
           const struct filebuf_id id[static 1])
{
    struct filebuf* filebuf_beg = filebuftab_begin(filebuftab);
    const struct filebuf* filebuf_end = filebuftab_end(filebuftab);

    while (filebuf_beg < filebuf_end) {
        int cmp = filebuf_ref_if_id(filebuf_beg, id);
        if (!cmp)
            return filebuf_beg;

        filebuf_beg = filebuftab_next(filebuftab, filebuf_beg);
    }

    return NULL;
}

/* requires writer lock */
static struct filebuf*
search_by_id(struct fildes_filebuftab filebuftab[static 1],
             const struct filebuf_id id[static 1], int fildes,
             struct picotm_error error[static 1])
{
    struct filebuf* filebuf_beg = filebuftab_begin(filebuftab);
    const struct filebuf* filebuf_end = filebuftab_end(filebuftab);

    while (filebuf_beg < filebuf_end) {
        int cmp = filebuf_ref_or_set_up_if_id(filebuf_beg, fildes, id, error);
        if (picotm_error_is_set(error))
            return NULL;
        else if (!cmp)
            return filebuf_beg; /* set-up filebuf structure; return */

        filebuf_beg = filebuftab_next(filebuftab, filebuf_beg);
    }

    return NULL;
}

struct filebuf*
fildes_filebuftab_ref_fildes(struct fildes_filebuftab self[static 1],
                             int fildes,
                             struct picotm_error error[static 1])
{
    struct filebuf_id id;
    filebuf_id_init_from_fildes(&id, fildes, error);
    if (picotm_error_is_set(error))
        return NULL;

    /* Try to find an existing filebuf structure with the given id.
     */

    rdlock_filebuftab(self, error);
    if (picotm_error_is_set(error))
        return NULL;

    struct filebuf* filebuf = find_by_id(self, &id);
    if (filebuf)
        goto out_unlock_filebuftab; /* found filebuf for id; return */

    unlock_filebuftab(self);

    /* No entry found; acquire writer lock to create a new entry in
     * the filebuf table.
     */

    wrlock_filebuftab(self, error);
    if (picotm_error_is_set(error))
        return NULL;

    /* Re-try find operation; maybe element was added meanwhile. */
    filebuf = find_by_id(self, &id);
    if (filebuf)
        goto out_unlock_filebuftab; /* found filebuf for id; return */

    /* No entry with the id exists; try to set up an existing, but
     * currently unused, filebuf structure.
     */

    struct filebuf_id empty_id;
    filebuf_id_clear(&empty_id);

    filebuf = search_by_id(self, &empty_id, fildes, error);
    if (picotm_error_is_set(error))
        goto err_unlock_filebuftab;
    else if (filebuf)
        goto out_unlock_filebuftab; /* found filebuf for id; return */

    /* The filebuf table is full; create a new entry for the filebuf
     * id at the end of the table.
     */

    filebuf = filebuftab_append(self, error);
    if (picotm_error_is_set(error))
        goto err_unlock_filebuftab;

    /* To perform the setup, we must have acquired a writer lock at
     * this point. No other transaction may interfere.
     */
    filebuf_ref_or_set_up(filebuf, fildes, error);
    if (picotm_error_is_set(error))
        goto err_unlock_filebuftab;

out_unlock_filebuftab:
    unlock_filebuftab(self);

    return filebuf;

err_unlock_filebuftab:
    unlock_filebuftab(self);
    return NULL;
}

size_t
fildes_filebuftab_index(struct fildes_filebuftab self[static 1],
                        struct filebuf filebuf[static 1])
{
    return ((unsigned char*)filebuf - self->tab) / self->stride;
}
