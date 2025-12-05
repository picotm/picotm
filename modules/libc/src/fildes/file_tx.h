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

#include "picotm/picotm-libc.h"
#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-ref.h"
#include "picotm/picotm-lib-slist.h"

#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct file;
struct file_tx;
struct picotm_error;
struct stat;

union fcntl_arg;

/**
 * \brief File operations.
 */
struct file_tx_ops {

    enum picotm_libc_file_type type;

    /*
     * File handling
     */

    void (*prepare)(struct file_tx*, struct file*, void*, struct picotm_error*);
    void (*release)(struct file_tx*);

    /*
     * Module interfaces
     */

    void (*finish)(struct file_tx*);

    /*
     * accept()
     */

    int (*accept_exec)(struct file_tx*,
                       int, struct sockaddr*, socklen_t*,
                       bool, int*, struct picotm_error*);
    void (*accept_apply)(struct file_tx*, int, int, struct picotm_error*);
    void (*accept_undo)(struct file_tx*, int, int, struct picotm_error*);

    /*
     * bind()
     */

    int (*bind_exec)(struct file_tx*,
                     int, const struct sockaddr*, socklen_t,
                     bool, int*, struct picotm_error*);
    void (*bind_apply)(struct file_tx*, int, int, struct picotm_error*);
    void (*bind_undo)(struct file_tx*, int, int, struct picotm_error*);

    /*
     * connect()
     */

    int (*connect_exec)(struct file_tx*,
                        int, const struct sockaddr*, socklen_t,
                        bool, int*, struct picotm_error*);
    void (*connect_apply)(struct file_tx*, int, int, struct picotm_error*);
    void (*connect_undo)(struct file_tx*, int, int, struct picotm_error*);

    /*
     * fchmod()
     */

    int (*fchmod_exec)(struct file_tx*,
                       int, mode_t,
                       bool, int*, struct picotm_error*);
    void (*fchmod_apply)(struct file_tx*, int, int, struct picotm_error*);
    void (*fchmod_undo)(struct file_tx*, int, int, struct picotm_error*);

    /*
     * fcntl()
     */

    int (*fcntl_exec)(struct file_tx*,
                      int, int, union fcntl_arg*,
                      bool, int*, struct picotm_error*);
    void (*fcntl_apply)(struct file_tx*, int, int, struct picotm_error*);
    void (*fcntl_undo)(struct file_tx*, int, int, struct picotm_error*);

    /*
     * fstat()
     */

    int (*fstat_exec)(struct file_tx*,
                      int, struct stat*,
                      bool, int*, struct picotm_error*);
    void (*fstat_apply)(struct file_tx*, int, int, struct picotm_error*);
    void (*fstat_undo)(struct file_tx*, int, int, struct picotm_error*);

    /*
     * fsync()
     */

    int (*fsync_exec)(struct file_tx*,
                      int,
                      bool, int*, struct picotm_error*);
    void (*fsync_apply)(struct file_tx*, int, int, struct picotm_error*);
    void (*fsync_undo)(struct file_tx*, int, int, struct picotm_error*);

    /*
     * listen()
     */

    int (*listen_exec)(struct file_tx*,
                       int, int,
                       bool, int*, struct picotm_error*);
    void (*listen_apply)(struct file_tx*, int, int, struct picotm_error*);
    void (*listen_undo)(struct file_tx*, int, int, struct picotm_error*);

    /*
     * lseek()
     */

    off_t (*lseek_exec)(struct file_tx*,
                        int, off_t, int,
                        bool, int*, struct picotm_error*);
    void (*lseek_apply)(struct file_tx*, int, int, struct picotm_error*);
    void (*lseek_undo)(struct file_tx*, int, int, struct picotm_error*);

    /*
     * pread()
     */

    ssize_t (*pread_exec)(struct file_tx*,
                          int, void*, size_t, off_t,
                          bool, int*, struct picotm_error*);
    void (*pread_apply)(struct file_tx*, int, int, struct picotm_error*);
    void (*pread_undo)(struct file_tx*, int, int, struct picotm_error*);

    /*
     * pwrite()
     */

    ssize_t (*pwrite_exec)(struct file_tx*,
                           int, const void*, size_t, off_t,
                           bool, int*, struct picotm_error*);
    void (*pwrite_apply)(struct file_tx*, int, int, struct picotm_error*);
    void (*pwrite_undo)(struct file_tx*, int, int, struct picotm_error*);

    /*
     * read()
     */

    ssize_t (*read_exec)(struct file_tx*,
                         int, void *buf, size_t,
                         bool, int*, struct picotm_error*);
    void (*read_apply)(struct file_tx*, int, int, struct picotm_error*);
    void (*read_undo)(struct file_tx*, int, int, struct picotm_error*);

    /*
     * recv()
     */

    ssize_t (*recv_exec)(struct file_tx*,
                         int, void*, size_t, int,
                         bool, int*, struct picotm_error*);
    void (*recv_apply)(struct file_tx*, int, int, struct picotm_error*);
    void (*recv_undo)(struct file_tx*, int, int, struct picotm_error*);

    /*
     * send()
     */

    ssize_t (*send_exec)(struct file_tx*,
                         int, const void*, size_t, int,
                         bool, int*, struct picotm_error*);
    void (*send_apply)(struct file_tx*, int, int, struct picotm_error*);
    void (*send_undo)(struct file_tx*, int, int, struct picotm_error*);

    /*
     * shutdown()
     */

    int (*shutdown_exec)(struct file_tx*,
                         int, int,
                         bool, int*, struct picotm_error*);
    void (*shutdown_apply)(struct file_tx*, int, int, struct picotm_error*);
    void (*shutdown_undo)(struct file_tx*, int, int, struct picotm_error*);

    /*
     * write()
     */

    ssize_t (*write_exec)(struct file_tx*,
                          int, const void*, size_t,
                          bool, int*, struct picotm_error*);
    void (*write_apply)(struct file_tx*, int, int, struct picotm_error*);
    void (*write_undo)(struct file_tx*, int, int, struct picotm_error*);
};

/**
 * Holds transaction-local state for a file.
 */
struct file_tx {

    struct picotm_ref16 ref;

    struct picotm_slist list_entry;

    struct file* file;

    const struct file_tx_ops* ops;
};

static inline struct file_tx*
file_tx_of_list_entry(struct picotm_slist* item)
{
    return picotm_containerof(item, struct file_tx, list_entry);
}

/**
 * \brief Init transaction-local file state.
 * \param   self    A file transaction.
 * \param   ops     The file type's constants and operations.
 */
void
file_tx_init(struct file_tx* self, const struct file_tx_ops* ops);

/**
 * \brief Uninit transaction-local file state.
 * \param   self    A file transaction.
 */
void
file_tx_uninit(struct file_tx* self);

/**
 * \brief Returns the file type.
 * \param   self    A file transaction.
 * \returns The open file description's file type.
 */
enum picotm_libc_file_type
file_tx_file_type(const struct file_tx* self);

/**
 * \brief Sets up a file transaction or acquires a reference on an
 *        already set-up instance.
 * \param       self    A file transaction.
 * \param       file    The global file state.
 * \param       data    User-data.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_tx_ref_or_set_up(struct file_tx* self, struct file* file,
                      void* data, struct picotm_error* error);

/**
 * Acquire a reference on a file transaction.
 * \param       self    A file transaction.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_tx_ref(struct file_tx* self, struct picotm_error* error);

/**
 * Release a reference on a file transaction.
 * \param   self    A file transaction.
 */
void
file_tx_unref(struct file_tx* self);

/**
 * Returns true if the transactions holds a reference to the
 * transaction-local file state.
 */
bool
file_tx_holds_ref(struct file_tx* self);

/*
 * Module interfaces
 */

/**
 * \brief Finishes the transaction-local file state at the end of a
 *        transaction.
 * \param   self    The transaction-local file state.
 */
void
file_tx_finish(struct file_tx* self);

/*
 * Callback functions
 */

#define _file_tx_file_op(_op, _file_op, _file_tx, ...) \
    (_file_tx)->ops->_op ## _ ## _file_op((_file_tx), __VA_ARGS__)

#define file_tx_exec(_op, _file_tx, ...) \
    _file_tx_file_op(_op, exec, (_file_tx), __VA_ARGS__)

#define file_tx_apply(_op, _file_tx, _fildes, _cookie, _error) \
    _file_tx_file_op(_op, apply, (_file_tx), (_fildes), (_cookie), (_error))

#define file_tx_undo(_op, _file_tx, _fildes, _cookie, _error) \
    _file_tx_file_op(_op, undo, (_file_tx), (_fildes), (_cookie), (_error))
