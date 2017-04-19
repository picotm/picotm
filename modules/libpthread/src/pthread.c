/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/pthread.h"

int
pthread_equal_tx(pthread_t t1, pthread_t t2)
{
    return pthread_equal(t1, t2);
}

pthread_t
pthread_self_tx()
{
    return pthread_self();
}
