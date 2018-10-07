/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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

#include <sys/types.h>

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct ioop {
    off_t  off;
    size_t nbyte;
    size_t bufoff;
};

void
ioop_init(struct ioop* self, off_t offset, size_t nbyte, size_t bufoff);

void
ioop_uninit(struct ioop* self);

/**
 * Reads an ioop into a buffer.
 * \param   self    The I/O operation.
 * \param   buf     The output buffer.
 * \param   nbyte   The output-buffer length.
 * \param   offset  The read offset.
 * \param   iobuf   The ioop data buffer, can be NULL if no data has been written.
 * \returns The number of bytes read.
 */
ssize_t
ioop_read(const struct ioop* self, void* buf, size_t nbyte, off_t offset,
          const void* iobuf);
