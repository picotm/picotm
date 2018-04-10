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

#include "picotm/config/picotm-libpthread-config.h"
#include "picotm/compiler.h"
#include "picotm/picotm-tm.h"
#include <pthread.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libpthread
 * \file
 *
 * \brief Transactional wrappers for interfaces of <pthread.h>.
 */

#if defined(PICOTM_LIBPTHREAD_HAVE_TYPE_PTHREAD_T) && \
            PICOTM_LIBPTHREAD_HAVE_TYPE_PTHREAD_T || \
    defined(__PICOTM_DOXYGEN)
PICOTM_TM_LOAD_TX(pthread_t, pthread_t);
PICOTM_TM_STORE_TX(pthread_t, pthread_t);
PICOTM_TM_PRIVATIZE_TX(pthread_t, pthread_t);
#endif

#if defined(PICOTM_LIBPTHREAD_HAVE_PTHREAD_EQUAL) && \
        PICOTM_LIBPTHREAD_HAVE_PTHREAD_EQUAL || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [pthread_equal()][posix::pthread_equal].
 *
 * [posix::pthread_equal]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_equal.html
 */
int
pthread_equal_tx(pthread_t t1, pthread_t t2);
#endif

#if defined(PICOTM_LIBPTHREAD_HAVE_PTHREAD_SELF) && \
        PICOTM_LIBPTHREAD_HAVE_PTHREAD_SELF || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [pthread_self()][posix::pthread_self].
 *
 * [posix::pthread_self]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_self.html
 */
pthread_t
pthread_self_tx(void);
#endif

PICOTM_END_DECLS

/**
 * \defgroup group_libpthread The POSIX Threads module.
 *
 * \brief Covers the POSIX Threads interface.
 *
 * There's currently only minimal support for POSIX threads. Most of the
 * POSIX threads interface's functionality requires irrevocability and is
 * probably not useful for transactional code.
 *
 * There are currently two supported interface. Call pthread_self_tx() to
 * get a reference to the thread's structure, and use pthread_equal_tx()
 * to compare thread structure with each other.
 */
