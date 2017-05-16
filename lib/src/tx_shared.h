/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <pthread.h>

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

struct tx;

struct tx_shared {
    /**
     * Locks the transaction system for either one exclusive transactions,
     * or mutliple non-exclusive transactions.
     *
     * \todo This lock implements irrevocability. Replace this lock with a
     * more fine-grained system that allows recovable transactions while an
     * irrevocable transaction is present.
     */
    pthread_rwlock_t exclusive_tx_lock;
    struct tx*       exclusive_tx;
};

int
tx_shared_init(struct tx_shared* self);

void
tx_shared_uninit(struct tx_shared* self);

int
tx_shared_make_irrevocable(struct tx_shared* self, struct tx* exclusive_tx);

int
tx_shared_wait_irrevocable(struct tx_shared* self);

void
tx_shared_release_irrevocability(struct tx_shared* self);
