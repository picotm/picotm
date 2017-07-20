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

#include <picotm/picotm-lib-ref.h>
#include <picotm/picotm-lib-rwstate.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "ofd_tx.h"
#include "picotm/picotm-libc.h"
#include "socket.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct socket;
struct stat;

union fcntl_arg;

/**
 * Holds transaction-local reads and writes for an open file description
 */
struct socket_tx {

    struct picotm_ref16 ref;

    SLIST_ENTRY(socket_tx) active_list;

    struct ofd_tx base;

    struct socket* socket;

    unsigned char* wrbuf;
    size_t         wrbuflen;
    size_t         wrbufsiz;

    struct ioop* wrtab;
    size_t       wrtablen;
    size_t       wrtabsiz;

    struct ioop* rdtab;
    size_t       rdtablen;
    size_t       rdtabsiz;

    struct fcntlop* fcntltab;
    size_t          fcntltablen;

    /** CC mode of domain */
    enum picotm_libc_cc_mode cc_mode;

    /** State of the local reader/writer locks. */
    struct picotm_rwstate rwstate[NUMBER_OF_SOCKET_FIELDS];
};

struct socket_tx*
socket_tx_of_ofd_tx(struct ofd_tx* ofd_tx);

/**
 * Init transaction-local open-file-description state
 */
void
socket_tx_init(struct socket_tx* self);

/**
 * Uninit state
 */
void
socket_tx_uninit(struct socket_tx* self);

/**
 * Validate the local state
 */
void
socket_tx_validate(struct socket_tx* self, struct picotm_error* error);

/**
 * Updates the data structures for concurrency control after a successful apply
 */
void
socket_tx_update_cc(struct socket_tx* self, struct picotm_error* error);

/**
 * Clears the data structures for concurrency control after a successful undo
 */
void
socket_tx_clear_cc(struct socket_tx* self, struct picotm_error* error);

/**
 * Acquire a reference on the open file description
 */
void
socket_tx_ref_or_set_up(struct socket_tx* self, struct socket* socket,
                        int fildes, struct picotm_error* error);

/**
 * Acquire a reference on the open file description
 */
void
socket_tx_ref(struct socket_tx* self);

/**
 * Release reference
 */
void
socket_tx_unref(struct socket_tx* self);

/**
 * Returns true if transactions hold a reference
 */
bool
socket_tx_holds_ref(struct socket_tx* self);

int
socket_tx_append_to_writeset(struct socket_tx* self, size_t nbyte,
                             off_t offset, const void* buf,
                             struct picotm_error* error);

int
socket_tx_append_to_readset(struct socket_tx* self, size_t nbyte,
                            off_t offset, const void* buf,
                            struct picotm_error* error);

/**
 * Prepares the open file description for commit
 */
void
socket_tx_lock(struct socket_tx* self);

/**
 * Finishes commit for open file description
 */
void
socket_tx_unlock(struct socket_tx* self);

/*
 * accept()
 */

int
socket_tx_accept_exec(struct socket_tx* self, int sockfd,
                      struct sockaddr* address, socklen_t* address_len,
                      bool isnoundo, int* cookie, struct picotm_error* error);

void
socket_tx_accept_apply(struct socket_tx* self, int sockfd, int cookie,
                       struct picotm_error* error);

void
socket_tx_accept_undo(struct socket_tx* self, int sockfd, int cookie,
                      struct picotm_error* error);

/*
 * bind()
 */

int
socket_tx_bind_exec(struct socket_tx* self, int sockfd,
                    const struct sockaddr* address, socklen_t addresslen,
                    bool isnoundo, int* cookie, struct picotm_error* error);

void
socket_tx_bind_apply(struct socket_tx* self, int sockfd, int cookie,
                     struct picotm_error* error);

void
socket_tx_bind_undo(struct socket_tx* self, int sockfd, int cookie,
                    struct picotm_error* error);

/*
 * connect()
 */

int
socket_tx_connect_exec(struct socket_tx* self, int sockfd,
                       const struct sockaddr* address, socklen_t addresslen,
                       bool isnoundo, int* cookie, struct picotm_error* error);

void
socket_tx_connect_apply(struct socket_tx* self, int sockfd, int cookie,
                        struct picotm_error* error);

void
socket_tx_connect_undo(struct socket_tx* self, int sockfd, int cookie,
                       struct picotm_error* error);

/*
 * fcntl()
 */

int
socket_tx_fcntl_exec(struct socket_tx* self, int fildes, int cmd,
                     union fcntl_arg* arg, bool isnoundo, int* cookie,
                     struct picotm_error* error);

void
socket_tx_fcntl_apply(struct socket_tx* self, int fildes, int cookie,
                      struct picotm_error* error);

void
socket_tx_fcntl_undo(struct socket_tx* self, int fildes, int cookie,
                     struct picotm_error* error);

/*
 * fstat()
 */

int
socket_tx_fstat_exec(struct socket_tx* self, int fildes, struct stat* buf,
                     bool isnoundo, int* cookie, struct picotm_error* error);

void
socket_tx_fstat_apply(struct socket_tx* self, int fildes, int cookie,
                      struct picotm_error* error);

void
socket_tx_fstat_undo(struct socket_tx* self, int fildes, int cookie,
                     struct picotm_error* error);

/*
 * listen()
 */

int
socket_tx_listen_exec(struct socket_tx* self, int sockfd, int backlog,
                      bool isnoundo, int* cookie, struct picotm_error* error);

void
socket_tx_listen_apply(struct socket_tx* self, int sockfd, int cookie,
                       struct picotm_error* error);

void
socket_tx_listen_undo(struct socket_tx* self, int sockfd, int cookie,
                      struct picotm_error* error);

/*
 * read()
 */

ssize_t
socket_tx_read_exec(struct socket_tx* self, int fildes, void* buf,
                    size_t nbyte, bool isnoundo,
                    enum picotm_libc_validation_mode val_mode, int* cookie,
                    struct picotm_error* error);

void
socket_tx_read_apply(struct socket_tx* self, int fildes, int cookie,
                     struct picotm_error* error);

void
socket_tx_read_undo(struct socket_tx* self, int fildes, int cookie,
                    struct picotm_error* error);

/*
 * recv()
 */

ssize_t
socket_tx_recv_exec(struct socket_tx* self, int sockfd, void* buffer,
                    size_t length, int flags, bool isnoundo, int* cookie,
                    struct picotm_error* error);

void
socket_tx_recv_apply(struct socket_tx* self, int sockfd, int cookie,
                     struct picotm_error* error);

void
socket_tx_recv_undo(struct socket_tx* self, int sockfd, int cookie,
                    struct picotm_error* error);

/*
 * send()
 */

ssize_t
socket_tx_send_exec(struct socket_tx* self, int sockfd, const void* buffer,
                    size_t length, int flags, bool isnoundo, int* cookie,
                    struct picotm_error* error);

void
socket_tx_send_apply(struct socket_tx* self, int fildes, int cookie,
                     struct picotm_error* error);

void
socket_tx_send_undo(struct socket_tx* self, int fildes, int cookie,
                    struct picotm_error* error);

/*
 * shutdown()
 */

int
socket_tx_shutdown_exec(struct socket_tx* self, int sockfd, int how,
                        bool isnoundo, int* cookie,
                        struct picotm_error* error);

void
socket_tx_shutdown_apply(struct socket_tx* self, int fildes, int cookie,
                         struct picotm_error* error);

void
socket_tx_shutdown_undo(struct socket_tx* self, int fildes, int cookie,
                        struct picotm_error* error);

/*
 * write()
 */

ssize_t
socket_tx_write_exec(struct socket_tx* self, int fildes, const void* buf,
                     size_t nbyte, bool isnoundo, int* cookie,
                     struct picotm_error* error);

void
socket_tx_write_apply(struct socket_tx* self, int fildes, int cookie,
                      struct picotm_error* error);

void
socket_tx_write_undo(struct socket_tx* self, int fildes, int cookie,
                     struct picotm_error* error);
