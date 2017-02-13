/* Copyright (C) 2009  Thomas Zimmermann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

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

