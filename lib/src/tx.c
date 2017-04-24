/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tx.h"
#include <errno.h>
#include <stdlib.h>
#include "ptr.h"
#include "table.h"
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
    self->nmodules = 0;
    self->is_initialized = true;

    return 0;
}

void
tx_release(struct tx* self)
{
    log_uninit(&self->log);

    struct component* com = self->com;
    const struct component* com_end = self->com + self->nmodules;

    while (com < com_end) {
        component_uninit(com);
        ++com;
    }
}

bool
tx_is_irrevocable(const struct tx* self)
{
    return self->mode == TX_MODE_IRREVOCABLE;
}

long
tx_register_module(struct tx* self,
                   int (*lock)(void*),
                   int (*unlock)(void*),
                   int (*validate)(void*, int),
                   int (*apply_event)(const struct event*, size_t, void*),
                   int (*undo_event)(const struct event*, size_t, void*),
                   int (*updatecc)(void*, int),
                   int (*clearcc)(void*, int),
                   int (*finish)(void*),
                   int (*uninit)(void*),
                   void* data)
{
    if (self->nmodules >= arraylen(self->com)) {
        return -ENOMEM;
    }

    long module = self->nmodules;

    int res = component_init(self->com + module, lock, unlock, validate,
                             apply_event, undo_event, updatecc, clearcc,
                             finish, uninit, data);
    if (res < 0) {
        return res;
    }

    self->nmodules = module + 1;

    return module;
}

int
tx_inject_event(struct tx* self, unsigned long module, unsigned long op,
                uintptr_t cookie)
{
    int res = log_inject_event(&self->log, module, op, cookie);
    if (res < 0) {
        return res;
    }
    return 0;
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

static int
lock_cb(void* com)
{
    int res = component_lock(com);
    return res < 0 ? res : 1;
}

static int
lock_components(struct component* com, unsigned long ncomponents)
{
    int res = tabwalk_1(com, ncomponents, sizeof(*com), lock_cb);
    if (res < 0) {
        return res;
    }
    return 0;
}

static int
unlock_cb(void* com)
{
    int res = component_unlock(com);
    return res < 0 ? res : 1;
}

static void
unlock_components(struct component* com, unsigned long ncomponents)
{
    int res = tabrwalk_1(com, ncomponents, sizeof(*com), unlock_cb);
    if (res) {
        /* TODO: may never fail */
        abort();
    }
}

static int
validate_components(struct component* com, unsigned long ncomponents,
                    bool is_irrevocable)
{
    int err = 0;

    const struct component* com_end = com + ncomponents;

    while (com < com_end) {
        err = component_validate(com, is_irrevocable);
        if (err < 0) {
            break;
        }
        ++com;
    }

    return err;
}

static int
update_components_cc(struct component* com, unsigned long ncomponents,
                     bool is_irrevocable)
{
    int err = 0;

    const struct component* com_end = com + ncomponents;

    while (com < com_end) {
        err = component_updatecc(com, is_irrevocable);
        if (err) {
            break;
        }
        ++com;
    }

    return err;
}

static int
clear_components_cc(struct component* com, unsigned long ncomponents,
                    bool is_irrevocable)
{
    int err = 0;

    const struct component* com_end = com + ncomponents;

    while (com < com_end) {
        err = component_clearcc(com, is_irrevocable);
        if (err) {
            break;
        }
        ++com;
    }

    return err;
}

static int
log_finish_cb_walk(void* com)
{
    return !component_finish(com) ? 1 : -1;
}

static void
finish_components(struct component* com, unsigned long ncomponents)
{
    int res = tabwalk_1(com, ncomponents, sizeof(*com), log_finish_cb_walk);
    if (res) {
        /* TODO: may never fail */
        abort();
    }
}

int
tx_commit(struct tx* self)
{
    int res = lock_components(self->com, self->nmodules);
    if (res < 0) {
        goto err_lock_components;
    }

    res = validate_components(self->com, self->nmodules,
                              tx_is_irrevocable(self));
    if (res < 0) {
        goto err_validate_components;
    }

    res = log_apply_events(&self->log, self->com, tx_is_irrevocable(self));
    if (res < 0) {
        goto err_log_apply_events;
    }

    res = update_components_cc(self->com, self->nmodules,
                               tx_is_irrevocable(self));
    if (res < 0) {
        goto err_update_components_cc;
    }

    unlock_components(self->com, self->nmodules);
    finish_components(self->com, self->nmodules);

    tx_shared_release_irrevocability(self->shared);

    return 0;

err_update_components_cc:
err_log_apply_events:
err_validate_components:
    unlock_components(self->com, self->nmodules);
err_lock_components:
    tx_shared_release_irrevocability(self->shared);
    return res;
}

int
tx_rollback(struct tx* self)
{
    log_undo_events(&self->log, self->com, tx_is_irrevocable(self));

    clear_components_cc(self->com, self->nmodules, tx_is_irrevocable(self));
    finish_components(self->com, self->nmodules);

    tx_shared_release_irrevocability(self->shared);

    return 0;
}

bool
tx_is_valid(struct tx* self)
{
    int res = validate_components(self->com, self->nmodules,
                                  tx_is_irrevocable(self));
    if (res < 0) {
        return false;
    }
    return true;
}
