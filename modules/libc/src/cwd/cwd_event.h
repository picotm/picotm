/*
 * MIT License
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
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

/**
 * \cond impl || libc_impl || libc_impl_cwd
 * \ingroup libc_impl
 * \ingroup libc_impl_cwd
 * \file
 * \endcond
 */

/**
 * \brief Opcodes for CWD events.
 */
enum cwd_op {
    /** \brief Represents a chdir() operation. */
    CWD_OP_CHDIR,
    /** \brief Represents a getcwd() operation. */
    CWD_OP_GETCWD,
    /** \brief Represents a realpath() operation. */
    CWD_OP_REALPATH,
    /** \brief The number of CWD operations. */
    LAST_CWD_OP
};

/**
 * \brief Represents a CWD event.
 */
struct cwd_event {
    char* alloced;
};
