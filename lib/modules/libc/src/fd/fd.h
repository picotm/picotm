/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FD_H
#define FD_H

#include <stdbool.h>
#include <pthread.h>
#include "counter.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

union fcntl_arg;

#define FD_FL_LAST_BIT  (OFD_FL_LAST_BIT+1)

enum fd_state
{
    FD_ST_UNUSED = 0, /** \brief File descriptor is currenty not in use */
    FD_ST_INUSE, /** \brief File descriptor is open */
    FD_ST_CLOSING /** \brief File descriptor has been closed by transaction; other users need to abort */
};

struct fd
{
    pthread_mutex_t lock;

    struct counter ref;

    enum fd_state state;

    int ofd;

    struct counter ver;
};

/** \brief Init global file-descriptor state*/
int
fd_init(struct fd *fd);

/** \brief Uninit state */
void
fd_uninit(struct fd *fd);

/** \brief Lock global file-descriptor state */
void
fd_lock(struct fd *fd);

/** \brief Unlock global file-descriptor state */
void
fd_unlock(struct fd *fd);

/** \brief Validates that ver is not smaller than the global version */
int
fd_validate(struct fd* fd, count_type ver, struct picotm_error* error);

/** \brief Aquires a reference on the file dscriptor */
int
fd_ref(struct fd *fd, int fildes, unsigned long flags);

int
fd_ref_state(struct fd *fd, int fildes, unsigned long flags, int *ofd, count_type *version);

/** \brief Releases a reference on the file descriptor */
void
fd_unref(struct fd *fd, int fildes);

/** \brief Return non-zero value if file-descriptor is open */
int
fd_is_open_nl(const struct fd *fd);

/** \brief Get file-descriptor version number */
count_type
fd_get_version_nl(struct fd *fd);

/** \brief Get index of file-descriptor's open file description */
int
fd_get_ofd_nl(struct fd *fd);

/** \brief Set file descriptor to state close */
void
fd_close(struct fd *fd);

/** \brief Dump file-descriptor state to stderr */
void
fd_dump(const struct fd *fd);

/** Validate file descriptor against version. */
bool
fd_is_valid(struct fd* fd, count_type version);

/** Set file-descriptor flags. */
int
fd_setfd(struct fd* fd, int fildes, int arg, struct picotm_error* error);

/** Get file-descriptor flags. */
int
fd_getfd(struct fd* fd, int fildes, struct picotm_error* error);

#endif
