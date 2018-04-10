/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "picotm/picotm-lib-rwlock.h"
#include "picotm/picotm-lib-shared-ref-obj.h"
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
 * Enumerates fields of `struct fifo`.
 */
enum fifo_field {
    FIFO_FIELD_FILE_MODE,
    FIFO_FIELD_READ_END,
    FIFO_FIELD_STATE,
    FIFO_FIELD_WRITE_END,
    NUMBER_OF_FIFO_FIELDS
};

/**
 * Represents a FIFO's open file description.
 */
struct fifo {

    /** Reference-counting base object. */
    struct picotm_shared_ref16_obj ref_obj;

    /** The FIFO's unique id. */
    struct file_id id;

    /** Reader/writer state locks. */
    struct picotm_rwlock  rwlock[NUMBER_OF_FIFO_FIELDS];
};

/**
 * \brief Initializes a FIFO instance.
 * \param       self    The FIFO instance to initialize.
 * \param[out]  error   Returns an error to the caller.
 */
void
fifo_init(struct fifo* self, struct picotm_error* error);

/**
 * \brief Uninitializes a FIFO instance.
 * \param   self    The FIFO instance to uninitialize.
 */
void
fifo_uninit(struct fifo* self);

/**
 * \brief Sets up an instance of `struct fifo` or acquires a reference
 *        on an already set-up instance.
 * \param       self    The FIFO instance.
 * \param       fildes  The FIFO's file descriptor.
 * \param[out]  error   Returns an error to the caller.
 */
void
fifo_ref_or_set_up(struct fifo* self, int fildes, struct picotm_error* error);

/**
 * \brief Acquires a reference on an instance of `struct fifo`.
 * \param       self    The FIFO instance.
 * \param[out]  error   Returns an error to the caller.
 */
void
fifo_ref(struct fifo* self, struct picotm_error* error);

/**
 * \brief Compares the FIFO's id to an id and acquires a reference if both
 *        id's are equal.
 * \param   self    The FIFO instance.
 * \param   id      The id to compare to.
 * \returns A value less than, equal to, or greater than if the FIFO's id
 *          is less than, equal to, or greater than the given id.
 */
int
fifo_cmp_and_ref(struct fifo* self, const struct file_id* id);

/**
 * \brief Compares the FIFO's id to an id and acquires a reference if both
 *        id's are equal. The FIFO instance is set up from the provided
 *        file descriptor if necessary.
 * \param       self        The FIFO instance.
 * \param       id          The id to compare to.
 * \param       fildes      The FIFO's file descriptor.
 * \param[out]  error       Returns an error ot the caller.
 * \returns A value less than, equal to, or greater than if the ofd's id is
 *          less than, equal to, or greater than the given id.
 */
int
fifo_cmp_and_ref_or_set_up(struct fifo* self, const struct file_id* id,
                           int fildes, struct picotm_error* error);

/**
 * \brief Unreferences a FIFO.
 * \param   self    The FIFO instance.
 */
void
fifo_unref(struct fifo* self);

/**
 * \brief Tries to acquire a reader lock on a FIFO.
 * \param       self        The FIFO instance.
 * \param       field       The reader lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
fifo_try_rdlock_field(struct fifo* self, enum fifo_field field,
                      struct picotm_rwstate* rwstate,
                      struct picotm_error* error);

/**
 * \brief Tries to acquire a writer lock on a FIFO.
 * \param       self        The FIFO instance.
 * \param       field       The writer lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
fifo_try_wrlock_field(struct fifo* self, enum fifo_field field,
                      struct picotm_rwstate* rwstate,
                      struct picotm_error* error);

/**
 * \brief Releases a lock on a character device.
 * \param   self    The FIFO instance.
 * \param   field   The reader/writer lock's field.
 * \param   rwstate The transaction's reader/writer state.
 */
void
fifo_unlock_field(struct fifo* self, enum fifo_field field,
                  struct picotm_rwstate* rwstate);
