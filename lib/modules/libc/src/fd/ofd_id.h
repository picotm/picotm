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

#include "fileid.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

/**
 * \brief The unique id of an open file description.
 *
 * By default, an open file description does not have an id by itself.
 * In picotm, we construct the id from the file id of the open file
 * description's file and a file descriptor that refers to the open
 * file description.
 *
 * If two file descriptors refer to the same open file description,
 * they create two different ids. Even though the open file description
 * is the same, these ids must compare as *different*. The look-up code
 * fixes this ad-hoc. It's a short coming resulting fom the non-existance
 * of unique system ids for open file descriptions.
 */
struct ofd_id {
    int fildes;
    struct file_id file_id;
};

/**
 * \brief Initializes an open-file-description id.
 */
void
ofd_id_init(struct ofd_id* self);

/**
 * \brief Initializes an open-file-description id from a file descriptor.
 * \param   self        An open-file-description id.
 * \param   fildes      The open file description's file descriptor.
 * \param[out]  error   Returns an error to the caller.
 */
void
ofd_id_init_from_fildes(struct ofd_id* self, int fildes,
                        struct picotm_error* error);

/**
 * \brief Uninitializes an open-file-description id.
 * \param self  An open-file-descriptinon id.
 */
void
ofd_id_uninit(struct ofd_id* self);

/**
 * \brief Sets an open-file-description id from a file descriptor.
 * \param       self    An open-file-description id.
 * \param       fildes  The open file description's file descriptor.
 * \param[out]  error   Returns an error to the caller.
 */
void
ofd_id_set_from_fildes(struct ofd_id* self, int fildes,
                       struct picotm_error* error);

/**
 * \brief Clears an open-file-description id to an empty state.
 * \param   self    An open-file-description id.
 */
void
ofd_id_clear(struct ofd_id* self);

/**
 * \brief Compares two open-file-description ids.
 * \param   lhs An open-file-description id.
 * \param   rhs An open-file-description id.
 * \returns A value less than, equal to, or greater than 0 if lhs is
 *          less than, equal to, or greater than rhs.
 */
int
ofd_id_cmp(const struct ofd_id* lhs, const struct ofd_id* rhs);
