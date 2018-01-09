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
