/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <picotm/compiler.h>
#include <picotm/picotm-tm.h>
#include <pthread.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libpthread
 * \file
 *
 * \brief Transactional wrappers for interfaces of <pthread.h>.
 */

PICOTM_TM_LOAD_TX(pthread_t, pthread_t);

PICOTM_TM_STORE_TX(pthread_t, pthread_t);

PICOTM_TM_PRIVATIZE_TX(pthread_t, pthread_t);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [pthread_equal()][posix::pthread_equal].
 *
 * [posix::pthread_equal]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_equal.html
 */
int
pthread_equal_tx(pthread_t t1, pthread_t t2);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [pthread_self()][posix::pthread_self].
 *
 * [posix::pthread_self]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_self.html
 */
pthread_t
pthread_self_tx(void);

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
