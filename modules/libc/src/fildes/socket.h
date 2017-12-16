/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
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
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <picotm/picotm-lib-rwlock.h>
#include <picotm/picotm-lib-shared-ref-obj.h>
#include "fileid.h"

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

    /** Reference-counting base object. */
    struct picotm_shared_ref16_obj ref_obj;

    /** The socket's unique id. */
    struct file_id id;

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
 * \brief Sets up an instance of `struct socket` or acquires a reference
 *        on an already set-up instance.
 * \param       self    The socket instance.
 * \param       fildes  The socket's file descriptor.
 * \param[out]  error   Returns an error to the caller.
 */
void
socket_ref_or_set_up(struct socket* self, int fildes,
                     struct picotm_error* error);

/**
 * \brief Acquires a reference on an instance of `struct socket`.
 * \param       self    The socket instance.
 * \param[out]  error   Returns an error to the caller.
 */
void
socket_ref(struct socket* self, struct picotm_error* error);

/**
 * \brief Compares the socket's id to an id and acquires a reference if both
 *        id's are equal.
 * \param   self    The socket instance.
 * \param   id      The id to compare to.
 * \returns A value less than, equal to, or greater than if the socket's id
 *          is less than, equal to, or greater than the given id.
 */
int
socket_cmp_and_ref(struct socket* self, const struct file_id* id);

/**
 * \brief Compares the socket's id to an id and acquires a reference if both
 *        id's are equal. The socket instance is set up from the provided
 *        file descriptor if necessary.
 * \param       self        The socket instance.
 * \param       id          The id to compare to.
 * \param       fildes      The socket's file descriptor.
 * \param[out]  error       Returns an error ot the caller.
 * \returns A value less than, equal to, or greater than if the ofd's id is
 *          less than, equal to, or greater than the given id.
 */
int
socket_cmp_and_ref_or_set_up(struct socket* self, const struct file_id* id,
                             int fildes, struct picotm_error* error);

/**
 * \brief Unreferences a socket.
 * \param   self    The socket instance.
 */
void
socket_unref(struct socket* self);

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
