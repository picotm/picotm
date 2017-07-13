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
#include <sys/types.h>
#include "dir.h"
#include "ofd_tx.h"
#include "picotm/picotm-libc.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct fchmodop;
struct picotm_error;
struct stat;

union fcntl_arg;

/**
 * Holds transaction-local state for a directory.
 */
struct dir_tx {

    struct picotm_ref16 ref;

    SLIST_ENTRY(dir_tx) active_list;

    struct ofd_tx base;

    struct dir* dir;

    unsigned long flags;

    struct fchmodop* fchmodtab;
    size_t           fchmodtablen;

    struct fcntlop* fcntltab;
    size_t          fcntltablen;

    /** Concurrency-control mode */
    enum picotm_libc_cc_mode cc_mode;

    /** State of the local locks */
    struct picotm_rwstate rwstate[NUMBER_OF_DIR_FIELDS];
};

struct dir_tx*
dir_tx_of_ofd_tx(struct ofd_tx* ofd_tx);

/**
 * Initialize transaction-local directory.
 * \param   self    The instance of `struct dir` to initialize.
 */
void
dir_tx_init(struct dir_tx* self);

/**
 * Uninitialize transaction-local directory.
 * \param   self    The instance of `struct dir` to initialize.
 */
void
dir_tx_uninit(struct dir_tx* self);

/**
 * Validate the local state
 */
void
dir_tx_validate(struct dir_tx* self, struct picotm_error* error);

/**
 * Updates the data structures for concurrency control after a successful apply
 */
void
dir_tx_update_cc(struct dir_tx* self, struct picotm_error* error);

/**
 * Clears the data structures for concurrency control after a successful undo
 */
void
dir_tx_clear_cc(struct dir_tx* self, struct picotm_error* error);

/**
 * Acquire a reference on the open file description
 */
void
dir_tx_ref_or_set_up(struct dir_tx* self, struct dir* dir, int fildes,
                        unsigned long flags, struct picotm_error* error);

/**
 * Acquire a reference on the open file description
 */
void
dir_tx_ref(struct dir_tx* self);

/**
 * Release reference
 */
void
dir_tx_unref(struct dir_tx* self);

/**
 * Returns true if transactions hold a reference
 */
bool
dir_tx_holds_ref(struct dir_tx* self);

/**
 * Prepares the open file description for commit
 */
void
dir_tx_lock(struct dir_tx* self);

/**
 * Finishes commit for open file description
 */
void
dir_tx_unlock(struct dir_tx* self);

/*
 * fchmod()
 */

int
dir_tx_fchmod_exec(struct dir_tx* self, int fildes, mode_t mode,
                   bool isnoundo, int* cookie,
                   struct picotm_error* error);

void
dir_tx_fchmod_apply(struct dir_tx* self, int fildes, int cookie,
                    struct picotm_error* error);

void
dir_tx_fchmod_undo(struct dir_tx* self, int fildes, int cookie,
                   struct picotm_error* error);

/*
 * fcntl()
 */

int
dir_tx_fcntl_exec(struct dir_tx* self, int fildes, int cmd,
                  union fcntl_arg* arg, bool isnoundo, int* cookie,
                  struct picotm_error* error);

void
dir_tx_fcntl_apply(struct dir_tx* self, int fildes, int cookie,
                   struct picotm_error* error);

void
dir_tx_fcntl_undo(struct dir_tx* self, int fildes, int cookie,
                  struct picotm_error* error);

/*
 * fstat()
 */

int
dir_tx_fstat_exec(struct dir_tx* self, int fildes, struct stat* buf,
                  bool isnoundo, int* cookie, struct picotm_error* error);

void
dir_tx_fstat_apply(struct dir_tx* self, int fildes, int cookie,
                   struct picotm_error* error);

void
dir_tx_fstat_undo(struct dir_tx* self, int fildes, int cookie,
                  struct picotm_error* error);

/*
 * fsync()
 */

int
dir_tx_fsync_exec(struct dir_tx* self, int fildes, bool isnoundo,
                  int* cookie, struct picotm_error* error);

void
dir_tx_fsync_apply(struct dir_tx* self, int fildes, int cookie,
                   struct picotm_error* error);

void
dir_tx_fsync_undo(struct dir_tx* self, int fildes, int cookie,
                  struct picotm_error* error);
