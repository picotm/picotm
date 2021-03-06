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

#pragma once

#include "chrdevtab.h"
#include "dirtab.h"
#include "fdtab.h"
#include "fifotab.h"
#include "pipebuftab.h"
#include "regfiletab.h"
#include "seekbuftab.h"
#include "sockbuftab.h"
#include "sockettab.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct chrdev;
struct dir;
struct fd;
struct fifo;
struct picotm_error;
struct picotm_rwstate;
struct regfile;
struct socket;

struct fildes {
    struct fildes_fdtab fdtab;

    struct fildes_chrdevtab chrdevtab;
    struct fildes_dirtab dirtab;
    struct fildes_fifotab fifotab;
    struct fildes_regfiletab regfiletab;
    struct fildes_sockettab sockettab;

    struct fildes_pipebuftab pipebuftab;
    struct fildes_seekbuftab seekbuftab;
    struct fildes_sockbuftab sockbuftab;
};

void
fildes_init(struct fildes* self, struct picotm_error* error);

void
fildes_uninit(struct fildes* self);

/*
 * fdtab
 */

struct fd*
fildes_ref_fd(struct fildes* self, int fildes,
              struct picotm_rwstate* lock_state,
              struct picotm_error* error);

struct fd*
fildes_get_fd(struct fildes* self, int fildes);

void
fildes_try_rdlock_fdtab(struct fildes* self,
                        struct picotm_rwstate* lock_state,
                        struct picotm_error* error);

void
fildes_try_wrlock_fdtab(struct fildes* self,
                        struct picotm_rwstate* lock_state,
                        struct picotm_error* error);

void
fildes_unlock_fdtab(struct fildes* self, struct picotm_rwstate* lock_state);

/*
 * chrdevtab
 */

/**
 * Returns a reference to an chrdev structure for the given file descriptor.
 *
 * \param       fildes  A file descriptor.
 * \param[out]  error   Returns an error to the caller.
 * \returns A referenced instance of `struct chrdev` that refers to the file
 *          descriptor's character device.
 */
struct chrdev*
fildes_ref_chrdev(struct fildes* self, int fildes, bool new_file,
                  struct picotm_error* error);

/**
 * Returns the index of an chrdev structure within the chrdev table.
 *
 * \param   chrdev An chrdev structure.
 * \returns The chrdev structure's index in the chrdev table.
 */
size_t
fildes_chrdev_index(struct fildes* self, struct chrdev* chrdev);

/*
 * dirtab
 */

/**
 * Returns a reference to a dir structure for the given file descriptor.
 *
 * \param       fildes  A file descriptor.
 * \param[out]  error   Returns an error to the caller.
 * \returns A referenced instance of `struct dir` that refers to the file
 *          descriptor's directory.
 */
struct dir*
fildes_ref_dir(struct fildes* self, int fildes, bool new_file,
               struct picotm_error* error);

/**
 * Returns the index of a dir structure within the dir table.
 *
 * \param   dir An dir structure.
 * \returns The dir structure's index in the dir table.
 */
size_t
fildes_dir_index(struct fildes* self, struct dir* dir);

/*
 * fifotab
 */

/**
 * Returns a reference to a fifo structure for the given file descriptor.
 *
 * \param       fildes  A file descriptor.
 * \param[out]  error   Returns an errorto the caller.
 * \returns A referenced instance of `struct fifo` that refers to the file
 *          descriptor's FIFO buffer.
 */
struct fifo*
fildes_ref_fifo(struct fildes* self, int fildes, bool new_file,
                struct picotm_error* error);

/**
 * Returns the index of an fifo structure within the fifo table.
 *
 * \param   fifo An fifo structure.
 * \returns The fifo structure's index in the fifo table.
 */
size_t
fildes_fifo_index(struct fildes* self, struct fifo* fifo);

/*
 * regfiletab
 */

/**
 * Returns a reference to a regfile structure for the given file descriptor.
 *
 * \param       fildes  A file descriptor.
 * \param[out]  error   Returns an error to the caller.
 * \returns A referenced instance of `struct regfile` that refers to the file
 *          descriptor's regular file.
 */
struct regfile*
fildes_ref_regfile(struct fildes* self, int fildes, bool new_file,
                   struct picotm_error* error);

/**
 * Returns the index of an regfile structure within the regfile table.
 *
 * \param   regfile An regfile structure.
 * \returns The regfile structure's index in the regfile table.
 */
size_t
fildes_regfile_index(struct fildes* self, struct regfile* regfile);

/*
 * sockettab
 */

/**
 * Returns a reference to a socket structure for the given file descriptor.
 *
 * \param       fildes  A file descriptor.
 * \param[out]  error   Returns an error to caller.
 * \returns A referenced instance of `struct socket` that refers to the file
 *          descriptor's socket.
 */
struct socket*
fildes_ref_socket(struct fildes* self, int fildes, bool new_file,
                  struct picotm_error* error);

/**
 * Returns the index of an socket structure within the socket table.
 *
 * \param   socket An socket structure.
 * \returns The socket structure's index in the socket table.
 */
size_t
fildes_socket_index(struct fildes* self, struct socket* socket);

/*
 * pipebuftab
 */

/**
 * Returns a reference to a pipebuf structure for the given file descriptor.
 *
 * \param       fildes  A file descriptor.
 * \param[out]  error   Returns an error to caller.
 * \returns A referenced instance of `struct pipebuf` that refers to the file
 *          descriptor's seekable buffer.
 */
struct pipebuf*
fildes_ref_pipebuf(struct fildes* self, int fildes, bool new_file,
                   struct picotm_error* error);

/**
 * Returns the index of a pipebuf structure within the pipebuf table.
 *
 * \param   pipebuf An pipebuf structure.
 * \returns The pipebuf structure's index in the pipebuf table.
 */
size_t
fildes_pipebuf_index(struct fildes* self, struct pipebuf* pipebuf);

/*
 * seekbuftab
 */

/**
 * Returns a reference to a seekbuf structure for the given file descriptor.
 *
 * \param       fildes  A file descriptor.
 * \param[out]  error   Returns an error to caller.
 * \returns A referenced instance of `struct seekbuf` that refers to the file
 *          descriptor's seekable buffer.
 */
struct seekbuf*
fildes_ref_seekbuf(struct fildes* self, int fildes, bool new_file,
                   struct picotm_error* error);

/**
 * Returns the index of a seekbuf structure within the seekbuf table.
 *
 * \param   seekbuf An seekbuf structure.
 * \returns The seekbuf structure's index in the seekbuf table.
 */
size_t
fildes_seekbuf_index(struct fildes* self, struct seekbuf* seekbuf);

/*
 * sockbuftab
 */

/**
 * Returns a reference to a sockbuf structure for the given file descriptor.
 *
 * \param       fildes  A file descriptor.
 * \param[out]  error   Returns an error to caller.
 * \returns A referenced instance of `struct sockbuf` that refers to the file
 *          descriptor's socket buffer.
 */
struct sockbuf*
fildes_ref_sockbuf(struct fildes* self, int fildes, bool new_file,
                   struct picotm_error* error);

/**
 * Returns the index of a sockbuf structure within the sockbuf table.
 *
 * \param   sockbuf An sockbuf structure.
 * \returns The sockbuf structure's index in the sockbuf table.
 */
size_t
fildes_sockbuf_index(struct fildes* self, struct sockbuf* sockbuf);
