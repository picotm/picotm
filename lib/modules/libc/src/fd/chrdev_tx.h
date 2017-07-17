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
#include "chrdev.h"
#include "ofd_tx.h"
#include "picotm/picotm-libc.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

union fcntl_arg;

/**
 * Holds transaction-local state for a character device.
 */
struct chrdev_tx {

    struct picotm_ref16 ref;

    SLIST_ENTRY(chrdev_tx) active_list;

    struct ofd_tx base;

    struct chrdev* chrdev;

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

    struct fcntlop* fcntltab;
    size_t          fcntltablen;

    /** Concurrency-control mode */
    enum picotm_libc_cc_mode cc_mode;

    /** State of the local reader/writer locks */
    struct picotm_rwstate rwstate[NUMBER_OF_CHRDEV_FIELDS];
};

struct chrdev_tx*
chrdev_tx_of_ofd_tx(struct ofd_tx* ofd_tx);

/**
 * Initialize transaction-local character device.
 * \param   self    The instance of `struct chrdev` to initialize.
 */
void
chrdev_tx_init(struct chrdev_tx* self);

/**
 * Uninitialize transaction-local character device.
 * \param   self    The instance of `struct chrdev` to initialize.
 */
void
chrdev_tx_uninit(struct chrdev_tx* self);

/**
 * Validate the local state
 */
void
chrdev_tx_validate(struct chrdev_tx* self, struct picotm_error* error);

/**
 * Updates the data structures for concurrency control after a successful apply
 */
void
chrdev_tx_update_cc(struct chrdev_tx* self, struct picotm_error* error);

/**
 * Clears the data structures for concurrency control after a successful undo
 */
void
chrdev_tx_clear_cc(struct chrdev_tx* self, struct picotm_error* error);

/**
 * Acquire a reference on the open file description
 */
void
chrdev_tx_ref_or_set_up(struct chrdev_tx* self, struct chrdev* chrdev, int fildes,
                        unsigned long flags, struct picotm_error* error);

/**
 * Acquire a reference on the open file description
 */
void
chrdev_tx_ref(struct chrdev_tx* self);

/**
 * Release reference
 */
void
chrdev_tx_unref(struct chrdev_tx* self);

/**
 * Returns true if transactions hold a reference
 */
bool
chrdev_tx_holds_ref(struct chrdev_tx* self);

int
chrdev_tx_append_to_writeset(struct chrdev_tx* self, size_t nbyte, off_t offset,
                             const void* buf, struct picotm_error* error);

int
chrdev_tx_append_to_readset(struct chrdev_tx* self, size_t nbyte, off_t offset,
                            const void* buf, struct picotm_error* error);

/**
 * Prepares the open file description for commit
 */
void
chrdev_tx_lock(struct chrdev_tx* self);

/**
 * Finishes commit for open file description
 */
void
chrdev_tx_unlock(struct chrdev_tx* self);

/*
 * fcntl()
 */

int
chrdev_tx_fcntl_exec(struct chrdev_tx* self, int fildes, int cmd,
                     union fcntl_arg* arg, bool isnoundo, int* cookie,
                     struct picotm_error* error);

void
chrdev_tx_fcntl_apply(struct chrdev_tx* self, int fildes, int cookie,
                      struct picotm_error* error);

void
chrdev_tx_fcntl_undo(struct chrdev_tx* self, int fildes, int cookie,
                     struct picotm_error* error);

/*
 * read()
 */

ssize_t
chrdev_tx_read_exec(struct chrdev_tx* self, int fildes, void* buf, size_t nbyte,
                    bool isnoundo, enum picotm_libc_validation_mode val_mode,
                    int* cookie,  struct picotm_error* error);

void
chrdev_tx_read_apply(struct chrdev_tx* self, int fildes, int cookie,
                     struct picotm_error* error);

void
chrdev_tx_read_undo(struct chrdev_tx* self, int fildes, int cookie,
                    struct picotm_error* error);

/*
 * write()
 */

ssize_t
chrdev_tx_write_exec(struct chrdev_tx* self, int fildes, const void* buf,
                     size_t nbyte, bool isnoundo, int* cookie,
                     struct picotm_error* error);

void
chrdev_tx_write_apply(struct chrdev_tx* self, int fildes, int cookie,
                      struct picotm_error* error);

void
chrdev_tx_write_undo(struct chrdev_tx* self, int fildes, int cookie,
                     struct picotm_error* error);
