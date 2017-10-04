/* Permission is hereby granted, free of charge, to any person obtaining a
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
 */

#include "tx.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "table.h"
#include "tx_shared.h"

/* The maximum number of retries per transaction. If a transacion reaches
 * this limit it switches to irrevocable mode. The actual limit depends on
 * the system's access pattern. The more conflicts, the lower the limit
 * should be. The current value has been choosen arbitrarily and requires
 * further optimization! */
static const unsigned long TX_NRETRIES_LIMIT = 10;

void
tx_init(struct tx* self, struct tx_shared* tx_shared)
{
    assert(self);

    log_init(&self->log);

    self->shared = tx_shared;
    self->mode = TX_MODE_REVOCABLE;
    self->nretries = 0;
    self->nmodules = 0;
    self->is_initialized = true;
}

void
tx_release(struct tx* self)
{
    assert(self);

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
    assert(self);

    return self->mode == TX_MODE_IRREVOCABLE;
}

unsigned long
tx_register_module(struct tx* self,
                   void (*lock)(void*, struct picotm_error*),
                   void (*unlock)(void*, struct picotm_error*),
                   void (*validate)(void*, int, struct picotm_error*),
                   void (*apply)(void*, struct picotm_error*),
                   void (*undo)(void*, struct picotm_error*),
                   void (*apply_event)(const struct picotm_event*,
                                       void*, struct picotm_error*),
                   void (*undo_event)(const struct picotm_event*,
                                      void*, struct picotm_error*),
                   void (*update_cc)(void*, int, struct picotm_error*),
                   void (*clear_cc)(void*, int, struct picotm_error*),
                   void (*finish)(void*, struct picotm_error*),
                   void (*uninit)(void*),
                   void* data,
                   struct picotm_error* error)
{
    assert(self);

    unsigned long module = self->nmodules;

    if (module >= picotm_arraylen(self->module)) {
        picotm_error_set_error_code(error, PICOTM_OUT_OF_MEMORY);
        return 0;
    }

    module_init(self->module + module, lock, unlock, validate,
                apply, undo, apply_event, undo_event,
                update_cc, clear_cc,
                finish, uninit, data);

    self->nmodules = module + 1;

    return module;
}

void
tx_append_event(struct tx* self, unsigned long module, unsigned long op,
                uintptr_t cookie, struct picotm_error* error)
{
    assert(self);

    log_append_event(&self->log, module, op, cookie, error);
}

void
tx_begin(struct tx* self, enum tx_mode mode, bool is_retry,
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
            tx_shared_wait_irrevocable(self->shared, error);
            break;
        case TX_MODE_IRREVOCABLE:
            /* If we're supposed to run exclusively, we wait
             * for the other transactions to finish. */
            tx_shared_make_irrevocable(self->shared, self, error);
            break;
    }
    if (picotm_error_is_set(error)) {
        return;
    }

    self->nretries = nretries;
    self->mode = mode;
}

static size_t
lock_cb(void* module, struct picotm_error* error)
{
    module_lock(module, error);
    return picotm_error_is_set(error) ? 0 : 1;
}

static void
lock_modules(struct module* module, unsigned long nmodules,
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
    module_unlock(module, error);
    return picotm_error_is_set(error) ? 0 : 1;
}

static void
unlock_modules(struct module* module, unsigned long nmodules,
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

    module_validate(module, *((bool*)is_irrevocable), error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static void
validate_modules(struct module* module, unsigned long nmodules,
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
    module_apply(module, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static void
apply_modules(struct module* module, unsigned long nmodules,
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
    module_undo(module, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static void
undo_modules(struct module* module, unsigned long nmodules,
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

    module_update_cc(module, *((bool*)is_irrevocable), error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static void
update_modules_cc(struct module* module, unsigned long nmodules,
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

    module_clear_cc(module, *((bool*)is_irrevocable), error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static void
clear_modules_cc(struct module* module, unsigned long nmodules,
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
    module_finish(module, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static void
finish_modules(struct module* module, unsigned long nmodules,
               struct picotm_error* error)
{
    tabwalk_1(module, nmodules, sizeof(*module), log_finish_cb, error);
    if (picotm_error_is_set(error)) {
        return;
    }
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

    log_apply_events(&self->log, self->module, is_irrevocable, error);
    if (picotm_error_is_set(error)) {
        goto err_log_apply_events;
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

    tx_shared_release_irrevocability(self->shared);

    return;

err_update_modules_cc:
err_log_apply_events:
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
    tx_shared_release_irrevocability(self->shared);
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

    log_undo_events(&self->log, self->module, is_irrevocable, error);
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

    tx_shared_release_irrevocability(self->shared);

    return;

err:
    picotm_error_mark_as_non_recoverable(error);
    tx_shared_release_irrevocability(self->shared);
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
