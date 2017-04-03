/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tx_shared.h"

int
tx_shared_init(struct tx_shared* self)
{
    int res = pthread_rwlock_init(&self->exclusive_tx_lock, NULL);
    if (res) {
        return -res;
    }
    self->exclusive_tx = NULL;
    return 0;
}

void
tx_shared_uninit(struct tx_shared* self)
{
    pthread_rwlock_destroy(&self->exclusive_tx_lock);
}

int
tx_shared_make_irrevocable(struct tx_shared* self, struct tx* exclusive_tx)
{
    int res = pthread_rwlock_wrlock(&self->exclusive_tx_lock);
    if (res) {
        return -res;
    }
    self->exclusive_tx = exclusive_tx;
    return 0;
}

int
tx_shared_wait_irrevocable(struct tx_shared* self)
{
    int res = pthread_rwlock_rdlock(&self->exclusive_tx_lock);
    if (res) {
        return -res;
    }
    return 0;
}

void
tx_shared_release_irrevocability(struct tx_shared* self)
{
    self->exclusive_tx = NULL;
    pthread_rwlock_unlock(&self->exclusive_tx_lock);
}
