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
#include "ofd.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct file_tx;

/**
 * Holds the transaction-local state for an open file description.
 */
struct ofd_tx {

    struct picotm_ref16 ref;

    SLIST_ENTRY(ofd_tx) active_list;

    struct ofd* ofd;

    struct file_tx* file_tx;

    struct seekop* seektab;
    size_t         seektablen;

    /** \brief Transaction-local file offset */
    off_t offset;

    /** State of the local reader/writer locks. */
    struct picotm_rwstate rwstate[NUMBER_OF_OFD_FIELDS];
};

/**
 * Init transaction-local open-file-description state.
 * \param   self    Transaction-local open file description.
 */
void
ofd_tx_init(struct ofd_tx* self);

/**
 * Uninit transaction-local open-file-description state.
 * \param   self    Transaction-local open file description.
 */
void
ofd_tx_uninit(struct ofd_tx* self);

/**
 * \brief Tests if a transaction-local open-file-description state holds a
 *        reference to a global state.
 * \param   self    An transaction-local open-file-description state structure.
 * \returns True if the transaction-local state references global state,
 *          false otherwise.
 */
bool
ofd_tx_holds_ref(struct ofd_tx* self);

/**
 * \brief Compare id of open file description to reference id.
 * \param   self    Transaction-local open file description.
 * \param   id      An open-file-description id.
 * \returns A value less than, equal to, or greater than 0 if the
 *          open file description's id is less than, equal to, or
 *          greater than the reference id.
 */
int
ofd_tx_cmp_with_id(const struct ofd_tx* self, const struct ofd_id* id);

/**
 * \brief Returns the current file offset of a transaction-local open file
 *        description.
 * \param       self    The transaction-local open-file-description state.
 * \param       fildes  The file descriptor.
 * \param[out]  error   Returns an error to the caller.
 * \returns The open file description's current file offset.
 */
off_t
ofd_tx_get_file_offset(struct ofd_tx* self, int fildes,
                       struct picotm_error* error);

/**
 * \brief Sets the current file offset of a transaction-local open file
 *        description.
 * \param       self    The transaction-local open-file-description state.
 * \param       offset  The new file offset.
 * \param[out]  error   Returns an error to the caller.
 */
void
ofd_tx_set_file_offset(struct ofd_tx* self, off_t offset,
                       struct picotm_error* error);

#define _ofd_tx_file_op(_op, _file_op, _ofd_tx, ...) \
    (_ofd_tx)->file_tx->ops->_op ## _ ## _file_op((_ofd_tx)->file_tx, \
                                                  (_ofd_tx), \
                                                  __VA_ARGS__)

/**
 * \brief Invokes to an exec() operation.
 * \param   _op     The name of the operation.
 * \param   _ofx_tx The transaction-local open file description.
 */
#define ofd_tx_exec(_op, _ofd_tx, ...) \
    _ofd_tx_file_op(_op, exec, (_ofd_tx), __VA_ARGS__)

/**
 * \brief Invokes to an apply() operation.
 * \param       _op     The name of the operation.
 * \param       _ofx_tx The transaction-local open file description.
 * \param       _fildes The file descriptor.
 * \param       _cookie Invocation-specific user data.
 * \param[out]  _error  Returns an error to the caller.
 */
#define ofd_tx_apply(_op, _ofd_tx, _fildes, _cookie, _error) \
    _ofd_tx_file_op(_op, apply, (_ofd_tx), (_fildes), (_cookie), (_error))

/**
 * \brief Invokes to an undo() operation.
 * \param       _op     The name of the operation.
 * \param       _ofx_tx The transaction-local open file description.
 * \param       _fildes The file descriptor.
 * \param       _cookie Invocation-specific user data.
 * \param[out]  _error  Returns an error to the caller.
 */
#define ofd_tx_undo(_op, _ofd_tx, _fildes, _cookie, _error) \
    _ofd_tx_file_op(_op, undo, (_ofd_tx), (_fildes), (_cookie), (_error))

/*
 * Reference counting
 */

/**
 * \brief Sets up a transaction-local open file description or acquires
 *        a reference on an already set-up instance.
 * \param       self    The transaction-local open file description.
 * \param       ofd     The global open file description.
 * \param       file_tx The transaction-local file state.
 * \param[out]  error   Returns an error to the caller.
 */
void
ofd_tx_ref_or_set_up(struct ofd_tx* self, struct ofd* ofd,
                     struct file_tx* file_tx, struct picotm_error* error);

/**
 * \brief Acquires a reference on the transaction-local open file description.
 * \param       self    The transaction-local open file description.
 * \param[out]  error   Returns an error to the caller.
 */
void
ofd_tx_ref(struct ofd_tx* self, struct picotm_error* error);

/**
 * \brief Releases a reference on the transaction-local open file description.
 * \param   self    The transaction-local open file description.
 */
void
ofd_tx_unref(struct ofd_tx* self);

/*
 * Locking
 */

/**
 * \brief Tries to acquire a reader lock on an open file description.
 * \param       self        The transaction-local open file description.
 * \param       field       The reader lock's field.
 * \param[out]  error       Returns an error ot the caller.
 */
void
ofd_tx_try_rdlock_field(struct ofd_tx* self, enum ofd_field field,
                        struct picotm_error* error);

/**
 * \brief Tries to acquire a writer lock on an open file description.
 * \param       self        The transaction-local open file description.
 * \param       field       The reader lock's field.
 * \param[out]  error       Returns an error ot the caller.
 */
void
ofd_tx_try_wrlock_field(struct ofd_tx* self, enum ofd_field field,
                        struct picotm_error* error);
/*
 * Module interface
 */

/**
 * Prepares the open file description for commit
 */
void
ofd_tx_lock(struct ofd_tx* self, struct picotm_error* error);

/**
 * Finishes commit for open file description
 */
void
ofd_tx_unlock(struct ofd_tx* self, struct picotm_error* error);

/**
 * Validate the local state
 */
void
ofd_tx_validate(struct ofd_tx* self, struct picotm_error* error);

/**
 * Updates the data structures for concurrency control after a successful apply
 */
void
ofd_tx_update_cc(struct ofd_tx* self, struct picotm_error* error);

/**
 * Clears the data structures for concurrency control after a successful undo
 */
void
ofd_tx_clear_cc(struct ofd_tx* self, struct picotm_error* error);
