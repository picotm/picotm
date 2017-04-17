/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tx.h"
#include "tx_shared.h"

int
tx_init(struct tx* self, struct tx_shared* tx_shared)
{
    int res = log_init(&self->log);
    if (res < 0) {
        return res;
    }

    self->shared = tx_shared;
    self->mode = TX_MODE_REVOCABLE;
    self->is_initialized = true;

    return 0;
}

void
tx_release(struct tx* self)
{
    log_uninit(&self->log);
}

struct log*
tx_log(struct tx* self)
{
    return &self->log;
}

bool
tx_is_irrevocable(const struct tx* self)
{
    return self->mode == TX_MODE_IRREVOCABLE;
}

int
tx_begin(struct tx* self, enum tx_mode mode)
{
    int res;

    switch (mode) {
        case TX_MODE_IRREVOCABLE:
            /* If we're supposed to run exclusively, we wait
             * for the other transactions to finish. */
            res = tx_shared_make_irrevocable(self->shared, self);
            break;
        default:
            /* If we're not the exclusive transaction, we wait
             * for a possible exclusive transaction to finish. */
            res = tx_shared_wait_irrevocable(self->shared);
            break;
    }
    if (res < 0) {
        return res;
    }

    self->mode = mode;

    return 0;
}

int
tx_commit(struct tx* self)
{
    int res = log_lock(&self->log);
    if (res < 0) {
        goto err_log_lock;
    }

    res = log_validate(&self->log, true, tx_is_irrevocable(self));
    if (res < 0) {
        goto err_log_validate;
    }

    res = log_apply_events(&self->log, tx_is_irrevocable(self));
    if (res < 0) {
        goto err_log_apply_events;
    }

    res = log_updatecc(&self->log, tx_is_irrevocable(self));
    if (res < 0) {
        goto err_log_updatecc;
    }

    res = log_unlock(&self->log);
    if (res < 0) {
        goto err_log_unlock;
    }

    res = log_finish(&self->log);
    if (res < 0) {
        goto err_log_finish;
    }

    tx_shared_release_irrevocability(self->shared);

    return 0;

err_log_finish:
err_log_unlock:
err_log_updatecc:
err_log_apply_events:
err_log_validate:
    log_unlock(&self->log);
err_log_lock:
    tx_shared_release_irrevocability(self->shared);
    return res;
}

int
tx_rollback(struct tx* self)
{
    int res = log_lock(&self->log);
    if (res < 0) {
        return res;
    }

    res = log_undo_events(&self->log, tx_is_irrevocable(self));
    if (res < 0) {
        goto err_log_undo_events;
    }

    res = log_clearcc(&self->log, tx_is_irrevocable(self));
    if (res < 0) {
        goto err_log_clearcc;
    }

    res = log_unlock(&self->log);
    if (res < 0) {
        goto err_log_unlock;
    }

    res = log_finish(&self->log);
    if (res < 0) {
        goto err_log_finish;
    }

    tx_shared_release_irrevocability(self->shared);

    return 0;

err_log_finish:
err_log_unlock:
err_log_clearcc:
err_log_undo_events:
    log_unlock(&self->log);
    return res;
}

bool
tx_is_valid(struct tx* self)
{
    int res = log_validate(&self->log, false, tx_is_irrevocable(self));
    if (res < 0) {
        return false;
    }
    return true;
}
