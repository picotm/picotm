/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tx.h"
#include <errno.h>
#include <picotm/picotm-error.h>
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

    struct module* module = self->module;
    const struct module* module_end = self->module + self->nmodules;

    while (module < module_end) {
        module_uninit(module);
        ++module;
    }
}

bool
tx_is_irrevocable(const struct tx* self)
{
    return self->mode == TX_MODE_IRREVOCABLE;
}

long
tx_register_module(struct tx* self,
                   int (*lock)(void*, struct picotm_error*),
                   int (*unlock)(void*, struct picotm_error*),
                   int (*validate)(void*, int, struct picotm_error*),
                   int (*apply_event)(const struct event*, size_t, void*, struct picotm_error*),
                   int (*undo_event)(const struct event*, size_t, void*, struct picotm_error*),
                   int (*updatecc)(void*, int, struct picotm_error*),
                   int (*clearcc)(void*, int, struct picotm_error*),
                   int (*finish)(void*, struct picotm_error*),
                   void (*uninit)(void*),
                   void* data)
{
    if (self->nmodules >= arraylen(self->module)) {
        return -ENOMEM;
    }

    long module = self->nmodules;

    int res = module_init(self->module + module, lock, unlock, validate,
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
lock_cb(void* module, void* error)
{
    int res = module_lock(module, error);
    return res < 0 ? -1 : 1;
}

static int
lock_modules(struct module* module, unsigned long nmodules,
             struct picotm_error* error)
{
    int res = tabwalk_2(module, nmodules, sizeof(*module), lock_cb, error);
    if (res < 0) {
        return -1;
    }
    return 0;
}

static int
unlock_cb(void* module, void* error)
{
    int res = module_unlock(module, error);
    return res < 0 ? -1 : 1;
}

static int
unlock_modules(struct module* module, unsigned long nmodules,
               struct picotm_error* error)
{
    int res = tabrwalk_2(module, nmodules, sizeof(*module), unlock_cb, error);
    if (res < 0) {
        return -1;
    }
    return 0;
}

static int
validate_cb(void* module, void* is_irrevocable, void* error)
{
    int res = module_validate(module, *((bool*)is_irrevocable), error);
    return res < 0 ? -1 : 1;
}

static int
validate_modules(struct module* module, unsigned long nmodules,
                 bool is_irrevocable, struct picotm_error* error)
{
    int res = tabwalk_3(module, nmodules, sizeof(*module), validate_cb,
                        &is_irrevocable, error);
    if (res < 0) {
        return -1;
    }
    return 0;
}

static int
update_cc_cb(void* module, void* is_irrevocable, void* error)
{
    int res = module_update_cc(module, *((bool*)is_irrevocable), error);
    return res < 0 ? -1 : 1;
}

static int
update_modules_cc(struct module* module, unsigned long nmodules,
                  bool is_irrevocable, struct picotm_error* error)
{
    int res = tabwalk_3(module, nmodules, sizeof(*module), update_cc_cb,
                        &is_irrevocable, error);
    if (res < 0) {
        return -1;
    }
    return 0;
}

static int
clear_cc_cb(void* module, void* is_irrevocable, void* error)
{
    int res = module_clear_cc(module, *((bool*)is_irrevocable), error);
    return res < 0 ? -1 : 1;
}

static int
clear_modules_cc(struct module* module, unsigned long nmodules,
                 bool is_irrevocable, struct picotm_error* error)
{
    int res = tabwalk_3(module, nmodules, sizeof(*module), clear_cc_cb,
                        &is_irrevocable, error);
    if (res < 0) {
        return -1;
    }
    return 0;
}

static int
log_finish_cb(void* module, void* error)
{
    int res = module_finish(module, error);
    return res < 0 ? -1 : 1;
}

static int
finish_modules(struct module* module, unsigned long nmodules,
               struct picotm_error* error)
{
    int res = tabwalk_2(module, nmodules, sizeof(*module), log_finish_cb,
                        error);
    if (res < 0) {
        return -1;
    }
    return 0;
}

int
tx_commit(struct tx* self, struct picotm_error* error)
{
    bool is_irrevocable = tx_is_irrevocable(self);
    bool is_non_recoverable = false;

    int res = lock_modules(self->module, self->nmodules, error);
    if (res < 0) {
        goto err_lock_modules;
    }

    res = validate_modules(self->module, self->nmodules, is_irrevocable,
                           error);
    if (res < 0) {
        goto err_validate_modules;
    }

    /* Point of no return! After we began to apply events, we cannot
     * roll-back or even recover from errors.
     */

    is_non_recoverable = true;

    res = log_apply_events(&self->log, self->module, is_irrevocable, error);
    if (res < 0) {
        goto err_log_apply_events;
    }

    res = update_modules_cc(self->module, self->nmodules, is_irrevocable,
                            error);
    if (res < 0) {
        goto err_update_modules_cc;
    }

    res = unlock_modules(self->module, self->nmodules, error);
    if (res < 0) {
        goto err;
    }

    res = finish_modules(self->module, self->nmodules, error);
    if (res < 0) {
        goto err;
    }

    tx_shared_release_irrevocability(self->shared);

    return 0;

err_update_modules_cc:
err_log_apply_events:
err_validate_modules:
    {
        struct picotm_error err_error = PICOTM_ERROR_INITIALIZER;
        unlock_modules(self->module, self->nmodules, &err_error);
    }
err_lock_modules:
err:
    if (is_non_recoverable) {
        picotm_error_mark_as_non_recoverable(error);
    }
    tx_shared_release_irrevocability(self->shared);
    return -1;
}

int
tx_rollback(struct tx* self, struct picotm_error* error)
{
    bool is_irrevocable = tx_is_irrevocable(self);

    int res = log_undo_events(&self->log, self->module, is_irrevocable,
                              error);
    if (res < 0) {
        goto err;
    }

    res = clear_modules_cc(self->module, self->nmodules, is_irrevocable,
                           error);
    if (res < 0) {
        goto err;
    }

    res = finish_modules(self->module, self->nmodules, error);
    if (res < 0) {
        goto err;
    }

    tx_shared_release_irrevocability(self->shared);

    return 0;

err:
    picotm_error_mark_as_non_recoverable(error);
    tx_shared_release_irrevocability(self->shared);
    return -1;
}

bool
tx_is_valid(struct tx* self)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    int res = validate_modules(self->module, self->nmodules,
                               tx_is_irrevocable(self), &error);
    if (res < 0) {
        return false;
    }
    return true;
}
