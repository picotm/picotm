/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 */

#include "tx.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm_event.h"
#include "table.h"
#include "tx_shared.h"

/* The maximum number of retries per transaction. If a transacion reaches
 * this limit it switches to irrevocable mode. The actual limit depends on
 * the system's access pattern. The more conflicts, the lower the limit
 * should be. The current value has been choosen arbitrarily and requires
 * further optimization! */
static const unsigned long TX_NRETRIES_LIMIT = 10;

void
tx_init(struct tx* self, struct tx_shared* tx_shared,
        struct picotm_error* error)
{
    assert(self);

    picotm_log_init(&self->log);

    self->env = NULL;
    self->shared = tx_shared;
    self->mode = TX_MODE_REVOCABLE;
    self->nretries = 0;
    self->nmodules = 0;
    self->is_initialized = true;

    picotm_lock_owner_init(&self->lo, error);
    if (picotm_error_is_set(error)) {
        goto err_picotm_lock_owner_init;
    }

    picotm_lock_manager_register_owner(&self->shared->lm, &self->lo, error);
    if (picotm_error_is_set(error)) {
        goto err_lock_manager_register_owner;
    }

    return;

err_lock_manager_register_owner:
    picotm_lock_owner_uninit(&self->lo);
err_picotm_lock_owner_init:
    picotm_log_uninit(&self->log);
}

void
tx_release(struct tx* self)
{
    assert(self);

    picotm_lock_manager_unregister_owner(&self->shared->lm, &self->lo);
    picotm_lock_owner_uninit(&self->lo);
    picotm_log_uninit(&self->log);

    struct picotm_module* module = self->module;
    const struct picotm_module* module_end = self->module + self->nmodules;

    while (module < module_end) {
        picotm_module_uninit(module);
        ++module;
    }
}

bool
tx_is_irrevocable(const struct tx* self)
{
    assert(self);

    return self->mode == TX_MODE_IRREVOCABLE;
}

unsigned long
tx_register_module(struct tx* self,
                   const struct picotm_module_ops* ops, void* data,
                   struct picotm_error* error)
{
    assert(self);

    unsigned long module = self->nmodules;

    if (module >= picotm_arraylen(self->module)) {
        picotm_error_set_error_code(error, PICOTM_OUT_OF_MEMORY);
        return 0;
    }

    picotm_module_init(self->module + module, ops, data);

    self->nmodules = module + 1;

    return module;
}

void
tx_append_event(struct tx* self, unsigned long module, uint16_t head,
                uintptr_t tail, struct picotm_error* error)
{
    assert(self);

    const struct picotm_event event = PICOTM_EVENT_INITIALIZER(module,
                                                               head,
                                                               tail);
    picotm_log_append(&self->log, &event, error);
}

void
tx_begin(struct tx* self, enum tx_mode mode, bool is_retry, jmp_buf* env,
         struct picotm_error* error)
{
    assert(self);

    unsigned long nretries = is_retry ? self->nretries + 1 : 0;

    if (nretries == TX_NRETRIES_LIMIT) {
        mode = TX_MODE_IRREVOCABLE;
    }

    switch (mode) {
        case TX_MODE_REVOCABLE:
            /* If we're not the exclusive transaction, we wait
             * for a possible exclusive transaction to finish. */
            picotm_lock_manager_wait_irrevocable(&self->shared->lm, error);
            break;
        case TX_MODE_IRREVOCABLE:
            /* If we're supposed to run exclusively, we wait
             * for the other transactions to finish. */
            picotm_lock_manager_make_irrevocable(&self->shared->lm, &self->lo,
                                                 error);
            break;
    }
    if (picotm_error_is_set(error)) {
        return;
    }

    /* Reset the timestamp *after* selecting the (ir-)revocability
     * mode. Otherwise the waiting time, and thus the time of a
     * running irrevocable transaction, would be accounted to this
     * transaction as well. */
    picotm_lock_owner_reset_timestamp(&self->lo, error);
    if (picotm_error_is_set(error)) {
        goto err_picotm_lock_owner_reset_timestamp;
    }

    self->nretries = nretries;
    self->mode = mode;
    self->env = env;

    return;

err_picotm_lock_owner_reset_timestamp:
    picotm_lock_manager_release_irrevocability(&self->shared->lm);
}

static size_t
lock_cb(void* module, struct picotm_error* error)
{
    picotm_module_lock(module, error);
    return picotm_error_is_set(error) ? 0 : 1;
}

static void
lock_modules(struct picotm_module* module, unsigned long nmodules,
             struct picotm_error* error)
{
    tabwalk_1(module, nmodules, sizeof(*module), lock_cb, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static size_t
unlock_cb(void* module, struct picotm_error* error)
{
    picotm_module_unlock(module, error);
    return picotm_error_is_set(error) ? 0 : 1;
}

static void
unlock_modules(struct picotm_module* module, unsigned long nmodules,
               struct picotm_error* error)
{
    tabrwalk_1(module, nmodules, sizeof(*module), unlock_cb, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static size_t
validate_cb(void* module, void* is_irrevocable, struct picotm_error* error)
{
    assert(is_irrevocable);

    picotm_module_validate(module, *((bool*)is_irrevocable), error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static void
validate_modules(struct picotm_module* module, unsigned long nmodules,
                 bool is_irrevocable, struct picotm_error* error)
{
    tabwalk_2(module, nmodules, sizeof(*module), validate_cb, &is_irrevocable,
              error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static size_t
apply_cb(void* module, struct picotm_error* error)
{
    picotm_module_apply(module, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static void
apply_modules(struct picotm_module* module, unsigned long nmodules,
              struct picotm_error* error)
{
    tabwalk_1(module, nmodules, sizeof(*module), apply_cb, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static size_t
undo_cb(void* module, struct picotm_error* error)
{
    picotm_module_undo(module, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static void
undo_modules(struct picotm_module* module, unsigned long nmodules,
             struct picotm_error* error)
{
    tabwalk_1(module, nmodules, sizeof(*module), undo_cb, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static size_t
update_cc_cb(void* module, void* is_irrevocable, struct picotm_error* error)
{
    assert(is_irrevocable);

    picotm_module_update_cc(module, *((bool*)is_irrevocable), error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static void
update_modules_cc(struct picotm_module* module, unsigned long nmodules,
                  bool is_irrevocable, struct picotm_error* error)
{
    tabwalk_2(module, nmodules, sizeof(*module), update_cc_cb,
              &is_irrevocable, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static size_t
clear_cc_cb(void* module, void* is_irrevocable, struct picotm_error* error)
{
    assert(is_irrevocable);

    picotm_module_clear_cc(module, *((bool*)is_irrevocable), error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static void
clear_modules_cc(struct picotm_module* module, unsigned long nmodules,
                 bool is_irrevocable, struct picotm_error* error)
{
    tabwalk_2(module, nmodules, sizeof(*module), clear_cc_cb, &is_irrevocable,
              error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static size_t
log_finish_cb(void* module, struct picotm_error* error)
{
    picotm_module_finish(module, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static void
finish_modules(struct picotm_module* module, unsigned long nmodules,
               struct picotm_error* error)
{
    tabwalk_1(module, nmodules, sizeof(*module), log_finish_cb, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
apply_event(struct tx* tx, const struct picotm_event* event,
            struct picotm_error* error)
{
    picotm_module_apply_event(tx->module + event->module,
                              event->head, event->tail,
                              error);
}

static void
apply_event_cb(struct picotm_event* event, void* data,
               struct picotm_error* error)
{
    apply_event(data, event, error);
}

static void
apply_events(struct tx* self, struct picotm_error* error)
{
    picotm_events_foreach1(picotm_log_begin(&self->log),
                           picotm_log_end(&self->log),
                           self, apply_event_cb, error);
    picotm_log_clear(&self->log);
}

static void
undo_event(struct tx* tx, const struct picotm_event* event,
           struct picotm_error* error)
{
    picotm_module_undo_event(tx->module + event->module,
                             event->head, event->tail,
                             error);
}

static void
undo_event_cb(struct picotm_event* event, void* data,
              struct picotm_error* error)
{
    undo_event(data, event, error);
}

static void
undo_events(struct tx* self, struct picotm_error* error)
{
    picotm_events_rev_foreach1(picotm_log_begin(&self->log),
                               picotm_log_end(&self->log),
                               self, undo_event_cb, error);
    picotm_log_clear(&self->log);
}

void
tx_commit(struct tx* self, struct picotm_error* error)
{
    assert(self);

    bool is_irrevocable = tx_is_irrevocable(self);
    bool is_non_recoverable = false;

    lock_modules(self->module, self->nmodules, error);
    if (picotm_error_is_set(error)) {
        goto err_lock_modules;
    }

    validate_modules(self->module, self->nmodules, is_irrevocable, error);
    if (picotm_error_is_set(error)) {
        goto err_validate_modules;
    }

    /* Point of no return! After we began to apply events, we cannot
     * roll-back or even recover from errors.
     */

    is_non_recoverable = true;

    apply_modules(self->module, self->nmodules, error);
    if (picotm_error_is_set(error)) {
        goto err_apply_modules;
    }

    apply_events(self, error);
    if (picotm_error_is_set(error)) {
        goto err_apply_events;
    }

    update_modules_cc(self->module, self->nmodules, is_irrevocable, error);
    if (picotm_error_is_set(error)) {
        goto err_update_modules_cc;
    }

    unlock_modules(self->module, self->nmodules, error);
    if (picotm_error_is_set(error)) {
        goto err;
    }

    finish_modules(self->module, self->nmodules, error);
    if (picotm_error_is_set(error)) {
        goto err;
    }

    picotm_lock_manager_release_irrevocability(&self->shared->lm);

    return;

err_update_modules_cc:
err_apply_events:
err_apply_modules:
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
}

void
tx_rollback(struct tx* self, struct picotm_error* error)
{
    assert(self);

    bool is_irrevocable = tx_is_irrevocable(self);

    undo_modules(self->module, self->nmodules, error);
    if (picotm_error_is_set(error)) {
        goto err;
    }

    undo_events(self, error);
    if (picotm_error_is_set(error)) {
        goto err;
    }

    clear_modules_cc(self->module, self->nmodules, is_irrevocable, error);
    if (picotm_error_is_set(error)) {
        goto err;
    }

    finish_modules(self->module, self->nmodules, error);
    if (picotm_error_is_set(error)) {
        goto err;
    }

    picotm_lock_manager_release_irrevocability(&self->shared->lm);

    return;

err:
    picotm_error_mark_as_non_recoverable(error);
    picotm_lock_manager_release_irrevocability(&self->shared->lm);
}

bool
tx_is_valid(struct tx* self)
{
    assert(self);

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    validate_modules(self->module, self->nmodules, tx_is_irrevocable(self),
                     &error);
    if (picotm_error_is_set(&error)) {
        return false;
    }
    return true;
}
