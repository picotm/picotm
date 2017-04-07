/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OFDTX_H
#define OFDTX_H

#include "systx/systx-libc.h"

struct sockaddr;

struct ioop;
struct seekop;
struct fcntlop;
struct com_fd_event;

enum {
    OFDTX_FL_LOCALSTATE = 0x1, /** \brief Signals local changes to state */
    OFDTX_FL_LOCALBUF   = 0x2, /** \brief Signals local changes to buffer */
    OFDTX_FL_TL_INCVER  = 0x4 /** OFD version number needs update */
};

/** \brief Holds transaction-local reads and writes for a file descriptor
 */
struct ofdtx
{
    int ofd;

    unsigned long flags;

    unsigned char *wrbuf;
    size_t         wrbuflen;
    size_t         wrbufsiz;

    struct ioop *wrtab;
    size_t       wrtablen;
    size_t       wrtabsiz;

    struct ioop *rdtab;
    size_t       rdtablen;
    size_t       rdtabsiz;

    struct seekop *seektab;
    size_t         seektablen;

    struct fcntlop *fcntltab;
    size_t          fcntltablen;

    enum systx_libc_cc_mode cc_mode; /**< \brief CC mode of domain */
    enum systx_libc_file_type type;
    off_t offset; /**< \brief Local file-pointer position */
    off_t size; /**< \brief Size of regular files */

    struct {
        struct {
            count_type     ver; /** \brief Last ofd version, modified by ofdtx */
            struct cmapss  cmapss;
            struct region *locktab; /**< \brief Table of all regions to be locked */
            size_t         locktablen;
            size_t         locktabsiz;
        } ts;
        struct {
            enum   rwstate    rwstate; /**< \brief State of the local ofd lock */
            struct rwstatemap rwstatemap; /**< \brief States of the local region locks */
            struct region    *locktab; /**< \brief Table of all locked areas */
            size_t            locktablen;
            size_t            locktabsiz;
        } tpl; /* 2pl */
    } modedata;
};

/** \brief Init transaction-local open-file-description state */
int
ofdtx_init(struct ofdtx *ofdtx);

/** \brief Uninit state */
void
ofdtx_uninit(struct ofdtx *ofdtx);

/** \brief Validate the local state */
int
ofdtx_validate(struct ofdtx *ofdtx);

/** \brief Update the data structures for concurrency control after a successful apply */
int
ofdtx_updatecc(struct ofdtx *ofdtx);

/** \brief Clear the data structures for concurrency control after a successful undo */
int
ofdtx_clearcc(struct ofdtx *ofdtx);

/** \brief Aquire a reference on the open file description */
enum error_code
ofdtx_ref(struct ofdtx *ofdtx, int ofd, int fildes, unsigned long flags, int *optcc);

/** \brief Release reference */
void
ofdtx_unref(struct ofdtx *ofdtx);

/** \brief Returns non-zero if transaction hold a reference */
int
ofdtx_holds_ref(struct ofdtx *ofdtx);

int
ofdtx_append_to_writeset(struct ofdtx *ofdtx, size_t nbyte,
                                              off_t offset,
                                              const void *buf);

int
ofdtx_append_to_readset(struct ofdtx *ofdtx, size_t nbyte,
                                             off_t offset,
                                             const void *buf);

/** \brief Prepares the open file description for commit */
int
ofdtx_pre_commit(struct ofdtx *ofdtx);

/** \brief Finishes commit for open file description */
int
ofdtx_post_commit(struct ofdtx *ofdtx);

/** \brief True if optimistic domain */
int
ofdtx_is_optimistic(const struct ofdtx *ofdtx);

/*
 * optimistic CC
 */

int
ofdtx_ts_get_state_version(struct ofdtx *ofdtx);

int
ofdtx_ts_get_region_versions(struct ofdtx *ofdtx, size_t nbyte, off_t offset);

int
ofdtx_ts_validate_state(struct ofdtx *ofdtx);

int
ofdtx_ts_validate_region(struct ofdtx *ofdtx, size_t nbyte, off_t offset);

/*
 * pessimistic CC
 */

int
ofdtx_2pl_lock_region(struct ofdtx *ofdtx, size_t nbyte,
                                           off_t offset,
                                           int write);

/** \brief Dump state to stderr */
void
ofdtx_dump(const struct ofdtx *ofdtx);


/* bind
 */

int
ofdtx_bind_exec(struct ofdtx *ofdtx, int sockfd,
                               const struct sockaddr *addr,
                                     socklen_t addrlen,
                                     int *cookie,
                                     int noundo);

int
ofdtx_bind_apply(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n);

int
ofdtx_bind_undo(struct ofdtx *ofdtx, int sockfd, int cookie);

/* connect
 */

int
ofdtx_connect_exec(struct ofdtx *ofdtx, int sockfd,
                                  const struct sockaddr *serv_addr,
                                        socklen_t addrlen,
                                        int *cookie,
                                        int noundo);

int
ofdtx_connect_apply(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n);

int
ofdtx_connect_undo(struct ofdtx *ofdtx, int sockfd, int cookie);

/* fcntl
 */

int
ofdtx_fcntl_exec(struct ofdtx *ofdtx, int fildes,
                                      int cmd, union com_fd_fcntl_arg *arg,
                                      int *cookie, int noundo);

int
ofdtx_fcntl_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n);

int
ofdtx_fcntl_undo(struct ofdtx *ofdtx, int fildes, int cookie);

/* fsync
 */

int
ofdtx_fsync_exec(struct ofdtx *ofdtx, int fildes, int noundo, int *cookie);

int
ofdtx_fsync_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n);

int
ofdtx_fsync_undo(struct ofdtx *ofdtx, int fildes, int cookie);

/* listen
 */

int
ofdtx_listen_exec(struct ofdtx *ofdtx, int sockfd,
                                       int backlog,
                                       int *cookie,
                                       int noundo);

int
ofdtx_listen_apply(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n);

int
ofdtx_listen_undo(struct ofdtx *ofdtx, int sockfd, int cookie);

/* lseek
 */

off_t
ofdtx_lseek_exec(struct ofdtx *ofdtx, int fildes, off_t offset, int whence,
                                      int *cookie, int noundo);

int
ofdtx_lseek_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n);

int
ofdtx_lseek_undo(struct ofdtx *ofdtx, int fildes, int cookie);

/* pread
 */

ssize_t
ofdtx_pread_exec(struct ofdtx *ofdtx, int fildes, void *buf, size_t nbyte, off_t off,
                                      int *cookie, int noundo,
                                      enum validation_mode valmode);

ssize_t
ofdtx_pread_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n);

int
ofdtx_pread_undo(struct ofdtx *ofdtx, int fildes, int cookie);

/* pwrite
 */

ssize_t
ofdtx_pwrite_exec(struct ofdtx *ofdtx, int fildes, const void *buf, size_t nbyte, off_t off,
                                       int *cookie, int noundo);

ssize_t
ofdtx_pwrite_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n);

int
ofdtx_pwrite_undo(struct ofdtx *ofdtx, int fildes, int cookie);

/* read
 */

ssize_t
ofdtx_read_exec(struct ofdtx *ofdtx, int fildes, void *buf, size_t nbyte,
                                     int *cookie, int noundo,
                                     enum validation_mode valmode);

ssize_t
ofdtx_read_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n);

off_t
ofdtx_read_undo(struct ofdtx *ofdtx, int fildes, int cookie);

/* recv
 */

ssize_t
ofdtx_recv_exec(struct ofdtx *ofdtx, int sockfd,
                                     void *buffer,
                                     size_t length,
                                     int flags,
                                     int *cookie, int noundo);

int
ofdtx_recv_apply(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n);

int
ofdtx_recv_undo(struct ofdtx *ofdtx, int sockfd, int cookie);

/* send
 */

ssize_t
ofdtx_send_exec(struct ofdtx *ofdtx, int sockfd,
                               const void *buffer,
                                     size_t length,
                                     int flags,
                                     int *cookie, int noundo);

int
ofdtx_send_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n);

int
ofdtx_send_undo(struct ofdtx *ofdtx, int fildes, int cookie);

/* shutdown
 */

int
ofdtx_shutdown_exec(struct ofdtx *ofdtx, int sockfd,
                                         int how,
                                         int *cookie,
                                         int noundo);

int
ofdtx_shutdown_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n);

int
ofdtx_shutdown_undo(struct ofdtx *ofdtx, int fildes, int cookie);

/* write
 */

ssize_t
ofdtx_write_exec(struct ofdtx *ofdtx, int fildes, const void *buf, size_t nbyte,
                                      int *cookie, int noundo);

ssize_t
ofdtx_write_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n);

int
ofdtx_write_undo(struct ofdtx *ofdtx, int fildes, int cookie);

#endif

