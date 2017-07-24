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
#include "fifo.h"
#include "file_tx.h"
#include "picotm/picotm-libc.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct fifo;
struct picotm_error;
struct stat;

union fcntl_arg;

/**
 * Holds transaction-local state for an FIFO.
 */
struct fifo_tx {

    struct picotm_ref16 ref;

    SLIST_ENTRY(fifo_tx) active_list;

    struct file_tx base;

    struct fifo* fifo;

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

    /** State of the local reader/writer locks */
    struct picotm_rwstate rwstate[NUMBER_OF_FIFO_FIELDS];
};

struct fifo_tx*
fifo_tx_of_file_tx(struct file_tx* file_tx);

/**
 * Init transaction-local open-file-description state
 */
void
fifo_tx_init(struct fifo_tx* self);

/**
 * Uninit state
 */
void
fifo_tx_uninit(struct fifo_tx* self);

/**
 * Validate the local state
 */
void
fifo_tx_validate(struct fifo_tx* self, struct picotm_error* error);

/**
 * Updates the data structures for concurrency control after a successful apply
 */
void
fifo_tx_update_cc(struct fifo_tx* self, struct picotm_error* error);

/**
 * Clears the data structures for concurrency control after a successful undo
 */
void
fifo_tx_clear_cc(struct fifo_tx* self, struct picotm_error* error);

/**
 * Acquire a reference on the open file description
 */
void
fifo_tx_ref_or_set_up(struct fifo_tx* self, struct fifo* fifo, int fildes,
                      struct picotm_error* error);

/**
 * Acquire a reference on the open file description
 */
void
fifo_tx_ref(struct fifo_tx* self);

/**
 * Release reference
 */
void
fifo_tx_unref(struct fifo_tx* self);

/**
 * Returns true if transactions hold a reference
 */
bool
fifo_tx_holds_ref(struct fifo_tx* self);

int
fifo_tx_append_to_writeset(struct fifo_tx* self, size_t nbyte, off_t offset,
                           const void* buf, struct picotm_error* error);

int
fifo_tx_append_to_readset(struct fifo_tx* self, size_t nbyte, off_t offset,
                          const void* buf, struct picotm_error* error);

/**
 * Prepares the open file description for commit
 */
void
fifo_tx_lock(struct fifo_tx* self);

/**
 * Finishes commit for open file description
 */
void
fifo_tx_unlock(struct fifo_tx* self);

/*
 * fcntl()
 */

int
fifo_tx_fcntl_exec(struct fifo_tx* self, int fildes, int cmd,
                   union fcntl_arg* arg, bool isnoundo, int* cookie,
                   struct picotm_error* error);

void
fifo_tx_fcntl_apply(struct fifo_tx* self, int fildes, int cookie,
                    struct picotm_error* error);

void
fifo_tx_fcntl_undo(struct fifo_tx* self, int fildes, int cookie,
                   struct picotm_error* error);

/*
 * fstat()
 */

int
fifo_tx_fstat_exec(struct fifo_tx* self, int fildes, struct stat* buf,
                   bool isnoundo, int* cookie, struct picotm_error* error);

void
fifo_tx_fstat_apply(struct fifo_tx* self, int fildes, int cookie,
                    struct picotm_error* error);

void
fifo_tx_fstat_undo(struct fifo_tx* self, int fildes, int cookie,
                   struct picotm_error* error);

/*
 * read()
 */

ssize_t
fifo_tx_read_exec(struct fifo_tx* self, int fildes, void* buf, size_t nbyte,
                  bool isnoundo, enum picotm_libc_validation_mode val_mode,
                  int* cookie,  struct picotm_error* error);

void
fifo_tx_read_apply(struct fifo_tx* self, int fildes, int cookie,
                   struct picotm_error* error);

void
fifo_tx_read_undo(struct fifo_tx* self, int fildes, int cookie,
                  struct picotm_error* error);

/*
 * write()
 */

ssize_t
fifo_tx_write_exec(struct fifo_tx* self, int fildes, const void* buf,
                   size_t nbyte, bool isnoundo, int* cookie,
                   struct picotm_error* error);

void
fifo_tx_write_apply(struct fifo_tx* self, int fildes, int cookie,
                    struct picotm_error* error);

void
fifo_tx_write_undo(struct fifo_tx* self, int fildes, int cookie,
                   struct picotm_error* error);
