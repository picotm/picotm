/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FDTX_H
#define FDTX_H

#include "counter.h"
#include "picotm/picotm-libc.h"

#define FDTX_FL_LOCALSTATE 1L /** \brief Signals local state changes */

struct fcntlop;

union com_fd_fcntl_arg;

/** \brief Holds transaction-local reads and writes for a file descriptor
 */
struct fdtx
{
    int fildes;
    int ofd;
    enum picotm_libc_cc_mode cc_mode;

	unsigned long flags;

    struct fcntlop *fcntltab;
    size_t          fcntltablen;

    count_type fdver; /* Last fd version, modified by fdtx */
};

/** \brief Init transaction-local file-descriptor state */
int
fdtx_init(struct fdtx *fdtx);

/** \brief Uninit state */
void
fdtx_uninit(struct fdtx *fdtx);

/** \brief Validate transaction-local state */
int
fdtx_validate(struct fdtx *fdtx);

int
fdtx_updatecc(struct fdtx *fdtx);

int
fdtx_clearcc(struct fdtx *fdtx);

/** \brief Aquire a reference on file-descriptor state or validate */
enum error_code
fdtx_ref_or_validate(struct fdtx *fdtx, int fildes, unsigned long flags);

/** \brief Aquire a reference on file-descriptor state */
enum error_code
fdtx_ref(struct fdtx *fdtx, int fildes, unsigned long flags);

/** \brief Release reference */
void
fdtx_unref(struct fdtx *fdtx);

/** \brief Returns non-zero if transaction holds a reference on file descriptor */
int
fdtx_holds_ref(const struct fdtx *fdtx);

int
fdtx_pre_commit(struct fdtx *fdtx);

int
fdtx_post_commit(struct fdtx *fdtx);

/** \brief Set file descriptor to CLOSING */
void
fdtx_signal_close(struct fdtx *fdtx);

/** \brief Dump file descriptor state */
void
fdtx_dump(const struct fdtx *fdtx);

/* close
 */

int
fdtx_close_exec(struct fdtx *fdtx, int fildes, int *cookie, int noundo);

int
fdtx_close_apply(struct fdtx *fdtx, int fildes, int cookie);

int
fdtx_close_undo(struct fdtx *fdtx, int fildes, int cookie);

/* fcntl
 */

int
fdtx_fcntl_exec(struct fdtx *fdtx, int cmd,
                                   union com_fd_fcntl_arg *arg,
                                   int *cookie,
                                   int noundo);

int
fdtx_fcntl_apply(struct fdtx *fdtx, int cookie);

int
fdtx_fcntl_undo(struct fdtx *fdtx, int cookie);

#endif

