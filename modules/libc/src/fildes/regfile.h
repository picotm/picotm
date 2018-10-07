/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include "picotm/picotm-lib-rwlock.h"
#include "picotm/picotm-lib-shared-ref-obj.h"
#include "fileid.h"
#include "rwlockmap.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;
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

    /** Reference-counting base object. */
    struct picotm_shared_ref16_obj ref_obj;

    /** The file's unique id. */
    struct file_id id;

    /** Reader/writer state locks. */
    struct picotm_rwlock  rwlock[NUMBER_OF_REGFILE_FIELDS];

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
 * \param       self    The file instance.
 * \param[out]  error   Returns an error to the caller.
 */
void
regfile_ref(struct regfile* self, struct picotm_error* error);

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
