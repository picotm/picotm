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

#include "filetab.h"
#include "picotm/picotm-lib-tab.h"
#include "picotm/picotm-module.h"
#include <string.h>

void
fildes_filetab_init(struct fildes_filetab self[static 1],
                    const struct fildes_filetab_ops ops[static 1],
                    size_t siz, unsigned char tab[siz], size_t stride,
                    struct picotm_error error[static 1])
{
    self->ops = ops;
    self->len = 0;
    self->siz = siz;
    self->tab = tab;
    self->stride = stride;

    int err = pthread_rwlock_init(&self->rwlock, nullptr);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static size_t
file_uninit_walk_cb(void* data0, void* data1,
                    struct picotm_error error[static 1])
{
    struct file* file = data0;
    struct fildes_filetab* filetab = data1;

    filetab->ops->uninit_file(file);

    return 1;
}

void
fildes_filetab_uninit(struct fildes_filetab self[static 1])
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_2(self->tab, self->len, self->stride,
                     file_uninit_walk_cb, self, &error);

    pthread_rwlock_destroy(&self->rwlock);
}

static void
rdlock_filetab(struct fildes_filetab filetab[static 1],
               struct picotm_error error[static 1])
{
    int err = pthread_rwlock_rdlock(&filetab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
wrlock_filetab(struct fildes_filetab filetab[static 1],
               struct picotm_error error[static 1])
{
    int err = pthread_rwlock_wrlock(&filetab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
unlock_filetab(struct fildes_filetab filetab[static 1])
{
    do {
        int err = pthread_rwlock_unlock(&filetab->rwlock);
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

static struct file*
filetab_at(const struct fildes_filetab filetab[static 1], size_t i)
{
    return (struct file*)(filetab->tab + (i * filetab->stride));
}

static struct file*
filetab_begin(const struct fildes_filetab filetab[static 1])
{
    return filetab_at(filetab, 0);
}

static const struct file*
filetab_end(const struct fildes_filetab filetab[static 1])
{
    return filetab_at(filetab, filetab->len);
}

static struct file*
filetab_next(struct fildes_filetab filetab[static 1],
             struct file file[static 1])
{
    return (struct file*)((unsigned char*)file + filetab->stride);
}

/* requires a writer lock */
static struct file*
filetab_append(struct fildes_filetab filetab[static 1],
               struct picotm_error error[static 1])
{
    if ((filetab->len * filetab->stride) >= filetab->siz) {
        /* Return error if not enough ids available */
        picotm_error_set_conflicting(error, nullptr);
        return nullptr;
    }

    struct file* file = filetab_at(filetab, filetab->len);

    filetab->ops->init_file(file, error);
    if (picotm_error_is_set(error))
        return nullptr;

    ++filetab->len;

    return file;
}

/* requires reader lock */
static struct file*
find_by_id(struct fildes_filetab filetab[static 1],
           const struct file_id id[static 1], bool ne_fildes,
           struct picotm_error error[static 1])
{
    struct picotm_error saved_error = PICOTM_ERROR_INITIALIZER;

    struct file* file_beg = filetab_begin(filetab);
    const struct file* file_end = filetab_end(filetab);

    while (file_beg < file_end) {

        struct picotm_error cmp_error = PICOTM_ERROR_INITIALIZER;

        bool found = file_ref_if_id(file_beg, id, ne_fildes, &cmp_error);
        if (picotm_error_is_set(&cmp_error)) {
            /* An error might be reported if the id's file descriptors don't
             * match. We save the error, but continue the loops. If we later
             * find a full match, the function succeeds. Otherwise, it reports
             * the last error. */
            memcpy(&saved_error, &cmp_error, sizeof(saved_error));
        } else if (found) {
            return file_beg;
        }

        file_beg = filetab_next(filetab, file_beg);
    }

    if (picotm_error_is_set(&saved_error))
        memcpy(error, &saved_error, sizeof(*error));

    return nullptr;
}

/* requires writer lock */
static struct file*
search_by_id(struct fildes_filetab filetab[static 1],
             const struct file_id id[static 1], int fildes,
             struct picotm_error error[static 1])
{
    struct file* file_beg = filetab_begin(filetab);
    const struct file* file_end = filetab_end(filetab);

    while (file_beg < file_end) {
        bool found = file_ref_or_set_up_if_id(file_beg, fildes, false, id, error);
        if (picotm_error_is_set(error))
            return nullptr;
        else if (found)
            return file_beg; /* set-up file structure; return */

        file_beg = filetab_next(filetab, file_beg);
    }

    return nullptr;
}

static bool
error_ne_fildes(bool new_file)
{
    return !new_file;
}

struct file*
fildes_filetab_ref_fildes(struct fildes_filetab self[static 1], int fildes,
                          bool new_file, struct picotm_error error[static 1])
{
    struct file_id id;
    file_id_init_from_fildes(&id, fildes, error);
    if (picotm_error_is_set(error))
        return nullptr;

    struct file* file;

    /* Try to find an existing file structure with the given id; iff
     * a new element was not explicitly requested.
     */

    if (!new_file) {
        rdlock_filetab(self, error);
        if (picotm_error_is_set(error))
            return nullptr;

        file = find_by_id(self, &id, error_ne_fildes(new_file), error);
        if (picotm_error_is_set(error))
            goto err_unlock_filetab;
        else if (file)
            goto out; /* found file for id; return */

        unlock_filetab(self);
    }

    /* Not found or new entry is requested; acquire writer lock to
     * create a new entry in the file table.
     */
    wrlock_filetab(self, error);
    if (picotm_error_is_set(error))
        return nullptr;

    if (!new_file) {
        /* Re-try find operation; maybe element was added meanwhile. */
        file = find_by_id(self, &id, error_ne_fildes(new_file), error);
        if (picotm_error_is_set(error))
            goto err_unlock_filetab;
        else if (file)
            goto out; /* found file for id; return */
    }

    /* No entry with the id exists; try to set up an existing, but
     * currently unused, file structure.
     */

    struct file_id empty_id;
    file_id_clear(&empty_id);

    file = search_by_id(self, &empty_id, fildes, error);
    if (picotm_error_is_set(error))
        goto err_unlock_filetab;
    else if (file)
        goto out; /* found file for id; return */

    /* The file table is full; create a new entry for the file id at
     * the end of the table.
     */
    file = filetab_append(self, error);
    if (picotm_error_is_set(error))
        goto err_unlock_filetab;

    /* To perform the setup, we must have acquired a writer lock at
     * this point. No other transaction may interfere.
     */
    file_ref_or_set_up(file, fildes, error);
    if (picotm_error_is_set(error))
        goto err_unlock_filetab;

out:
    unlock_filetab(self);

    return file;

err_unlock_filetab:
    unlock_filetab(self);
    return nullptr;
}

size_t
fildes_filetab_index(struct fildes_filetab self[static 1],
                     struct file file[static 1])
{
    return ((unsigned char*)file - self->tab) / self->stride;
}
