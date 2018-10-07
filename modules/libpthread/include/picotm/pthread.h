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
/** \addtogroup group_libpthread
 * \{ */
PICOTM_TM_LOAD_TX(pthread_t, pthread_t);
PICOTM_TM_STORE_TX(pthread_t, pthread_t);
PICOTM_TM_PRIVATIZE_TX(pthread_t, pthread_t);
/** \} */
#endif

#if defined(PICOTM_LIBPTHREAD_HAVE_PTHREAD_EQUAL) && \
        PICOTM_LIBPTHREAD_HAVE_PTHREAD_EQUAL || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libpthread
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
 * \ingroup group_libpthread
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
