/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/picotm-libc.h"
#include <assert.h>
#include <picotm/picotm-module.h>
#include <stdlib.h>
#include "fpu_tx.h"

struct fpu_module {
    struct fpu_tx tx;
    bool          is_initialized;
};

/*
 * Module interface
 */

static void
fpu_module_undo_events(const struct event* event, size_t nevents,
                       struct fpu_module* module, struct picotm_error* error)
{
    fpu_tx_undo(&module->tx, error);
}

static void
fpu_module_finish(struct fpu_module* module, struct picotm_error* error)
{
    fpu_tx_finish(&module->tx);
}

static void
fpu_module_uninit(struct fpu_module* module)
{
    fpu_tx_uninit(&module->tx);
    module->is_initialized = false;
}

/*
 * Thread-local data
 */

static void
undo_events_cb(const struct event* event, size_t nevents, void* data,
               struct picotm_error* error)
{
    fpu_module_undo_events(event, nevents, data, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    fpu_module_finish(data, error);
}

static void
uninit_cb(void* data)
{
    fpu_module_uninit(data);
}

static struct fpu_tx*
get_fpu_tx(bool initialize, struct picotm_error* error)
{
    static __thread struct fpu_module t_module;

    if (t_module.is_initialized) {
        return &t_module.tx;
    } else if (!initialize) {
        return NULL;
    }

    unsigned long module = picotm_register_module(NULL, NULL, NULL,
                                                  NULL, undo_events_cb,
                                                  NULL, NULL,
                                                  finish_cb,
                                                  uninit_cb,
                                                  &t_module,
                                                  error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    fpu_tx_init(&t_module.tx, module, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    t_module.is_initialized = true;

    return &t_module.tx;
}

static struct fpu_tx*
get_non_null_fpu_tx(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    struct fpu_tx* fpu_tx = get_fpu_tx(true, &error);
    if (picotm_error_is_set(&error)) {
        picotm_recover_from_error(&error);
    }
    /* assert() here as there's no legal way that fpu_tx could be NULL */
    assert(fpu_tx);
    return fpu_tx;
}

/*
 * Public interface
 */

enum {
    SAVE_FENV = 0,
    SAVE_FEXCEPT
};

void
fpu_module_save_fenv()
{
    struct fpu_tx* fpu_tx = get_non_null_fpu_tx();

    /* We have to save the floating-point enviroment
     * only once per transaction. */
    if (fpu_tx_fenv_saved(fpu_tx)) {
        return;
    }

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    fpu_tx_save_fenv(fpu_tx, &error);
    if (picotm_error_is_set(&error)) {
        picotm_recover_from_error(&error);
    }

    int res = picotm_inject_event(fpu_tx->module, SAVE_FENV, 0);
    if (res < 0) {
        picotm_recover_from_errno(0);
    }
}

void
fpu_module_save_fexcept()
{
    struct fpu_tx* fpu_tx = get_non_null_fpu_tx();

    /* We have to save the floating-point status
     * flags only once per transaction. */
    if (fpu_tx_fexcept_saved(fpu_tx)) {
        return;
    }

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    fpu_tx_save_fexcept(fpu_tx, &error);
    if (picotm_error_is_set(&error)) {
        picotm_recover_from_error(&error);
    }

    int res = picotm_inject_event(fpu_tx->module, SAVE_FEXCEPT, 0);
    if (res < 0) {
        picotm_recover_from_errno(0);
    }
}
