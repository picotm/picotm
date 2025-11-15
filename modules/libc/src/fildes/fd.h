/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann
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

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;
struct picotm_rwstate;

union fcntl_arg;

/**
 * \brief Enumerates the file-descriptor state.
 */
enum fd_state {
    /** File descriptor is currenty not in use */
    FD_STATE_UNUSED,
    /** File descriptor is open */
    FD_STATE_INUSE,
    /** File descriptor has been closed by transaction; other users need to abort */
    FD_STATE_CLOSING
};

/**
 * Enumerates fields of `struct fd`.
 */
enum fd_field {
    FD_FIELD_STATE,
    NUMBER_OF_FD_FIELDS
};

struct fd {

    /** Reference-counting base object. */
    struct picotm_shared_ref16_obj ref_obj;

    int fildes;
    enum fd_state state;

    /** File-descriptor reader/writer locks */
    struct picotm_rwlock rwlock[NUMBER_OF_FD_FIELDS];
};

/**
 * Initializes global file-descriptor state.
 * \param       self    The file-descriptor structure.
 * \param[out]  error   Returns an error to the caller.
 */
void
fd_init(struct fd* self, struct picotm_error* error);

/**
 * Uninitializes global file-descriptor state.
 * \param   self    The file-descriptor structure.
 */
void
fd_uninit(struct fd* self);

/**
 * Tries to acquires a reader lock on a file descriptor.
 * \param       self    The file-descriptor structure.
 * \param       field   The reader lock's field.
 * \param       state   The transaction's reader-writer state.
 * \param[out]  error   Returns an error.
 */
void
fd_try_rdlock_field(struct fd* self, enum fd_field field,
                    struct picotm_rwstate* state,
                    struct picotm_error* error);

/**
 * Tries to acquires a writer lock on a file descriptor.
 * \param       self    The file-descriptor structure.
 * \param       field   The writer lock's field.
 * \param       state   The transaction's reader-writer state.
 * \param[out]  error   Returns an error.
 */
void
fd_try_wrlock_field(struct fd* self, enum fd_field field,
                    struct picotm_rwstate* state,
                    struct picotm_error* error);

/**
 * Releases a reader/writer lock.
 * \param   self    The file-descriptor structure.
 * \param   field   The reader/writer lock's field.
 * \param   state   The transaction's reader-writer state.
 */
void
fd_unlock_field(struct fd* self, enum fd_field field,
                struct picotm_rwstate* state);

/** \brief Aquires a reference on the file dscriptor */
void
fd_ref(struct fd* self, struct picotm_error* error);

/** \brief Aquires a reference on the file descriptor */
void
fd_ref_or_set_up(struct fd* self, int fildes, struct picotm_error* error);

/** \brief Releases a reference on the file descriptor */
void
fd_unref(struct fd* self);

/** \brief Return non-zero value if file-descriptor is open */
int
fd_is_open_nl(const struct fd* self);

/** \brief Set file descriptor to state close */
void
fd_close(struct fd* self);

/** Set file-descriptor flags. */
int
fd_setfd(struct fd* self, int arg, struct picotm_error* error);

/** Get file-descriptor flags. */
int
fd_getfd(struct fd* self, struct picotm_error* error);
