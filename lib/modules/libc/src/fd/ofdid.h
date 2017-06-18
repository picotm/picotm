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

#ifndef OFDID_H
#define OFDID_H

#include "fileid.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

struct ofdid {
    struct file_id fbuf_id;

    mode_t mode;
    int    fl;
};

/** \brief Initialize id with values */
void
ofdid_init(struct ofdid *id, dev_t dev, ino_t ino, mode_t mode, int fl);

/** \brief initialize id from file descriptor */
void
ofdid_init_from_fildes(struct ofdid *id, int fildes,
                       struct picotm_error* error);

/** \brief Clear file descriptor */
void
ofdid_clear(struct ofdid *id);

/** \brief Return non-zero value id id has been initialized */
int
ofdid_is_empty(const struct ofdid *id);

/** Compare two ids, return value as with strcmp */
int
ofdidcmp(const struct ofdid *id1, const struct ofdid *id2);

/** Dump id to stderr */
void
ofdid_print(const struct ofdid *id);

#endif

