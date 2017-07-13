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
#include "ofd_tx.h"
#include "picotm/picotm-libc.h"
#include "regfile.h"
#include "rwcountermap.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

union fcntl_arg;

/**
 * Holds transaction-local reads and writes for a regular file.
 */
struct regfile_tx {

    struct picotm_ref16 ref;

    SLIST_ENTRY(regfile_tx) active_list;

    struct ofd_tx base;

    struct regfile* regfile;

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

    struct fchmodop* fchmodtab;
    size_t           fchmodtablen;

    struct fcntlop* fcntltab;
    size_t          fcntltablen;

    /** CC mode of domain */
    enum picotm_libc_cc_mode cc_mode;

    /** Local file-pointer position */
    off_t offset;

    /** Size of regular files */
    off_t size;

    /** State of the local reader/writer locks. */
    struct picotm_rwstate rwstate[NUMBER_OF_REGFILE_FIELDS];

    /** States of the local region locks */
    struct rwcountermap rwcountermap;

    /** Table of all locked areas */
    struct region*    locktab;
    size_t            locktablen;
    size_t            locktabsiz;
};

struct regfile_tx*
regfile_tx_of_ofd_tx(struct ofd_tx* ofd_tx);

/**
 * Init transaction-local open-file-description state
 */
void
regfile_tx_init(struct regfile_tx* self);

/**
 * Uninit state
 */
void
regfile_tx_uninit(struct regfile_tx* self);

/**
 * Validate the local state
 */
void
regfile_tx_validate(struct regfile_tx* self, struct picotm_error* error);

/**
 * Updates the data structures for concurrency control after a successful apply
 */
void
regfile_tx_update_cc(struct regfile_tx* self, struct picotm_error* error);

/**
 * Clears the data structures for concurrency control after a successful undo
 */
void
regfile_tx_clear_cc(struct regfile_tx* self, struct picotm_error* error);

/**
 * Acquire a reference on the open file description
 */
void
regfile_tx_ref_or_set_up(struct regfile_tx* self, struct regfile* regfile,
                         int fildes, unsigned long flags,
                         struct picotm_error* error);

/**
 * Acquire a reference on the open file description
 */
void
regfile_tx_ref(struct regfile_tx* self);

/**
 * Release reference
 */
void
regfile_tx_unref(struct regfile_tx* self);

/**
 * Returns true if transactions hold a reference
 */
bool
regfile_tx_holds_ref(struct regfile_tx* self);

int
regfile_tx_append_to_writeset(struct regfile_tx* self, size_t nbyte,
                              off_t offset, const void* buf,
                              struct picotm_error* error);

int
regfile_tx_append_to_readset(struct regfile_tx* self, size_t nbyte,
                             off_t offset, const void* buf,
                             struct picotm_error* error);

/**
 * Prepares the open file description for commit
 */
void
regfile_tx_lock(struct regfile_tx* self);

/**
 * Finishes commit for open file description
 */
void
regfile_tx_unlock(struct regfile_tx* self);

/*
 * fchmod()
 */

int
regfile_tx_fchmod_exec(struct regfile_tx* self, int fildes, mode_t mode,
                       bool isnoundo, int* cookie,
                       struct picotm_error* error);

void
regfile_tx_fchmod_apply(struct regfile_tx* self, int fildes, int cookie,
                        struct picotm_error* error);

void
regfile_tx_fchmod_undo(struct regfile_tx* self, int fildes, int cookie,
                       struct picotm_error* error);

/*
 * fcntl()
 */

int
regfile_tx_fcntl_exec(struct regfile_tx* self, int fildes, int cmd,
                      union fcntl_arg* arg, bool isnoundo, int* cookie,
                      struct picotm_error* error);

void
regfile_tx_fcntl_apply(struct regfile_tx* self, int fildes, int cookie,
                       struct picotm_error* error);

void
regfile_tx_fcntl_undo(struct regfile_tx* self, int fildes, int cookie,
                      struct picotm_error* error);

/*
 * fsync()
 */

int
regfile_tx_fsync_exec(struct regfile_tx* self, int fildes, bool isnoundo,
                      int* cookie, struct picotm_error* error);

void
regfile_tx_fsync_apply(struct regfile_tx* self, int fildes, int cookie,
                       struct picotm_error* error);

void
regfile_tx_fsync_undo(struct regfile_tx* self, int fildes, int cookie,
                      struct picotm_error* error);

/*
 * lseek()
 */

off_t
regfile_tx_lseek_exec(struct regfile_tx* self, int fildes, off_t offset,
                      int whence, bool isnoundo, int* cookie,
                      struct picotm_error* error);

void
regfile_tx_lseek_apply(struct regfile_tx* self, int fildes, int cookie,
                       struct picotm_error* error);

void
regfile_tx_lseek_undo(struct regfile_tx* self, int fildes, int cookie,
                      struct picotm_error* error);

/*
 * pread()
 */

ssize_t
regfile_tx_pread_exec(struct regfile_tx* self, int fildes, void* buf,
                      size_t nbyte, off_t off, bool isnoundo,
                      enum picotm_libc_validation_mode val_mode, int* cookie,
                      struct picotm_error* error);

void
regfile_tx_pread_apply(struct regfile_tx* self, int fildes, int cookie,
                       struct picotm_error* error);

void
regfile_tx_pread_undo(struct regfile_tx* self, int fildes, int cookie,
                      struct picotm_error* error);

/*
 * pwrite()
 */

ssize_t
regfile_tx_pwrite_exec(struct regfile_tx* self, int fildes, const void* buf,
                       size_t nbyte, off_t off, bool isnoundo, int* cookie,
                       struct picotm_error* error);

void
regfile_tx_pwrite_apply(struct regfile_tx* self, int fildes, int cookie,
                        struct picotm_error* error);

void
regfile_tx_pwrite_undo(struct regfile_tx* self, int fildes, int cookie,
                       struct picotm_error* error);

/*
 * read()
 */

ssize_t
regfile_tx_read_exec(struct regfile_tx* self, int fildes, void* buf,
                     size_t nbyte, bool isnoundo,
                     enum picotm_libc_validation_mode val_mode, int* cookie,
                     struct picotm_error* error);

void
regfile_tx_read_apply(struct regfile_tx* self, int fildes, int cookie,
                      struct picotm_error* error);

void
regfile_tx_read_undo(struct regfile_tx* self, int fildes, int cookie,
                     struct picotm_error* error);

/*
 * write()
 */

ssize_t
regfile_tx_write_exec(struct regfile_tx* self, int fildes, const void* buf,
                      size_t nbyte, bool isnoundo, int* cookie,
                      struct picotm_error* error);

void
regfile_tx_write_apply(struct regfile_tx* self, int fildes, int cookie,
                       struct picotm_error* error);

void
regfile_tx_write_undo(struct regfile_tx* self, int fildes, int cookie,
                      struct picotm_error* error);
