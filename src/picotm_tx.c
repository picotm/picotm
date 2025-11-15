/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2019  Thomas Zimmermann
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "picotm_tx.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm_event.h"
#include "picotm_lock_manager.h"
#include "table.h"

/* The maximum number of retries per transaction. If a transacion reaches
 * this limit it switches to irrevocable mode. The actual limit depends on
 * the system's access pattern. The more conflicts, the lower the limit
 * should be. The current value has been choosen arbitrarily and requires
 * further optimization! */
static const unsigned long TX_NRETRIES_LIMIT = 10;

void
picotm_tx_init(struct picotm_tx* self, struct picotm_lock_manager* lm,
               struct picotm_error* error)
{
    assert(self);
    assert(lm);

    picotm_log_init(&self->log);

    self->env = NULL;
    self->mode = TX_MODE_REVOCABLE;
    self->nretries = 0;
    self->nmodules = 0;

    self->lm = lm;

    picotm_lock_owner_init(&self->lo, error);
    if (picotm_error_is_set(error)) {
        goto err_picotm_lock_owner_init;
    }

    picotm_lock_manager_register_owner(self->lm, &self->lo, error);
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
picotm_tx_release(struct picotm_tx* self)
{
    assert(self);

    picotm_lock_manager_unregister_owner(self->lm, &self->lo);
    picotm_lock_owner_uninit(&self->lo);
    picotm_log_uninit(&self->log);

    struct picotm_module* module = self->module;
    const struct picotm_module* module_end = self->module + self->nmodules;

    while (module < module_end) {
        picotm_module_release(module);
        ++module;
    }
}

bool
picotm_tx_is_irrevocable(const struct picotm_tx* self)
{
    assert(self);

    return self->mode == TX_MODE_IRREVOCABLE;
}

unsigned long
picotm_tx_register_module(struct picotm_tx* self,
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
picotm_tx_append_event(struct picotm_tx* self, unsigned long module,
                       uint16_t head, uintptr_t tail,
                       struct picotm_error* error)
{
    assert(self);

    const struct picotm_event event = PICOTM_EVENT_INITIALIZER(module,
                                                               head,
                                                               tail);
    picotm_log_append(&self->log, &event, error);
}

static size_t
begin_cb(void* module, struct picotm_error* error)
{
    picotm_module_begin(module, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static size_t
begin_modules(struct picotm_module* module, unsigned long nmodules,
              struct picotm_error* error)
{
    size_t nwalked = tabwalk_1(module, nmodules, sizeof(*module), begin_cb,
                               error);
    if (picotm_error_is_set(error)) {
        return nwalked;
    }
    return nwalked;
}

static size_t
finish_cb(void* module, struct picotm_error* error)
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
    tabwalk_1(module, nmodules, sizeof(*module), finish_cb, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static inline bool
log_is_empty(struct picotm_tx* self)
{
    return picotm_log_begin(&self->log) == picotm_log_end(&self->log);
}

void
picotm_tx_begin(struct picotm_tx* self, enum picotm_tx_mode mode,
                bool is_retry, __picotm_jmp_buf* env,
                struct picotm_error* error)
{
    assert(self);
    assert(log_is_empty(self));

    unsigned long nretries = is_retry ? self->nretries + 1 : 0;

    if (nretries == TX_NRETRIES_LIMIT) {
        mode = TX_MODE_IRREVOCABLE;
    }

    switch (mode) {
        case TX_MODE_REVOCABLE:
            /* If we're not the exclusive transaction, we wait
             * for a possible exclusive transaction to finish. */
            picotm_lock_manager_wait_irrevocable(self->lm, error);
            break;
        case TX_MODE_IRREVOCABLE:
            /* If we're supposed to run exclusively, we wait
             * for the other transactions to finish. */
            picotm_lock_manager_make_irrevocable(self->lm, &self->lo, error);
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

    size_t nbegin = begin_modules(self->module, self->nmodules, error);
    if (picotm_error_is_set(error)) {
        goto err_begin_modules;
    }

    return;

err_begin_modules: {
        struct picotm_error err_error = PICOTM_ERROR_INITIALIZER;
        finish_modules(self->module, nbegin, &err_error);
        if (picotm_error_is_set(&err_error)) {
            picotm_error_mark_as_non_recoverable(error);
            return;
        }
    }
err_picotm_lock_owner_reset_timestamp:
    picotm_lock_manager_release_irrevocability(self->lm, &self->lo);
}

static size_t
prepare_commit_cb(void* module, void* is_irrevocable,
                  struct picotm_error* error)
{
    assert(is_irrevocable);

    picotm_module_prepare_commit(module, *((bool*)is_irrevocable), error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static void
prepare_commit_modules(struct picotm_module* module, unsigned long nmodules,
                       bool is_irrevocable, struct picotm_error* error)
{
    tabwalk_2(module, nmodules, sizeof(*module), prepare_commit_cb,
              &is_irrevocable, error);
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

static void
apply_event(struct picotm_tx* tx, const struct picotm_event* event,
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
apply_events(struct picotm_tx* self, struct picotm_error* error)
{
    picotm_events_foreach1(picotm_log_begin(&self->log),
                           picotm_log_end(&self->log),
                           self, apply_event_cb, error);
    picotm_log_clear(&self->log);
}

static void
undo_event(struct picotm_tx* tx, const struct picotm_event* event,
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
undo_events(struct picotm_tx* self, struct picotm_error* error)
{
    picotm_events_rev_foreach1(picotm_log_begin(&self->log),
                               picotm_log_end(&self->log),
                               self, undo_event_cb, error);
    picotm_log_clear(&self->log);
}

void
picotm_tx_commit(struct picotm_tx* self, struct picotm_error* error)
{
    assert(self);

    bool is_irrevocable = picotm_tx_is_irrevocable(self);
    bool is_non_recoverable = false;

    prepare_commit_modules(self->module, self->nmodules, is_irrevocable,
                           error);
    if (picotm_error_is_set(error)) {
        goto err_prepare_commit_modules;
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

    finish_modules(self->module, self->nmodules, error);
    if (picotm_error_is_set(error)) {
        goto err;
    }

    picotm_lock_manager_release_irrevocability(self->lm, &self->lo);

    return;

err_apply_events:
err_apply_modules:
err_prepare_commit_modules:
err:
    if (is_non_recoverable) {
        picotm_error_mark_as_non_recoverable(error);
    }
}

void
picotm_tx_rollback(struct picotm_tx* self, struct picotm_error* error)
{
    assert(self);

    undo_modules(self->module, self->nmodules, error);
    if (picotm_error_is_set(error)) {
        goto err;
    }

    undo_events(self, error);
    if (picotm_error_is_set(error)) {
        goto err;
    }

    finish_modules(self->module, self->nmodules, error);
    if (picotm_error_is_set(error)) {
        goto err;
    }

    picotm_lock_manager_release_irrevocability(self->lm, &self->lo);

    return;

err:
    picotm_error_mark_as_non_recoverable(error);
    picotm_lock_manager_release_irrevocability(self->lm, &self->lo);
}
