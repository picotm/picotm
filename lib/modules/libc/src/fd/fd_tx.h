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
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/queue.h>
#include "picotm/picotm-libc.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

#define FDTX_FL_LOCALSTATE 1L /** \brief Signals local state changes */

struct fcntlop;
struct ofd_tx;
struct picotm_error;

union fcntl_arg;

/**
 * Holds transaction-local reads and writes for a file descriptor
 */
struct fd_tx {

    struct picotm_ref16 ref;

    SLIST_ENTRY(fd_tx) active_list;

    struct fd* fd;

    struct ofd_tx* ofd_tx;
    enum picotm_libc_cc_mode cc_mode;

	unsigned long flags;

    struct fcntlop* fcntltab;
    size_t          fcntltablen;

    unsigned long fdver; /* Last fd version, modified by fdtx */
};

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

void
fd_tx_update_cc(struct fd_tx* self, struct picotm_error* error);

void
fd_tx_clear_cc(struct fd_tx* self, struct picotm_error* error);

/**
 * Aquire a reference on file-descriptor state
 */
void
fd_tx_ref_or_set_up(struct fd_tx* self, struct fd* fd, struct ofd_tx* ofd_tx,
                    unsigned long flags, struct picotm_error* error);

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

void
fd_tx_lock(struct fd_tx* self);

void
fd_tx_unlock(struct fd_tx* self);

/**
 * Set file descriptor to CLOSING
 */
void
fd_tx_signal_close(struct fd_tx* self);

/**
 * Dump file-descriptor state
 */
void
fd_tx_dump(const struct fd_tx* self);

/*
 * close()
 */

int
fd_tx_close_exec(struct fd_tx* self, int fildes, int* cookie, int noundo,
                 struct picotm_error* error);

void
fd_tx_close_apply(struct fd_tx* self, int fildes, int cookie,
                  struct picotm_error* error);

void
fd_tx_close_undo(struct fd_tx* self, int fildes, int cookie,
                 struct picotm_error* error);

/*
 * fcntl()
 */

int
fd_tx_fcntl_exec(struct fd_tx* self, int cmd, union fcntl_arg *arg,
                 int* cookie, int noundo, struct picotm_error* error);

void
fd_tx_fcntl_apply(struct fd_tx* self, int cookie, bool* next_domain,
                  struct picotm_error* error);

void
fd_tx_fcntl_undo(struct fd_tx* self, int cookie, bool* next_domain,
                 struct picotm_error* error);
