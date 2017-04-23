/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <picotm/compiler.h>
#include <picotm/picotm-tm.h>
#include <pthread.h>

PICOTM_BEGIN_DECLS

PICOTM_TM_LOAD_TX(pthread_t, pthread_t);

PICOTM_TM_STORE_TX(pthread_t, pthread_t);

PICOTM_TM_PRIVATIZE_TX(pthread_t, pthread_t);

PICOTM_NOTHROW
/**
 * Executes pthread_equal() within a transaction.
 */
int
pthread_equal_tx(pthread_t t1, pthread_t t2);

PICOTM_NOTHROW
/**
 * Executes pthread_self() within a transaction.
 */
pthread_t
pthread_self_tx(void);

PICOTM_END_DECLS
