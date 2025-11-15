/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann
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

#include "fdtab.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-rwstate.h"
#include "picotm/picotm-lib-tab.h"
#include "picotm/picotm-module.h"
#include "fd.h"

void
fildes_fdtab_init(struct fildes_fdtab* self, struct picotm_error* error)
{
    self->len = 0;

    int err = pthread_rwlock_init(&self->rwlock, NULL);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }

    picotm_rwlock_init(&self->lock);
}

static size_t
fd_uninit_walk_cb(void* fd, struct picotm_error* error)
{
    fd_uninit(fd);
    return 1;
}

void
fildes_fdtab_uninit(struct fildes_fdtab* self)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(self->tab, picotm_arraylen(self->tab),
                     sizeof(self->tab[0]), fd_uninit_walk_cb,
                     &error);

    pthread_rwlock_destroy(&self->rwlock);
}

static void
rdlock_fdtab(struct fildes_fdtab* fdtab, struct picotm_error* error)
{
    int err = pthread_rwlock_rdlock(&fdtab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
wrlock_fdtab(struct fildes_fdtab* fdtab, struct picotm_error* error)
{
    int err = pthread_rwlock_wrlock(&fdtab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
unlock_fdtab(struct fildes_fdtab* fdtab)
{
    do {
        int err = pthread_rwlock_unlock(&fdtab->rwlock);
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

/* requires reader lock */
static struct fd*
find_by_id(struct fildes_fdtab* fdtab, int fildes, struct picotm_error* error)
{
    if (fdtab->len <= (size_t)fildes) {
        return NULL;
    }

    struct fd* fd = fdtab->tab + fildes;

    fd_ref_or_set_up(fd, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return fd;
}

/* requires writer lock */
static struct fd*
search_by_id(struct fildes_fdtab* fdtab, int fildes,
             struct picotm_error* error)
{
    struct fd* fd_beg = picotm_arrayat(fdtab->tab, fdtab->len);
    const struct fd* fd_end = picotm_arrayat(fdtab->tab, fildes + 1);

    while (fd_beg < fd_end) {

        fd_init(fd_beg, error);
        if (picotm_error_is_set(error)) {
            return NULL;
        }

        ++fdtab->len;
        ++fd_beg;
    }

    struct fd* fd = fdtab->tab + fildes;

    fd_ref_or_set_up(fd, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return fd;
}

struct fd*
fildes_fdtab_ref_fildes(struct fildes_fdtab* self, int fildes,
                        struct picotm_rwstate* lock_state,
                        struct picotm_error* error)
{
    /* Try to find an existing fd structure with the given file
     * descriptor.
     */

    rdlock_fdtab(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct fd* fd = find_by_id(self, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_find_by_id;
    } else if (fd) {
        goto unlock; /* found fd for fildes; return */
    }

    unlock_fdtab(self);

    /* Not found; acquire writer lock to create a new entry
     * in the file-descriptor table.
     */

    wrlock_fdtab(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    fd = search_by_id(self, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_search_by_id;
        return NULL;
    }

unlock:
    unlock_fdtab(self);

    return fd;

err_search_by_id:
err_find_by_id:
    unlock_fdtab(self);
    return NULL;
}

struct fd*
fildes_fdtab_get_fd(struct fildes_fdtab* self, int fildes)
{
    return self->tab + fildes;
}

void
fildes_fdtab_try_rdlock(struct fildes_fdtab* self,
                        struct picotm_rwstate* lock_state,
                        struct picotm_error* error)
{
    picotm_rwstate_try_rdlock(lock_state, &self->lock, error);
}

void
fildes_fdtab_try_wrlock(struct fildes_fdtab* self,
                        struct picotm_rwstate* lock_state,
                        struct picotm_error* error)
{
    picotm_rwstate_try_wrlock(lock_state, &self->lock, error);
}

void
fildes_fdtab_unlock(struct fildes_fdtab* self,
                    struct picotm_rwstate* lock_state)
{
    picotm_rwstate_unlock(lock_state, &self->lock);
}
