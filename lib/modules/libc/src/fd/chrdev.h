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
#include <picotm/picotm-lib-rwlock.h>
#include <pthread.h>
#include "fileid.h"
#include "picotm/picotm-libc.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_rwstate;

/**
 * Enumerates fields of `struct chrdev`.
 */
enum chrdev_field {
    CHRDEV_FIELD_FILE_MODE,
    CHRDEV_FIELD_FILE_OFFSET,
    CHRDEV_FIELD_STATE,
    NUMBER_OF_CHRDEV_FIELDS
};

/**
 * Represents a character device's open file description.
 */
struct chrdev {

    /** Internal lock. */
    pthread_rwlock_t lock;

    /** The reference counter. */
    struct picotm_shared_ref16 ref;

    /** The character device's unique id. */
    struct file_id id;

    /** Concurrency-control mode for the character device. */
    enum picotm_libc_cc_mode cc_mode;

    /** Reader/writer state locks. */
    struct picotm_rwlock rwlock[NUMBER_OF_CHRDEV_FIELDS];
};

/**
 * \brief Initializes a chrdev instance.
 * \param       self    The chrdev instance to initialize.
 * \param[out]  error   Returns an error to the caller.
 */
void
chrdev_init(struct chrdev* self, struct picotm_error* error);

/**
 * \brief Uninitializes a chrdev structure.
 * \param   self    The chrdev instance to uninitialize.
 */
void
chrdev_uninit(struct chrdev* self);

/**
 * \brief Sets up an instance of `struct chrdev` or acquires a reference
 *        on an already set-up instance.
 * \param       self    The chrdev instance.
 * \param       fildes  The character device's file descriptor.
 * \param[out]  error   Returns an error to the caller.
 */
void
chrdev_ref_or_set_up(struct chrdev* self, int fildes,
                     struct picotm_error* error);

/**
 * \brief Acquires a reference on an instance of `struct chrdev`.
 * \param   self    The chrdev instance.
 */
void
chrdev_ref(struct chrdev* self);

/**
 * \brief Compares the chrdev's id to an id and acquires a reference if both
 *        id's are equal.
 * \param   self    The chrdev instance.
 * \param   id      The id to compare to.
 * \returns A value less than, equal to, or greater than if the chrdev's id
 *          is less than, equal to, or greater than the given id.
 */
int
chrdev_cmp_and_ref(struct chrdev* self, const struct file_id* id);

/**
 * \brief Compares the chrdev's id to an id and acquires a reference if both
 *        id's are equal. The chrdev instance is set up from the provided
 *        file descriptor if necessary.
 * \param       self        The chrdev instance.
 * \param       id          The id to compare to.
 * \param       fildes      The character device's file descriptor.
 * \param[out]  error       Returns an error ot the caller.
 * \returns A value less than, equal to, or greater than if the ofd's id is
 *          less than, equal to, or greater than the given id.
 */
int
chrdev_cmp_and_ref_or_set_up(struct chrdev* self, const struct file_id* id,
                             int fildes, struct picotm_error* error);

/**
 * \brief Unreferences a character device.
 * \param   self    The chrdev instance.
 */
void
chrdev_unref(struct chrdev* self);

/**
 * \brief Returns the current concurrency-control mode of a chrdev
 *        instance.
 * \param   self    The chrdev instance.
 * \returns The current concurrency-control mode of the given chrdev.
 */
enum picotm_libc_cc_mode
chrdev_get_cc_mode(struct chrdev* self);

/**
 * \brief Tries to acquire a reader lock on a character device.
 * \param       self        The chrdev instance.
 * \param       field       The reader lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
chrdev_try_rdlock_field(struct chrdev* self, enum chrdev_field field,
                        struct picotm_rwstate* rwstate,
                        struct picotm_error* error);

/**
 * \brief Tries to acquire a writer lock on a character device.
 * \param       self        The chrdev instance.
 * \param       field       The writer lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
chrdev_try_wrlock_field(struct chrdev* self, enum chrdev_field field,
                        struct picotm_rwstate* rwstate,
                        struct picotm_error* error);

/**
 * \brief Releases a lock on a character device.
 * \param   self    The chrdev instance.
 * \param   field   The reader/writer lock's field.
 * \param   rwstate The transaction's reader/writer state.
 */
void
chrdev_unlock_field(struct chrdev* self, enum chrdev_field field,
                    struct picotm_rwstate* rwstate);
