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
#include "file.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;
struct picotm_rwstate;

/**
 * Enumerates fields of `struct socket`.
 */
enum socket_field {
    SOCKET_FIELD_FILE_MODE,
    SOCKET_FIELD_RECV_END,
    SOCKET_FIELD_SEND_END,
    SOCKET_FIELD_STATE,
    NUMBER_OF_SOCKET_FIELDS
};

/**
 * Represents a socket's open file description.
 */
struct socket {

    /** Base object. */
    struct file base;

    /** Reader/writer state locks. */
    struct picotm_rwlock rwlock[NUMBER_OF_SOCKET_FIELDS];
};

/**
 * \brief Initializes a socket instance.
 * \param       self    The socket instance to initialize.
 * \param[out]  error   Returns an error to the caller.
 */
void
socket_init(struct socket* self, struct picotm_error* error);

/**
 * \brief Uninitializes a socket instance.
 * \param   self    The socket instance to uninitialize.
 */
void
socket_uninit(struct socket* self);

/**
 * \brief Tries to acquire a reader lock on a socket.
 * \param       self        The socket instance.
 * \param       field       The reader lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
socket_try_rdlock_field(struct socket* self, enum socket_field field,
                        struct picotm_rwstate* rwstate,
                        struct picotm_error* error);

/**
 * \brief Tries to acquire a writer lock on a socket.
 * \param       self        The socket instance.
 * \param       field       The writer lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
socket_try_wrlock_field(struct socket* self, enum socket_field field,
                        struct picotm_rwstate* rwstate,
                        struct picotm_error* error);

/**
 * \brief Releases a lock on a socket.
 * \param   self    The socket instance.
 * \param   field   The reader/writer lock's field.
 * \param   rwstate The transaction's reader/writer state.
 */
void
socket_unlock_field(struct socket* self, enum socket_field field,
                    struct picotm_rwstate* rwstate);
