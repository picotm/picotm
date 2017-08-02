/* Permission is hereby granted, free of charge, to any person obtaining a
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
 */

#pragma once

#include <stdbool.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "picotm/picotm-libc.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct file_tx;
struct ofd_tx;
struct picotm_error;
struct stat;

union fcntl_arg;

/**
 * \brief File operations.
 */
struct file_tx_ops {

    /*
     * Reference counting
     */

    void (*ref)(struct file_tx*);
    void (*unref)(struct file_tx*);

    /*
     * Module interfaces
     */

    void (*lock)(struct file_tx*, struct picotm_error*);
    void (*unlock)(struct file_tx*, struct picotm_error*);
    void (*validate)(struct file_tx*, struct picotm_error*r);
    void (*update_cc)(struct file_tx*, struct picotm_error*);
    void (*clear_cc)(struct file_tx*, struct picotm_error*);

    /*
     * accept()
     */

    int (*accept_exec)(struct file_tx*, struct ofd_tx*, int,
                       struct sockaddr*, socklen_t*, bool, int*,
                       struct picotm_error*);

    void (*accept_apply)(struct file_tx*, struct ofd_tx*, int, int,
                         struct picotm_error*);

    void (*accept_undo)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    /*
     * bind()
     */

    int (*bind_exec)(struct file_tx*, struct ofd_tx*, int,
                     const struct sockaddr*, socklen_t, bool, int*,
                     struct picotm_error*);

    void (*bind_apply)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);

    void (*bind_undo)(struct file_tx*, struct ofd_tx*, int, int,
                      struct picotm_error*);

    /*
     * connect()
     */

    int (*connect_exec)(struct file_tx*, struct ofd_tx*, int,
                        const struct sockaddr*, socklen_t, bool, int*,
                        struct picotm_error*);

    void (*connect_apply)(struct file_tx*, struct ofd_tx*, int, int,
                          struct picotm_error*);

    void (*connect_undo)(struct file_tx*, struct ofd_tx*, int, int,
                         struct picotm_error*);

    /*
     * fchmod()
     */

    int (*fchmod_exec)(struct file_tx*, struct ofd_tx*, int, mode_t, bool,
                       int*, struct picotm_error*);

    void (*fchmod_apply)(struct file_tx*, struct ofd_tx*, int, int,
                         struct picotm_error*);

    void (*fchmod_undo)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    /*
     * fcntl()
     */

    int (*fcntl_exec)(struct file_tx*, struct ofd_tx*, int, int,
                      union fcntl_arg*, bool, int*, struct picotm_error*);

    void (*fcntl_apply)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    void (*fcntl_undo)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);

    /*
     * fstat()
     */

    int (*fstat_exec)(struct file_tx*, struct ofd_tx*, int, struct stat*,
                      bool, int*, struct picotm_error*);

    void (*fstat_apply)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    void (*fstat_undo)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);

    /*
     * fsync()
     */

    int (*fsync_exec)(struct file_tx*, struct ofd_tx*, int, bool, int*,
                      struct picotm_error*);

    void (*fsync_apply)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    void (*fsync_undo)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);

    /*
     * listen()
     */

    int (*listen_exec)(struct file_tx*, struct ofd_tx*, int, int, bool, int*,
                       struct picotm_error*);

    void (*listen_apply)(struct file_tx*, struct ofd_tx*, int, int,
                         struct picotm_error*);

    void (*listen_undo)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    /*
     * lseek()
     */

    off_t (*lseek_exec)(struct file_tx*, struct ofd_tx*, int, off_t, int,
                        bool, int*, struct picotm_error*);

    void (*lseek_apply)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    void (*lseek_undo)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);

    /*
     * pread()
     */

    ssize_t (*pread_exec)(struct file_tx*, struct ofd_tx*, int, void*, size_t,
                          off_t, bool, enum picotm_libc_validation_mode, int*,
                          struct picotm_error*);

    void (*pread_apply)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    void (*pread_undo)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);

    /*
     * pwrite()
     */

    ssize_t (*pwrite_exec)(struct file_tx*, struct ofd_tx*, int, const void*,
                           size_t, off_t, bool, int*, struct picotm_error*);

    void (*pwrite_apply)(struct file_tx*, struct ofd_tx*, int, int,
                         struct picotm_error*);

    void (*pwrite_undo)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    /*
     * read()
     */

    ssize_t (*read_exec)(struct file_tx*, struct ofd_tx*, int, void *buf,
                         size_t, bool, enum picotm_libc_validation_mode, int*,
                         struct picotm_error*);

    void (*read_apply)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);

    void (*read_undo)(struct file_tx*, struct ofd_tx*, int, int,
                      struct picotm_error*);

    /*
     * recv()
     */

    ssize_t (*recv_exec)(struct file_tx*, struct ofd_tx*, int, void*, size_t,
                         int, bool, int*, struct picotm_error*);

    void (*recv_apply)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);

    void (*recv_undo)(struct file_tx*, struct ofd_tx*, int, int,
                      struct picotm_error*);

    /*
     * send()
     */

    ssize_t (*send_exec)(struct file_tx*, struct ofd_tx*, int, const void*,
                         size_t, int, bool, int*, struct picotm_error*);

    void (*send_apply)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);

    void (*send_undo)(struct file_tx*, struct ofd_tx*, int, int,
                      struct picotm_error*);

    /*
     * shutdown()
     */

    int (*shutdown_exec)(struct file_tx*, struct ofd_tx*, int, int, bool,
                         int*, struct picotm_error*);

    void (*shutdown_apply)(struct file_tx*, struct ofd_tx*, int, int,
                           struct picotm_error*);

    void (*shutdown_undo)(struct file_tx*, struct ofd_tx*, int, int,
                          struct picotm_error*);

    /*
     * write()
     */

    ssize_t (*write_exec)(struct file_tx*, struct ofd_tx*, int, const void*,
                          size_t, bool, int*, struct picotm_error*);

    void (*write_apply)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    void (*write_undo)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);
};

/**
 * Holds transaction-local state for a file.
 */
struct file_tx {

    SLIST_ENTRY(file_tx) active_list;

    const struct file_tx_ops* ops;

    enum picotm_libc_file_type type;
};

/**
 * \brief Init transaction-local file state.
 * \param   self    A file transaction.
 * \param   type    The open file description's file type.
 */
void
file_tx_init(struct file_tx* self, enum picotm_libc_file_type type,
             const struct file_tx_ops* ops);

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
 * Acquire a reference on a file transaction.
 * \param   self    A file transaction.
 */
void
file_tx_ref(struct file_tx* self);

/**
 * Release a reference on a file transaction.
 * \param   self    A file transaction.
 */
void
file_tx_unref(struct file_tx* self);

/*
 * Module interfaces
 */

/**
 * Locks the transaction-local file state before commit or validation.
 * \param   self        The transaction-local file state.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_tx_lock(struct file_tx* self, struct picotm_error* error);

/**
 * Unlocks the transaction-local file state after commit or validation.
 * \param   self        The transaction-local file state.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_tx_unlock(struct file_tx* self, struct picotm_error* error);

/**
 * Validates the transaction-local file state.
 * \param       self    The transaction-local file state.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_tx_validate(struct file_tx* self, struct picotm_error* error);

/**
 * \brief Updates the concurrency control on transaction-local file state
 *        after a successful apply.
 * \param       self    The transaction-local file state.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_tx_update_cc(struct file_tx* self, struct picotm_error* error);

/**
 * \brief Clears the concurrency control on transaction-local file state
 *        after a successful apply.
 * \param       self    The transaction-local file state.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_tx_clear_cc(struct file_tx* self, struct picotm_error* error);
