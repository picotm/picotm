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

#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-ref.h"
#include "picotm/picotm-lib-rwstate.h"
#include "picotm/picotm-lib-slist.h"
#include "picotm/picotm-libc.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "fd.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

#define FDTX_FL_LOCALSTATE 1L /** \brief Signals local state changes */

struct fcntlop;
struct file_tx;
struct picotm_error;
struct stat;

union fcntl_arg;

/**
 * Holds transaction-local reads and writes for a file descriptor
 */
struct fd_tx {

    struct picotm_ref16 ref;

    struct picotm_slist active_list;

    struct fd* fd;

    struct file_tx* file_tx;
    enum picotm_libc_cc_mode cc_mode;

    /** Reader/writer state for file descriptor */
    struct picotm_rwstate   rwstate[NUMBER_OF_FD_FIELDS];

    struct fcntlop* fcntltab;
    size_t          fcntltablen;
};

static inline struct fd_tx*
fd_tx_of_slist(struct picotm_slist* item)
{
    return picotm_containerof(item, struct fd_tx, active_list);
}

/**
 * Init transaction-local file-descriptor state
 */
void
fd_tx_init(struct fd_tx* self);

/**
 * Uninit state
 */
void
fd_tx_uninit(struct fd_tx* self);

/**
 * Validate transaction-local state
 */
void
fd_tx_validate(struct fd_tx* self, struct picotm_error* error);

/**
 * Prepare transaction-local state for commit
 */
void
fd_tx_prepare_commit(struct fd_tx* self, struct picotm_error* error);

void
fd_tx_finish(struct fd_tx* self);

/**
 * Aquire a reference on file-descriptor state
 */
void
fd_tx_ref_or_set_up(struct fd_tx* self, struct fd* fd,
                    struct file_tx* file_tx,
                    struct picotm_error* error);

/**
 * Aquire a reference
 */
void
fd_tx_ref(struct fd_tx* self);

/**
 * Release reference
 */
void
fd_tx_unref(struct fd_tx* self);

/**
 * Returns true if transaction holds a reference on file descriptor
 */
bool
fd_tx_holds_ref(const struct fd_tx* self);

/**
 * Set file descriptor to CLOSING
 */
void
fd_tx_signal_close(struct fd_tx* self);

/*
 * accept()
 */

int
fd_tx_accept_exec(struct fd_tx* self,
                  int sockfd, struct sockaddr* address,
                  socklen_t* address_len,
                  bool isnoundo, int* cookie, struct picotm_error* error);

void
fd_tx_accept_apply(struct fd_tx* self, int sockfd, int cookie,
                   struct picotm_error* error);

void
fd_tx_accept_undo(struct fd_tx* self, int sockfd, int cookie,
                  struct picotm_error* error);

/*
 * bind()
 */

int
fd_tx_bind_exec(struct fd_tx* self,
                int sockfd, const struct sockaddr *addr,
                socklen_t addrlen,
                bool isnoundo, int* cookie, struct picotm_error* error);

void
fd_tx_bind_apply(struct fd_tx* self, int sockfd, int cookie,
                 struct picotm_error* error);

void
fd_tx_bind_undo(struct fd_tx* self, int sockfd, int cookie,
                struct picotm_error* error);

/*
 * close()
 */

int
fd_tx_close_exec(struct fd_tx* self,
                 int fildes,
                 bool isnoundo, int* cookie, struct picotm_error* error);

void
fd_tx_close_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error);

void
fd_tx_close_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error);

/*
 * connect()
 */

int
fd_tx_connect_exec(struct fd_tx* self,
                   int sockfd, const struct sockaddr* serv_addr,
                   socklen_t addrlen,
                   bool isnoundo, int* cookie, struct picotm_error* error);

void
fd_tx_connect_apply(struct fd_tx* self, int sockfd, int cookie,
                    struct picotm_error* error);

void
fd_tx_connect_undo(struct fd_tx* self, int sockfd, int cookie,
                   struct picotm_error* error);

/*
 * dup()
 */

int
fd_tx_dup_exec(struct fd_tx* self,
               int fildes, bool cloexec,
               bool isnoundo, int* cookie, struct picotm_error* error);

void
fd_tx_dup_apply(struct fd_tx* self, int fildes, int cookie,
                struct picotm_error* error);

void
fd_tx_dup_undo(struct fd_tx* self, int fildes, int cookie,
               struct picotm_error* error);

/*
 * fchmod()
 */

int
fd_tx_fchmod_exec(struct fd_tx* self,
                  int fildes, mode_t mode,
                  bool isnoundo, int* cookie, struct picotm_error* error);

void
fd_tx_fchmod_apply(struct fd_tx* self, int fildes, int cookie,
                   struct picotm_error* error);

void
fd_tx_fchmod_undo(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error);

/*
 * fcntl()
 */

int
fd_tx_fcntl_exec(struct fd_tx* self,
                 int fildes, int cmd, union fcntl_arg* arg,
                 bool isnoundo, int* cookie, struct picotm_error* error);

void
fd_tx_fcntl_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error);

void
fd_tx_fcntl_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error);

/*
 * fstat()
 */

int
fd_tx_fstat_exec(struct fd_tx* self,
                 int fildes, struct stat* buf,
                 bool isnoundo, int* cookie, struct picotm_error* error);

void
fd_tx_fstat_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error);

void
fd_tx_fstat_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error);

/*
 * fsync()
 */

int
fd_tx_fsync_exec(struct fd_tx* self,
                 int fildes,
                 bool isnoundo, int* cookie, struct picotm_error* error);

void
fd_tx_fsync_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error);

void
fd_tx_fsync_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error);

/*
 * listen()
 */

int
fd_tx_listen_exec(struct fd_tx* self,
                  int sockfd, int backlog,
                  bool isnoundo, int* cookie, struct picotm_error* error);

void
fd_tx_listen_apply(struct fd_tx* self, int sockfd, int cookie,
                   struct picotm_error* error);

void
fd_tx_listen_undo(struct fd_tx* self, int sockfd, int cookie,
                  struct picotm_error* error);

/*
 * lseek()
 */

off_t
fd_tx_lseek_exec(struct fd_tx* self,
                 int fildes, off_t offset, int whence,
                 bool isnoundo, int* cookie, struct picotm_error* error);

void
fd_tx_lseek_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error);

void
fd_tx_lseek_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error);

/*
 * pread()
 */

ssize_t
fd_tx_pread_exec(struct fd_tx* self,
                 int fildes, void* buf, size_t nbyte, off_t off,
                 bool isnoundo, int* cookie, struct picotm_error* error);

void
fd_tx_pread_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error);

void
fd_tx_pread_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error);

/*
 * pwrite()
 */

ssize_t
fd_tx_pwrite_exec(struct fd_tx* self,
                  int fildes, const void* buf, size_t nbyte, off_t off,
                  bool isnoundo, int* cookie, struct picotm_error* error);

void
fd_tx_pwrite_apply(struct fd_tx* self, int fildes, int cookie,
                   struct picotm_error* error);

void
fd_tx_pwrite_undo(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error);

/*
 * read()
 */

ssize_t
fd_tx_read_exec(struct fd_tx* self,
                int fildes, void* buf, size_t nbyte,
                bool isnoundo, int* cookie, struct picotm_error* error);

void
fd_tx_read_apply(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error);

void
fd_tx_read_undo(struct fd_tx* self, int fildes, int cookie,
                struct picotm_error* error);

/*
 * recv()
 */

ssize_t
fd_tx_recv_exec(struct fd_tx* self,
                int sockfd, void* buffer, size_t length, int flags,
                bool isnoundo, int* cookie, struct picotm_error* error);

void
fd_tx_recv_apply(struct fd_tx* self, int sockfd, int cookie,
                 struct picotm_error* error);

void
fd_tx_recv_undo(struct fd_tx* self, int sockfd, int cookie,
                struct picotm_error* error);

/*
 * send()
 */

ssize_t
fd_tx_send_exec(struct fd_tx* self,
                int sockfd, const void* buffer, size_t length, int flags,
                bool isnoundo, int* cookie, struct picotm_error* error);

void
fd_tx_send_apply(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error);

void
fd_tx_send_undo(struct fd_tx* self, int fildes, int cookie,
                struct picotm_error* error);

/*
 * shutdown()
 */

int
fd_tx_shutdown_exec(struct fd_tx* self,
                    int sockfd, int how,
                    bool isnoundo, int* cookie, struct picotm_error* error);

void
fd_tx_shutdown_apply(struct fd_tx* self, int fildes, int cookie,
                     struct picotm_error* error);

void
fd_tx_shutdown_undo(struct fd_tx* self, int fildes, int cookie,
                    struct picotm_error* error);

/*
 * write()
 */

ssize_t
fd_tx_write_exec(struct fd_tx* self,
                 int fildes, const void* buf, size_t nbyte,
                 bool isnoundo, int* cookie, struct picotm_error* error);

void
fd_tx_write_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error);

void
fd_tx_write_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error);
