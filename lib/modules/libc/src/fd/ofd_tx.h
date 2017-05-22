/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include "counter.h"
#include "fcntlop.h"
#include "picotm/picotm-libc.h"
#include "rwlock.h"
#include "rwstatemap.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct fcntlop;
struct ioop;
struct picotm_error;
struct seekop;
struct sockaddr;
struct fd_event;

enum {
    /** Signals local changes to state */
    OFDTX_FL_LOCALSTATE = 0x1,
    /** Signals local changes to buffer */
    OFDTX_FL_LOCALBUF   = 0x2,
    /** OFD version number needs update */
    OFDTX_FL_TL_INCVER  = 0x4
};

/**
 * Holds transaction-local reads and writes for an open file description
 */
struct ofd_tx
{
    int ofd;

    unsigned long flags;

    unsigned char* wrbuf;
    size_t         wrbuflen;
    size_t         wrbufsiz;

    struct ioop* wrtab;
    size_t       wrtablen;
    size_t       wrtabsiz;

    struct ioop* rdtab;
    size_t       rdtablen;
    size_t       rdtabsiz;

    struct seekop* seektab;
    size_t         seektablen;

    struct fcntlop* fcntltab;
    size_t          fcntltablen;

    /** CC mode of domain */
    enum picotm_libc_cc_mode cc_mode;

    enum picotm_libc_file_type type;

    /** Local file-pointer position */
    off_t offset;

    /** Size of regular files */
    off_t size;

    struct {
        struct {
            /** State of the local ofd lock */
            enum   rwstate    rwstate;
            /** States of the local region locks */
            struct rwstatemap rwstatemap;
            /** Table of all locked areas */
            struct region*    locktab;
            size_t            locktablen;
            size_t            locktabsiz;
        } tpl; /* 2pl */
    } modedata;
};

/**
 * Init transaction-local open-file-description state
 */
int
ofd_tx_init(struct ofd_tx* self);

/**
 * Uninit state
 */
void
ofd_tx_uninit(struct ofd_tx* self);

/**
 * Validate the local state
 */
int
ofd_tx_validate(struct ofd_tx* self, struct picotm_error* error);

/**
 * Updates the data structures for concurrency control after a successful apply
 */
int
ofd_tx_update_cc(struct ofd_tx* self, struct picotm_error* error);

/**
 * Clears the data structures for concurrency control after a successful undo
 */
int
ofd_tx_clear_cc(struct ofd_tx* self, struct picotm_error* error);

/**
 * Acquire a reference on the open file description
 */
enum error_code
ofd_tx_ref(struct ofd_tx* self, int ofd, int fildes, unsigned long flags);

/**
 * Release reference
 */
void
ofd_tx_unref(struct ofd_tx* self);

/**
 * Returns non-zero if transactions hold a reference
 */
int
ofd_tx_holds_ref(struct ofd_tx* self);

int
ofd_tx_append_to_writeset(struct ofd_tx* self, size_t nbyte, off_t offset,
                          const void* buf);

int
ofd_tx_append_to_readset(struct ofd_tx* self, size_t nbyte, off_t offset,
                         const void* buf);

/**
 * Prepares the open file description for commit
 */
int
ofd_tx_pre_commit(struct ofd_tx* self);

/**
 * Finishes commit for open file description
 */
int
ofd_tx_post_commit(struct ofd_tx* self);

/*
 * pessimistic CC
 */

int
ofd_tx_2pl_lock_region(struct ofd_tx* self, size_t nbyte, off_t offset,
                       int write);

/**
 * Dump state to stderr
 */
void
ofd_tx_dump(const struct ofd_tx* self);

/*
 * bind()
 */

int
ofd_tx_bind_exec(struct ofd_tx* self, int sockfd, const struct sockaddr *addr,
                 socklen_t addrlen, int* cookie, int noundo);

int
ofd_tx_bind_apply(struct ofd_tx* self, int sockfd,
                  const struct fd_event* event, size_t n,
                  struct picotm_error* error);

int
ofd_tx_bind_undo(struct ofd_tx* self, int sockfd, int cookie,
                 struct picotm_error* error);

/*
 * connect()
 */

int
ofd_tx_connect_exec(struct ofd_tx* self, int sockfd,
                    const struct sockaddr* serv_addr, socklen_t addrlen,
                    int* cookie, int noundo);

int
ofd_tx_connect_apply(struct ofd_tx* self, int sockfd,
                     const struct fd_event* event, size_t n,
                     struct picotm_error* error);

int
ofd_tx_connect_undo(struct ofd_tx* self, int sockfd, int cookie,
                    struct picotm_error* error);

/*
 * fcntl()
 */

int
ofd_tx_fcntl_exec(struct ofd_tx* self, int fildes, int cmd,
                  union fcntl_arg* arg, int* cookie, int noundo);

int
ofd_tx_fcntl_apply(struct ofd_tx* self, int fildes,
                   const struct fd_event* event, size_t n,
                   struct picotm_error* error);

int
ofd_tx_fcntl_undo(struct ofd_tx* self, int fildes, int cookie,
                  struct picotm_error* error);

/*
 * fsync()
 */

int
ofd_tx_fsync_exec(struct ofd_tx* self, int fildes, int noundo, int* cookie);

int
ofd_tx_fsync_apply(struct ofd_tx* self, int fildes,
                   const struct fd_event* event, size_t n,
                   struct picotm_error* error);

int
ofd_tx_fsync_undo(struct ofd_tx* self, int fildes, int cookie,
                  struct picotm_error* error);

/*
 * listen()
 */

int
ofd_tx_listen_exec(struct ofd_tx* self, int sockfd, int backlog, int* cookie,
                   int noundo);

int
ofd_tx_listen_apply(struct ofd_tx* self, int sockfd,
                    const struct fd_event* event, size_t n,
                    struct picotm_error* error);

int
ofd_tx_listen_undo(struct ofd_tx* self, int sockfd, int cookie,
                   struct picotm_error* error);

/*
 * lseek()
 */

off_t
ofd_tx_lseek_exec(struct ofd_tx* self, int fildes, off_t offset, int whence,
                  int* cookie, int noundo);

int
ofd_tx_lseek_apply(struct ofd_tx* self, int fildes,
                   const struct fd_event* event, size_t n,
                   struct picotm_error* error);

int
ofd_tx_lseek_undo(struct ofd_tx* self, int fildes, int cookie,
                  struct picotm_error* error);

/*
 * pread()
 */

ssize_t
ofd_tx_pread_exec(struct ofd_tx* self, int fildes, void* buf, size_t nbyte,
                  off_t off, int* cookie, int noundo,
                  enum picotm_libc_validation_mode val_mode);

ssize_t
ofd_tx_pread_apply(struct ofd_tx* self, int fildes,
                   const struct fd_event* event, size_t n,
                   struct picotm_error* error);

int
ofd_tx_pread_undo(struct ofd_tx* self, int fildes, int cookie,
                  struct picotm_error* error);

/*
 * pwrite()
 */

ssize_t
ofd_tx_pwrite_exec(struct ofd_tx* self, int fildes, const void* buf,
                   size_t nbyte, off_t off, int* cookie, int noundo);

ssize_t
ofd_tx_pwrite_apply(struct ofd_tx* self, int fildes,
                    const struct fd_event* event, size_t n,
                    struct picotm_error* error);

int
ofd_tx_pwrite_undo(struct ofd_tx* self, int fildes, int cookie,
                   struct picotm_error* error);

/*
 * read()
 */

ssize_t
ofd_tx_read_exec(struct ofd_tx* self, int fildes, void *buf, size_t nbyte,
                 int* cookie, int noundo,
                 enum picotm_libc_validation_mode val_mode);

ssize_t
ofd_tx_read_apply(struct ofd_tx* self, int fildes,
                  const struct fd_event* event, size_t n,
                  struct picotm_error* error);

off_t
ofd_tx_read_undo(struct ofd_tx* self, int fildes, int cookie,
                 struct picotm_error* error);

/*
 * recv()
 */

ssize_t
ofd_tx_recv_exec(struct ofd_tx* self, int sockfd, void* buffer, size_t length,
                 int flags, int* cookie, int noundo);

int
ofd_tx_recv_apply(struct ofd_tx* self, int sockfd,
                  const struct fd_event* event, size_t n,
                  struct picotm_error* error);

int
ofd_tx_recv_undo(struct ofd_tx* self, int sockfd, int cookie,
                 struct picotm_error* error);

/*
 * send()
 */

ssize_t
ofd_tx_send_exec(struct ofd_tx* self, int sockfd, const void* buffer,
                 size_t length, int flags, int* cookie, int noundo);

int
ofd_tx_send_apply(struct ofd_tx* self, int fildes,
                  const struct fd_event* event, size_t n,
                  struct picotm_error* error);

int
ofd_tx_send_undo(struct ofd_tx* self, int fildes, int cookie,
                 struct picotm_error* error);

/*
 * shutdown()
 */

int
ofd_tx_shutdown_exec(struct ofd_tx* self, int sockfd, int how, int* cookie,
                     int noundo);

int
ofd_tx_shutdown_apply(struct ofd_tx* self, int fildes,
                      const struct fd_event* event, size_t n,
                      struct picotm_error* error);

int
ofd_tx_shutdown_undo(struct ofd_tx* self, int fildes, int cookie,
                     struct picotm_error* error);

/*
 * write()
 */

ssize_t
ofd_tx_write_exec(struct ofd_tx* self, int fildes, const void* buf,
                  size_t nbyte, int* cookie, int noundo);

ssize_t
ofd_tx_write_apply(struct ofd_tx* self, int fildes,
                   const struct fd_event* event, size_t n,
                   struct picotm_error* error);

int
ofd_tx_write_undo(struct ofd_tx* self, int fildes, int cookie,
                  struct picotm_error* error);
