/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MUTEX_H
#define MUTEX_H

/**
 * \cond impl || libc_impl
 * \ingroup libc_impl
 * \file
 * \endcond
 */

int
mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);

int
mutex_uninit(pthread_mutex_t *mutex);

void
mutex_lock(pthread_mutex_t *mutex);

void
mutex_unlock(pthread_mutex_t *mutex);

#endif

