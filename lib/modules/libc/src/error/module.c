/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "module.h"
#include <assert.h>
#include <picotm/picotm-module.h>
#include <stdlib.h>
#include "error_tx.h"

struct error_module {
    struct error_tx tx;
    bool            is_initialized;
};

/*
 * Module interface
 */

static void
errno_undo_events(const struct event* event, size_t nevents,
                  struct error_module* module, struct picotm_error* error)
{
    error_tx_undo(&module->tx, error);
}

static void
errno_finish(struct error_module* module, struct picotm_error* error)
{
    error_tx_finish(&module->tx, error);
}

static void
errno_release(struct error_module* module)
{
    error_tx_uninit(&module->tx);
    module->is_initialized = false;
}

/*
 * Thread-local data
 */

static void
undo_events_cb(const struct event* event, size_t nevents, void* data,
               struct picotm_error* error)
{
    errno_undo_events(event, nevents, data, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    errno_finish(data, error);
}

static void
uninit_cb(void* data)
{
    errno_release(data);
}

static struct error_tx*
get_error_tx(bool initialize, struct picotm_error* error)
{
    static __thread struct error_module t_module;

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

    error_tx_init(&t_module.tx, module);

    t_module.is_initialized = true;

    return &t_module.tx;
}

static struct error_tx*
get_non_null_error_tx(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    struct error_tx* error_tx = get_error_tx(true, &error);
    if (picotm_error_is_set(&error)) {
        picotm_recover_from_error(&error);
    }
    /* assert here as there's no legal way that error_tx could be NULL */
    assert(error_tx);
    return error_tx;
}

/*
 * Public interface
 */

enum {
    SAVE_ERRNO = 0
};

void
error_module_save_errno()
{
    struct error_tx* error_tx = get_non_null_error_tx();

    /* We only have to save 'errno' once per transaction. */
    if (error_tx_errno_saved(error_tx)) {
        return;
    }

    error_tx_save_errno(error_tx);

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    picotm_append_event(error_tx->module, SAVE_ERRNO, 0, &error);
    if (picotm_error_is_set(&error)) {
        picotm_recover_from_error(&error);
    }
}

void
error_module_set_error_recovery(enum picotm_libc_error_recovery recovery)
{
    return error_tx_set_error_recovery(get_non_null_error_tx(), recovery);
}

enum picotm_libc_error_recovery
error_module_get_error_recovery()
{
    return error_tx_get_error_recovery(get_non_null_error_tx());
}
