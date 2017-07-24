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
#include <stdbool.h>
#include "fileid.h"
#include "picotm/picotm-libc.h"
#include "rwlockmap.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_rwstate;
struct rwcountermap;

/**
 * Enumerates fields of `struct regfile`.
 */
enum regfile_field {
    REGFILE_FIELD_FILE_MODE,
    REGFILE_FIELD_FILE_SIZE,
    REGFILE_FIELD_STATE,
    NUMBER_OF_REGFILE_FIELDS
};

/**
 * Represents a file's open file description.
 */
struct regfile {

    /** Internal lock. */
    pthread_rwlock_t lock;

    /** The reference counter. */
    struct picotm_shared_ref16 ref;

    /** The file's unique id. */
    struct file_id id;

    /** Concurrency-control mode for the file. */
    enum picotm_libc_cc_mode cc_mode;

    /** Reader/writer state locks. */
    struct picotm_rwlock  rwlock[NUMBER_OF_REGFILE_FIELDS];

    /** \brief Current file offset. */
    off_t offset;

    /** \brief Reader/writer region-lock table. */
    struct rwlockmap rwlockmap;
};

/**
 * \brief Initializes a file instance.
 * \param       self    The file instance to initialize.
 * \param[out]  error   Returns an error to the caller.
 */
void
regfile_init(struct regfile* self, struct picotm_error* error);

/**
 * \brief Uninitializes a file instance.
 * \param   self    The file instance to uninitialize.
 */
void
regfile_uninit(struct regfile* self);

/**
 * \brief Sets up an instance of `struct regfile` or acquires a reference
 *        on an already set-up instance.
 * \param       self    The file instance.
 * \param       fildes  The file's file descriptor.
 * \param[out]  error   Returns an error to the caller.
 */
void
regfile_ref_or_set_up(struct regfile* self, int fildes,
                      struct picotm_error* error);

/**
 * \brief Acquires a reference on an instance of `struct regfile`.
 * \param   self    The file instance.
 */
void
regfile_ref(struct regfile* self);

/**
 * \brief Compares the file's id to an id and acquires a reference if both
 *        id's are equal.
 * \param   self    The file instance.
 * \param   id      The id to compare to.
 * \returns A value less than, equal to, or greater than if the file's id
 *          is less than, equal to, or greater than the given id.
 */
int
regfile_cmp_and_ref(struct regfile* self, const struct file_id* id);

/**
 * \brief Compares the file's id to an id and acquires a reference if both
 *        id's are equal. The file instance is set up from the provided
 *        file descriptor if necessary.
 * \param       self        The file instance.
 * \param       id          The id to compare to.
 * \param       fildes      The file's file descriptor.
 * \param[out]  error       Returns an error ot the caller.
 * \returns A value less than, equal to, or greater than if the ofd's id is
 *          less than, equal to, or greater than the given id.
 */
int
regfile_cmp_and_ref_or_set_up(struct regfile* self, const struct file_id* id,
                              int fildes, struct picotm_error* error);

/**
 * \brief Unreferences a file.
 * \param   self    The file instance.
 */
void
regfile_unref(struct regfile* self);

/**
 * \brief Returns the current concurrency-control mode of a file
 *        instance.
 * \param   self    The file instance.
 * \returns The current concurrency-control mode of the given file.
 */
enum picotm_libc_cc_mode
regfile_get_cc_mode(struct regfile* self);

/**
 * \brief Returns the current file offset of a file instance.
 * \param   self    The file instance.
 * \returns The current file offset of the given file.
 */
off_t
regfile_get_offset(struct regfile* self);

/**
 * \brief Tries to acquire a reader lock on a file.
 * \param       self        The file instance.
 * \param       field       The reader lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
regfile_try_rdlock_field(struct regfile* self, enum regfile_field field,
                         struct picotm_rwstate* rwstate,
                         struct picotm_error* error);

/**
 * \brief Tries to acquire a writer lock on a file.
 * \param       self        The file instance.
 * \param       field       The writer lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
regfile_try_wrlock_field(struct regfile* self, enum regfile_field field,
                         struct picotm_rwstate* rwstate,
                         struct picotm_error* error);

/**
 * \brief Releases a lock on a file.
 * \param   self    The file instance.
 * \param   field   The reader/writer lock's field.
 * \param   rwstate The transaction's reader/writer state.
 */
void
regfile_unlock_field(struct regfile* self, enum regfile_field field,
                     struct picotm_rwstate* rwstate);

/**
 * \brief Tries to acquire a reader lock on a file region.
 * \param       self            The file instance.
 * \param       off             The region's offset.
 * \param       nbyte           The region's length.
 * \param       rwcountermap    The transaction's reader/writer counter map.
 * \param[out]  error           Returns an error ot the caller.
 */
void
regfile_try_rdlock_region(struct regfile* self, off_t off, size_t nbyte,
                          struct rwcountermap* rwcountermap,
                          struct picotm_error* error);

/**
 * \brief Tries to acquire a writer lock on a file region.
 * \param       self            The file instance.
 * \param       off             The region's offset.
 * \param       nbyte           The region's length.
 * \param       rwcountermap    The transaction's reader/writer counter map.
 * \param[out]  error           Returns an error ot the caller.
 */
void
regfile_try_wrlock_region(struct regfile* self, off_t off, size_t nbyte,
                          struct rwcountermap* rwcountermap,
                          struct picotm_error* error);

/**
 * \brief Releases a reader/writer lock on a file region.
 * \param   self            The file instance.
 * \param   off             The region's offset.
 * \param   nbyte           The region's length.
 * \param   rwcountermap    The transaction's reader/writer counter map.
 */
void
regfile_unlock_region(struct regfile* self, off_t off, size_t nbyte,
                      struct rwcountermap* rwcountermap);
