/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "mutex.h"

int
mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    int err = pthread_mutex_init(mutex, attr);

    if (err) {
        errno = err;
        perror("pthread_mutex_init");
        return -1;
    }

    return 0;
}

int
mutex_uninit(pthread_mutex_t *mutex)
{
    int err = pthread_mutex_destroy(mutex);

    if (err) {
        errno = err;
        perror("pthread_mutex_destroy");
        return -1;
    }

    return 0;
}

void
mutex_lock(pthread_mutex_t *mutex)
{
    int err = pthread_mutex_lock(mutex);

    if (err) {
        errno = err;
        perror("pthread_mutex_lock");
        abort();
    }
}

void
mutex_unlock(pthread_mutex_t *mutex)
{
    int err = pthread_mutex_unlock(mutex);

    if (err) {
        errno = err;
        perror("pthread_mutex_unlock");
        abort();
    }
}

