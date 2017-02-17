/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tx.h"

int
tx_init(struct tx* self)
{
    int res = log_init(&self->log);
    if (res < 0) {
        return res;
    }

    self->mode = SYSTX_MODE_START;
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
    return self->mode == SYSTX_MODE_IRREVOCABLE;
}

int
tx_begin(struct tx* self, enum __systx_mode mode)
{
    self->mode = mode;

    return 0;
}

int
tx_commit(struct tx* self)
{
    int res = log_lock(&self->log);
    if (res < 0) {
        return res;
    }

    res = log_validate(&self->log, true, tx_is_irrevocable(self));
    if (res < 0) {
        goto err_log_validate;
    }

    res = log_apply_events(&self->log, tx_is_irrevocable(self));
    if (res < 0) {
        goto err_log_apply_events;
    }

    res = log_unlock(&self->log);
    if (res < 0) {
        goto err_log_unlock;
    }

    res = log_finish(&self->log);
    if (res < 0) {
        goto err_log_finish;
    }

    return 0;

err_log_finish:
err_log_unlock:
err_log_apply_events:
err_log_validate:
    log_unlock(&self->log);
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

    res = log_unlock(&self->log);
    if (res < 0) {
        goto err_log_unlock;
    }

    res = log_finish(&self->log);
    if (res < 0) {
        goto err_log_finish;
    }

    return 0;

err_log_finish:
err_log_unlock:
err_log_undo_events:
    log_unlock(&self->log);
    return res;
}
