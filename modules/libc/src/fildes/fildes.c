/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018-2020  Thomas Zimmermann <contact@tzimmermann.org>
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

#include "fildes.h"
#include "picotm/picotm-error.h"

void
fildes_init(struct fildes* self, struct picotm_error* error)
{
    fildes_fdtab_init(&self->fdtab, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    fildes_chrdevtab_init(&self->chrdevtab, error);
    if (picotm_error_is_set(error)) {
        goto err_fildes_chrdevtab_init;
    }
    fildes_dirtab_init(&self->dirtab, error);
    if (picotm_error_is_set(error)) {
        goto err_fildes_dirtab_init;
    }
    fildes_fifotab_init(&self->fifotab, error);
    if (picotm_error_is_set(error)) {
        goto err_fildes_fifotab_init;
    }
    fildes_regfiletab_init(&self->regfiletab, error);
    if (picotm_error_is_set(error)) {
        goto err_fildes_regfiletab_init;
    }
    fildes_sockettab_init(&self->sockettab, error);
    if (picotm_error_is_set(error)) {
        goto err_fildes_sockettab_init;
    }

    fildes_pipebuftab_init(&self->pipebuftab, error);
    if (picotm_error_is_set(error)) {
        goto err_fildes_pipebuftab_init;
    }
    fildes_seekbuftab_init(&self->seekbuftab, error);
    if (picotm_error_is_set(error)) {
        goto err_fildes_seekbuftab_init;
    }
    fildes_sockbuftab_init(&self->sockbuftab, error);
    if (picotm_error_is_set(error)) {
        goto err_fildes_sockbuftab_init;
    }

    return;

err_fildes_sockbuftab_init:
    fildes_seekbuftab_uninit(&self->seekbuftab);
err_fildes_seekbuftab_init:
    fildes_pipebuftab_uninit(&self->pipebuftab);
err_fildes_pipebuftab_init:
    fildes_sockettab_uninit(&self->sockettab);
err_fildes_sockettab_init:
    fildes_regfiletab_uninit(&self->regfiletab);
err_fildes_regfiletab_init:
    fildes_fifotab_uninit(&self->fifotab);
err_fildes_fifotab_init:
    fildes_dirtab_uninit(&self->dirtab);
err_fildes_dirtab_init:
    fildes_chrdevtab_uninit(&self->chrdevtab);
err_fildes_chrdevtab_init:
    fildes_fdtab_uninit(&self->fdtab);
}

void
fildes_uninit(struct fildes* self)
{
    fildes_sockbuftab_uninit(&self->sockbuftab);
    fildes_seekbuftab_uninit(&self->seekbuftab);
    fildes_pipebuftab_uninit(&self->pipebuftab);

    fildes_sockettab_uninit(&self->sockettab);
    fildes_regfiletab_uninit(&self->regfiletab);
    fildes_fifotab_uninit(&self->fifotab);
    fildes_dirtab_uninit(&self->dirtab);
    fildes_chrdevtab_uninit(&self->chrdevtab);

    fildes_fdtab_uninit(&self->fdtab);
}

/*
 * fdtab
 */

struct fd*
fildes_ref_fd(struct fildes* self, int fildes,
              struct picotm_rwstate* lock_state,
              struct picotm_error* error)
{
    return fildes_fdtab_ref_fildes(&self->fdtab, fildes, lock_state, error);
}

struct fd*
fildes_get_fd(struct fildes* self, int fildes)
{
    return fildes_fdtab_get_fd(&self->fdtab, fildes);
}

void
fildes_try_rdlock_fdtab(struct fildes* self,
                        struct picotm_rwstate* lock_state,
                        struct picotm_error* error)
{
    fildes_fdtab_try_rdlock(&self->fdtab, lock_state, error);
}

void
fildes_try_wrlock_fdtab(struct fildes* self,
                        struct picotm_rwstate* lock_state,
                        struct picotm_error* error)
{
    fildes_fdtab_try_wrlock(&self->fdtab, lock_state, error);
}

void
fildes_unlock_fdtab(struct fildes* self, struct picotm_rwstate* lock_state)
{
    fildes_fdtab_unlock(&self->fdtab, lock_state);
}

/*
 * chrdevtab
 */

struct chrdev*
fildes_ref_chrdev(struct fildes* self, int fildes, bool new_file,
                  struct picotm_error* error)
{
    return fildes_chrdevtab_ref_fildes(&self->chrdevtab, fildes, new_file,
                                       error);
}

size_t
fildes_chrdev_index(struct fildes* self, struct chrdev* chrdev)
{
    return fildes_chrdevtab_index(&self->chrdevtab, chrdev);
}

/*
 * dirtab
 */

struct dir*
fildes_ref_dir(struct fildes* self, int fildes, bool new_file,
               struct picotm_error* error)
{
    return fildes_dirtab_ref_fildes(&self->dirtab, fildes, new_file, error);
}

size_t
fildes_dir_index(struct fildes* self, struct dir* dir)
{
    return fildes_dirtab_index(&self->dirtab, dir);
}

/*
 * fifotab
 */

struct fifo*
fildes_ref_fifo(struct fildes* self, int fildes, bool new_file,
                struct picotm_error* error)
{
    return fildes_fifotab_ref_fildes(&self->fifotab, fildes, new_file, error);
}

size_t
fildes_fifo_index(struct fildes* self, struct fifo* fifo)
{
    return fildes_fifotab_index(&self->fifotab, fifo);
}

/*
 * regfiletab
 */

struct regfile*
fildes_ref_regfile(struct fildes* self, int fildes, bool new_file,
                   struct picotm_error* error)
{
    return fildes_regfiletab_ref_fildes(&self->regfiletab, fildes, new_file,
                                        error);
}

size_t
fildes_regfile_index(struct fildes* self, struct regfile* regfile)
{
    return fildes_regfiletab_index(&self->regfiletab, regfile);
}

/*
 * sockettab
 */

struct socket*
fildes_ref_socket(struct fildes* self, int fildes, bool new_file,
                  struct picotm_error* error)
{
    return fildes_sockettab_ref_fildes(&self->sockettab, fildes, new_file,
                                       error);
}

size_t
fildes_socket_index(struct fildes* self, struct socket* socket)
{
    return fildes_sockettab_index(&self->sockettab, socket);
}

/*
 * pipebuftab
 */

struct pipebuf*
fildes_ref_pipebuf(struct fildes* self, int fildes, bool new_file,
                   struct picotm_error* error)
{
    return fildes_pipebuftab_ref_fildes(&self->pipebuftab, fildes, error);
}

size_t
fildes_pipebuf_index(struct fildes* self, struct pipebuf* pipebuf)
{
    return fildes_pipebuftab_index(&self->pipebuftab, pipebuf);
}

/*
 * seekbuftab
 */

struct seekbuf*
fildes_ref_seekbuf(struct fildes* self, int fildes, bool new_file,
                   struct picotm_error* error)
{
    return fildes_seekbuftab_ref_fildes(&self->seekbuftab, fildes, error);
}

size_t
fildes_seekbuf_index(struct fildes* self, struct seekbuf* seekbuf)
{
    return fildes_seekbuftab_index(&self->seekbuftab, seekbuf);
}

/*
 * sockbuftab
 */

struct sockbuf*
fildes_ref_sockbuf(struct fildes* self, int fildes, bool new_file,
                   struct picotm_error* error)
{
    return fildes_sockbuftab_ref_fildes(&self->sockbuftab, fildes, error);
}

size_t
fildes_sockbuf_index(struct fildes* self, struct sockbuf* sockbuf)
{
    return fildes_sockbuftab_index(&self->sockbuftab, sockbuf);
}
