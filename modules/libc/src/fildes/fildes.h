/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
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
#include "ofdtab.h"
#include "regfiletab.h"
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
struct ofd;
struct picotm_error;
struct picotm_rwstate;
struct regfile;
struct socket;

struct fildes {
    struct fildes_fdtab fdtab;

    struct fildes_ofdtab ofdtab;

    struct fildes_chrdevtab chrdevtab;
    struct fildes_dirtab dirtab;
    struct fildes_fifotab fifotab;
    struct fildes_regfiletab regfiletab;
    struct fildes_sockettab sockettab;
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
 * ofdtab
 */

/**
 * Returns a reference to an ofd structure for the given file descriptor.
 *
 * \param       fildes          A file descriptor.
 * \param       newly_created   True if the open file description has been
 *                              newly created.
 * \param[out]  error           Returns an error.
 * \returns A referenced instance of `struct ofd` that refers to the file
 *          descriptor's open file description.
 *
 * We cannot distiguish between open file descriptions. Two
 * file descriptors refering to the same buffer might share
 * the same open file description, or not.
 *
 * As a workaround, we only allow one file descriptor per file
 * buffer at the same time. If we see a second file descriptor
 * refering to a buffer that is already in use, the look-up
 * fails.
 *
 * For a solution, Linux (or any other Unix) has to provide a
 * unique id for each open file description, or at least give
 * us a way of figuring out the relationship between file descriptors
 * and open file descriptions.
 */
struct ofd*
fildes_ref_ofd(struct fildes* self, int fildes, bool newly_created,
               struct picotm_error* error);

/**
 * Returns the index of an ofd structure within the ofd table.
 *
 * \param   ofd An ofd structure.
 * \returns The ofd structure's index in the ofd table.
 */
size_t
fildes_ofd_index(struct fildes* self, struct ofd* ofd);

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
fildes_ref_chrdev(struct fildes* self, int fildes,
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
fildes_ref_dir(struct fildes* self, int fildes, struct picotm_error* error);

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
fildes_ref_fifo(struct fildes* self, int fildes, struct picotm_error* error);

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
fildes_ref_regfile(struct fildes* self, int fildes,
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
fildes_ref_socket(struct fildes* self, int fildes,
                  struct picotm_error* error);

/**
 * Returns the index of an socket structure within the socket table.
 *
 * \param   socket An socket structure.
 * \returns The socket structure's index in the socket table.
 */
size_t
fildes_socket_index(struct fildes* self, struct socket* socket);
