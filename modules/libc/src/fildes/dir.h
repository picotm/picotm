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
 * Enumerates fields of `struct dir`.
 */
enum dir_field {
    DIR_FIELD_FILE_MODE,
    DIR_FIELD_STATE,
    NUMBER_OF_DIR_FIELDS
};

/**
 * Represents a directory's open file description.
 */
struct dir {

    /** Reference-counting base object. */
    struct picotm_shared_ref16_obj ref_obj;

    /** The directory's unique id. */
    struct file_id id;

    /** Reader/writer state locks. */
    struct picotm_rwlock rwlock[NUMBER_OF_DIR_FIELDS];
};

/**
 * \brief Initializes a dir instance.
 * \param       self    The dir instance to initialize.
 * \param[out]  error   Returns an error to the caller.
 */
void
dir_init(struct dir* self, struct picotm_error* error);

/**
 * \brief Uninitializes a dir structure.
 * \param   self    The dir instance to uninitialize.
 */
void
dir_uninit(struct dir* self);

/**
 * \brief Sets up an instance of `struct dir` or acquires a reference
 *        on an already set-up instance.
 * \param       self    The dir instance.
 * \param       fildes  The directory's file descriptor.
 * \param[out]  error   Returns an error to the caller.
 */
void
dir_ref_or_set_up(struct dir* self, int fildes, struct picotm_error* error);

/**
 * \brief Acquires a reference on an instance of `struct dir`.
 * \param       self    The dir instance.
 * \param[out]  error   Returns an error to the caller.
 */
void
dir_ref(struct dir* self, struct picotm_error* error);

/**
 * \brief Compares the dir's id to an id and acquires a reference if both
 *        id's are equal.
 * \param   self    The dir instance.
 * \param   id      The id to compare to.
 * \returns A value less than, equal to, or greater than if the dir's id
 *          is less than, equal to, or greater than the given id.
 */
int
dir_cmp_and_ref(struct dir* self, const struct file_id* id);

/**
 * \brief Compares the dir's id to an id and acquires a reference if both
 *        id's are equal. The dir instance is set up from the provided
 *        file descriptor if necessary.
 * \param       self        The dir instance.
 * \param       id          The id to compare to.
 * \param       fildes      The directory's file descriptor.
 * \param[out]  error       Returns an error ot the caller.
 * \returns A value less than, equal to, or greater than if the ofd's id is
 *          less than, equal to, or greater than the given id.
 */
int
dir_cmp_and_ref_or_set_up(struct dir* self, const struct file_id* id,
                          int fildes, struct picotm_error* error);

/**
 * \brief Unreferences a directory.
 * \param   self    The dir instance.
 */
void
dir_unref(struct dir* self);

/**
 * \brief Tries to acquire a reader lock on a directory.
 * \param       self        The dir instance.
 * \param       field       The reader/writer lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
dir_try_rdlock_field(struct dir* self, enum dir_field field,
                     struct picotm_rwstate* rwstate,
                     struct picotm_error* error);

/**
 * \brief Tries to acquire a writer lock on a directory.
 * \param       self        The dir instance.
 * \param       field       The reader/writer lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
dir_try_wrlock_field(struct dir* self, enum dir_field field,
                     struct picotm_rwstate* rwstate,
                     struct picotm_error* error);

/**
 * \brief Releases a lock on a directory.
 * \param   self    The dir instance.
 * \param   field   The reader/writer lock's field.
 * \param   rwstate The transaction's reader/writer state.
 */
void
dir_unlock_field(struct dir* self, enum dir_field field,
                 struct picotm_rwstate* rwstate);
