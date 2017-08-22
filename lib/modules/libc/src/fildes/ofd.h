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

#include <picotm/picotm-lib-rwlock.h>
#include <picotm/picotm-lib-shared-ref-obj.h>
#include <stdbool.h>
#include <sys/types.h>
#include "ofd_id.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_rwstate;

/**
 * Enumerates fields of `struct ofd`.
 */
enum ofd_field {
    OFD_FIELD_FILE_OFFSET,
    OFD_FIELD_STATE,
    NUMBER_OF_OFD_FIELDS
};

/**
 * Represents an open file description.
 */
struct ofd {

    /** Reference-counting base object. */
    struct picotm_shared_ref16_obj ref_obj;

    /** The open file description's unique id. */
    struct ofd_id id;

    /** Reader/writer state locks. */
    struct picotm_rwlock  rwlock[NUMBER_OF_OFD_FIELDS];
};

/**
 * \brief Initializes an open file description.
 * \param       self    The open file description to initialize.
 * \param[out]  error   Returns an error to the caller.
 */
void
ofd_init(struct ofd* self, struct picotm_error* error);

/**
 * \brief Uninitializes an open file description.
 * \param   self    The open file description to uninitialize.
 */
void
ofd_uninit(struct ofd* self);

/**
 * \brief Acquires a reference on an open file description.
 * \param   self    The open file description.
 */
void
ofd_ref(struct ofd* self);

/**
 * \brief Sets up an open file description or acquires a reference
 *        on an already set-up instance.
 * \param       self    The open file description.
 * \param       fildes  A file descriptor for the setup.
 * \param[out]  error   Returns an error to the caller.
 */
void
ofd_ref_or_set_up(struct ofd* self, int fildes,
                  struct picotm_error* error);

/**
 * \brief Releases a reference on an open file description.
 * \param   self    The open file description.
 */
void
ofd_unref(struct ofd* self);

/**
 * \brief Compares the open file description's id to a reference id and
 *        acquires a reference if both id's are equal. The instance is
 *        set up from the provided file descriptor if necessary.
 * \param       self        The open file description.
 * \param       id          The id to compare to.
 * \param       fildes      A file descriptor for the setup.
 * \param       ne_fildes   True to request non-equal file descriptors, false
 *                          otherwise.
 * \param[out]  error       Returns an error ot the caller.
 * \returns A value less than, equal to, or greater than if the ofd's id is
 *          less than, equal to, or greater than the given id.
 */
int
ofd_cmp_and_ref_or_set_up(struct ofd* self, const struct ofd_id* id,
                          int fildes, bool ne_fildes,
                          struct picotm_error* error);

/**
 * \brief Compares the open file description's id to a reference id and
 *        acquires a reference if both id's are equal.
 * \param       self        The open file description.
 * \param       id          The id to compare to.
 * \param       ne_fildes   True to request non-equal file descriptors, false
 *                          otherwise.
 * \param[out]  error       Returns an error ot the caller.
 * \returns A value less than, equal to, or greater than if the ofd's id is
 *          less than, equal to, or greater than the given id.
 */
int
ofd_cmp_and_ref(struct ofd* self, const struct ofd_id* id, bool ne_fildes,
                struct picotm_error* error);

/**
 * \brief Tries to acquire a reader lock on an open file description.
 * \param       self        The open file description.
 * \param       field       The reader lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
ofd_try_rdlock_field(struct ofd* self, enum ofd_field field,
                     struct picotm_rwstate* rwstate,
                     struct picotm_error* error);

/**
 * \brief Tries to acquire a writer lock on an open file description.
 * \param       self        The open file description.
 * \param       field       The writer lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
ofd_try_wrlock_field(struct ofd* self, enum ofd_field field,
                     struct picotm_rwstate* rwstate,
                     struct picotm_error* error);

/**
 * \brief Releases a lock on an open file description.
 * \param   self    The open file description.
 * \param   field   The reader/writer lock's field.
 * \param   rwstate The transaction's reader/writer state.
 */
void
ofd_unlock_field(struct ofd* self, enum ofd_field field,
                 struct picotm_rwstate* rwstate);
