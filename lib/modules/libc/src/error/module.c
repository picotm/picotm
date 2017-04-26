/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "module.h"
#include <picotm/picotm-module.h>
#include <stdlib.h>
#include "error_tx.h"

struct error_tx_thread_state {
    struct error_tx instance;
    bool            is_initialized;
};

static int errno_undo_events(const struct event* event, size_t nevents,
                             struct error_tx_thread_state* t_error_tx);
static int errno_finish(struct error_tx_thread_state* t_error_tx);
static int errno_release(struct error_tx_thread_state* t_error_tx);

static int
undo_events_cb(const struct event* event, size_t nevents, void* data)
{
    return errno_undo_events(event, nevents, data);
}

static int
finish_cb(void* data)
{
    return errno_finish(data);
}

static int
release_cb(void* data)
{
    return errno_release(data);
}

/*
 * Thread-local data
 */

struct error_tx*
get_error_tx(bool initialize)
{
    static __thread struct error_tx_thread_state t_error_tx;

    if (t_error_tx.is_initialized) {
        return &t_error_tx.instance;
    } else if (!initialize) {
        return NULL;
    }

    long res = picotm_register_module(NULL, NULL, NULL,
                                      NULL, undo_events_cb,
                                      NULL, NULL,
                                      finish_cb,
                                      release_cb,
                                      &t_error_tx);
    if (res < 0) {
        return NULL;
    }
    unsigned long module = res;

    res = error_tx_init(&t_error_tx.instance, module);
    if (res < 0) {
        return NULL;
    }

    t_error_tx.is_initialized = true;

    return &t_error_tx.instance;
}

static struct error_tx*
get_non_null_error_tx(bool initialize)
{
    struct error_tx* error_tx = get_error_tx(initialize);
    if (!error_tx) {
        /* abort here as there's no legal way that error_tx could be NULL */
        abort();
    }
    return error_tx;
}

/*
 * Module interface
 */

int
errno_undo_events(const struct event* event, size_t nevents,
                  struct error_tx_thread_state* t_error_tx)
{
    int res = error_tx_undo(&t_error_tx->instance);
    if (res < 0) {
        return res;
    }
    return 0;
}

int
errno_finish(struct error_tx_thread_state* t_error_tx)
{
    int res = error_tx_finish(&t_error_tx->instance);
    if (res < 0) {
        return res;
    }
    return 0;
}

int
errno_release(struct error_tx_thread_state* t_error_tx)
{
    error_tx_uninit(&t_error_tx->instance);
    t_error_tx->is_initialized = false;
    return 0;
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
    struct error_tx* error_tx = get_non_null_error_tx(true);

    /* We only have to save 'errno' once per transaction. */
    if (error_tx_errno_saved(error_tx)) {
        return;
    }

    error_tx_save_errno(error_tx);

    int res = picotm_inject_event(error_tx->module, SAVE_ERRNO, 0);
    if (res < 0) {
        picotm_recover_from_errno(res);
    }
}

void
error_module_set_error_recovery(enum picotm_libc_error_recovery recovery)
{
    return error_tx_set_error_recovery(get_non_null_error_tx(true), recovery);
}

enum picotm_libc_error_recovery
error_module_get_error_recovery()
{
    return error_tx_get_error_recovery(get_non_null_error_tx(true));
}
