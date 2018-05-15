/*
 * MIT License
 * Copyright (c) 2018   Thomas Zimmermann <tdz@users.sourceforge.net>
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

#pragma once

#include <stdbool.h>
#include <stddef.h>

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

/*
 * fdtab
 */

struct fd*
fdtab_ref_fildes(int fildes, struct picotm_rwstate* lock_state,
                 struct picotm_error* error);

struct fd*
fdtab_get_fd(int fildes);

void
fdtab_try_rdlock(struct picotm_rwstate* lock_state,
                 struct picotm_error* error);

void
fdtab_try_wrlock(struct picotm_rwstate* lock_state,
                 struct picotm_error* error);

void
fdtab_unlock(struct picotm_rwstate* lock_state);

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
ofdtab_ref_fildes(int fildes, bool newly_created, struct picotm_error* error);

/**
 * Returns the index of an ofd structure within the ofd table.
 *
 * \param   ofd An ofd structure.
 * \returns The ofd structure's index in the ofd table.
 */
size_t
ofdtab_index(struct ofd* ofd);

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
chrdevtab_ref_fildes(int fildes, struct picotm_error* error);

/**
 * Returns the index of an chrdev structure within the chrdev table.
 *
 * \param   chrdev An chrdev structure.
 * \returns The chrdev structure's index in the chrdev table.
 */
size_t
chrdevtab_index(struct chrdev* chrdev);

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
dirtab_ref_fildes(int fildes, struct picotm_error* error);

/**
 * Returns the index of a dir structure within the dir table.
 *
 * \param   dir An dir structure.
 * \returns The dir structure's index in the dir table.
 */
size_t
dirtab_index(struct dir* dir);

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
fifotab_ref_fildes(int fildes, struct picotm_error* error);

/**
 * Returns the index of an fifo structure within the fifo table.
 *
 * \param   fifo An fifo structure.
 * \returns The fifo structure's index in the fifo table.
 */
size_t
fifotab_index(struct fifo* fifo);

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
regfiletab_ref_fildes(int fildes, struct picotm_error* error);

/**
 * Returns the index of an regfile structure within the regfile table.
 *
 * \param   regfile An regfile structure.
 * \returns The regfile structure's index in the regfile table.
 */
size_t
regfiletab_index(struct regfile* regfile);

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
sockettab_ref_fildes(int fildes, struct picotm_error* error);

/**
 * Returns the index of an socket structure within the socket table.
 *
 * \param   socket An socket structure.
 * \returns The socket structure's index in the socket table.
 */
size_t
sockettab_index(struct socket* socket);
