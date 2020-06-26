/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2019-2020  Thomas Zimmermann <contact@tzimmermann.org>
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

#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-rwlock.h"
#include "picotm/picotm-lib-shared-ref-obj.h"
#include "filebuf_id.h"
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
 * Enumerates fields of `struct seekbuf`.
 */
enum seekbuf_field {
    SEEKBUF_FIELD_FILE_SIZE,
    SEEKBUF_FIELD_STATE,
    NUMBER_OF_SEEKBUF_FIELDS
};

/**
 * Represents a seekable I/O buffer, such as a seekbuf.
 */
struct seekbuf {

    /** Reference-counting base object. */
    struct picotm_shared_ref16_obj ref_obj;

    /** The seekbuf's unique id. */
    struct filebuf_id id;

    /** Reader/writer state locks. */
    struct picotm_rwlock  rwlock[NUMBER_OF_SEEKBUF_FIELDS];

    /** \brief Reader/writer region-lock table. */
    struct rwlockmap rwlockmap;
};

/**
 * \brief Initializes a seekbuf instance.
 * \param       self    The seekbuf instance to initialize.
 * \param[out]  error   Returns an error to the caller.
 */
void
seekbuf_init(struct seekbuf* self, struct picotm_error* error);

/**
 * \brief Uninitializes a seekbuf instance.
 * \param   self    The seekbuf instance to uninitialize.
 */
void
seekbuf_uninit(struct seekbuf* self);

/**
 * \brief Sets up an instance of `struct seekbuf` or acquires a reference
 *        on an already set-up instance.
 * \param       self    The seekbuf instance.
 * \param       fildes  The seekbuf's seekbuf descriptor.
 * \param[out]  error   Returns an error to the caller.
 */
void
seekbuf_ref_or_set_up(struct seekbuf* self, int fildes,
                      struct picotm_error* error);

/**
 * \brief Compares the seekbuf's id to an id and acquires a reference if both
 *        id's are equal. The seekbuf instance is set up from the provided
 *        seekbuf descriptor if necessary.
 * \param       self        The seekbuf instance.
 * \param       fildes      The seekbuf's seekbuf descriptor.
 * \param       id          The id to compare to.
 * \param[out]  error       Returns an error ot the caller.
 * \returns A value less than, equal to, or greater than if the seekbuf's
 *          id is less than, equal to, or greater than the given id.
 */
int
seekbuf_ref_or_set_up_if_id(struct seekbuf* self, int fildes,
                         const struct filebuf_id* id,
                         struct picotm_error* error);

/**
 * \brief Acquires a reference on an instance of `struct seekbuf`.
 * \param       self    The seekbuf instance.
 * \param[out]  error   Returns an error to the caller.
 */
void
seekbuf_ref(struct seekbuf* self, struct picotm_error* error);

/**
 * \brief Compares the seekbuf's id to an id and acquires a reference if both
 *        id's are equal.
 * \param   self    The seekbuf instance.
 * \param   id      The id to compare to.
 * \returns A value less than, equal to, or greater than if the seekbuf's id
 *          is less than, equal to, or greater than the given id.
 */
int
seekbuf_ref_if_id(struct seekbuf* self, const struct filebuf_id* id);

/**
 * \brief Unreferences a seekbuf.
 * \param   self    The seekbuf instance.
 */
void
seekbuf_unref(struct seekbuf* self);

/**
 * \brief Tries to acquire a reader lock on a seekbuf.
 * \param       self        The seekbuf instance.
 * \param       field       The reader lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
seekbuf_try_rdlock_field(struct seekbuf* self, enum seekbuf_field field,
                         struct picotm_rwstate* rwstate,
                         struct picotm_error* error);

/**
 * \brief Tries to acquire a writer lock on a seekbuf.
 * \param       self        The seekbuf instance.
 * \param       field       The writer lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
seekbuf_try_wrlock_field(struct seekbuf* self, enum seekbuf_field field,
                         struct picotm_rwstate* rwstate,
                         struct picotm_error* error);

/**
 * \brief Releases a lock on a seekbuf.
 * \param   self    The seekbuf instance.
 * \param   field   The reader/writer lock's field.
 * \param   rwstate The transaction's reader/writer state.
 */
void
seekbuf_unlock_field(struct seekbuf* self, enum seekbuf_field field,
                     struct picotm_rwstate* rwstate);

/**
 * \brief Tries to acquire a reader lock on a seekbuf region.
 * \param       self            The seekbuf instance.
 * \param       off             The region's offset.
 * \param       nbyte           The region's length.
 * \param       rwcountermap    The transaction's reader/writer counter map.
 * \param[out]  error           Returns an error ot the caller.
 */
void
seekbuf_try_rdlock_region(struct seekbuf* self, off_t off, size_t nbyte,
                          struct rwcountermap* rwcountermap,
                          struct picotm_error* error);

/**
 * \brief Tries to acquire a writer lock on a seekbuf region.
 * \param       self            The seekbuf instance.
 * \param       off             The region's offset.
 * \param       nbyte           The region's length.
 * \param       rwcountermap    The transaction's reader/writer counter map.
 * \param[out]  error           Returns an error ot the caller.
 */
void
seekbuf_try_wrlock_region(struct seekbuf* self, off_t off, size_t nbyte,
                          struct rwcountermap* rwcountermap,
                          struct picotm_error* error);

/**
 * \brief Releases a reader/writer lock on a seekbuf region.
 * \param   self            The seekbuf instance.
 * \param   off             The region's offset.
 * \param   nbyte           The region's length.
 * \param   rwcountermap    The transaction's reader/writer counter map.
 */
void
seekbuf_unlock_region(struct seekbuf* self, off_t off, size_t nbyte,
                      struct rwcountermap* rwcountermap);
