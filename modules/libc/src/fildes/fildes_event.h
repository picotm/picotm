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

#pragma once

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

/**
 * \brief Opcodes for file-descriptor events.
 */
enum fildes_op {
    /** \brief Represents an accept() operation. */
    FILDES_OP_ACCEPT,
    /** \brief Represents a bind() operation. */
    FILDES_OP_BIND,
    /** \brief Represents a close() operation. */
    FILDES_OP_CLOSE,
    /** \brief Represents a connect() operation. */
    FILDES_OP_CONNECT,
    /** \brief Represents a dup() operation. */
    FILDES_OP_DUP,
    /** \brief Represents a fchmod() operation. */
    FILDES_OP_FCHMOD,
    /** \brief Represents a fcntl() operation. */
    FILDES_OP_FCNTL,
    /** \brief Represents a fstat() operation. */
    FILDES_OP_FSTAT,
    /** \brief Represents a fsync() operation. */
    FILDES_OP_FSYNC,
    /** \brief Represents a listen() operation. */
    FILDES_OP_LISTEN,
    /** \brief Represents an lseek() operation. */
    FILDES_OP_LSEEK,
    /** \brief Represents an mkstemp() operation. */
    FILDES_OP_MKSTEMP,
    /** \brief Represents an open() operation. */
    FILDES_OP_OPEN,
    /** \brief Represents a pipe() operation. */
    FILDES_OP_PIPE,
    /** \brief Represents a pread() operation. */
    FILDES_OP_PREAD,
    /** \brief Represents a pwrite() operation. */
    FILDES_OP_PWRITE,
    /** \brief Represents a read() operation. */
    FILDES_OP_READ,
    /** \brief Represents a recv() operation. */
    FILDES_OP_RECV,
    /** \brief Represents a send() operation. */
    FILDES_OP_SEND,
    /** \brief Represents a shutdown() operation. */
    FILDES_OP_SHUTDOWN,
    /** \brief Represents a socket() operation. */
    FILDES_OP_SOCKET,
    /** \brief Represents a sync() operation. */
    FILDES_OP_SYNC,
    /** \brief Represents a write() operation. */
    FILDES_OP_WRITE,
    /** \brief The number of file-descriptor operations. */
    LAST_FILDES_OP
};

/**
 * \brief Represents a file-descriptor event.
 */
struct fildes_event {
    int fildes;
    int cookie;
};
