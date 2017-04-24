/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/picotm-libc.h"
#include <picotm/picotm-module.h>
#include <stdlib.h>
#include "fpu_tx.h"

struct fpu_tx_thread_state {
    struct fpu_tx instance;
    bool          is_initialized;
};

static int fpu_undo_events(const struct event* event, size_t nevents,
                           struct fpu_tx_thread_state* t_fpu_tx);
static int fpu_finish(struct fpu_tx_thread_state* t_fpu_tx);
static int fpu_release(struct fpu_tx_thread_state* t_fpu_tx);

static int
undo_events_cb(const struct event* event, size_t nevents, void* data)
{
    return fpu_undo_events(event, nevents, data);
}

static int
finish_cb(void* data)
{
    return fpu_finish(data);
}

static int
release_cb(void* data)
{
    return fpu_release(data);
}

/*
 * Thread-local data
 */

struct fpu_tx*
get_fpu_tx(bool initialize)
{
    static __thread struct fpu_tx_thread_state t_fpu_tx;

    if (t_fpu_tx.is_initialized) {
        return &t_fpu_tx.instance;
    } else if (!initialize) {
        return NULL;
    }

    long res = picotm_register_module(NULL, NULL, NULL,
                                      NULL, undo_events_cb,
                                      NULL, NULL,
                                      finish_cb,
                                      release_cb,
                                      &t_fpu_tx);
    if (res < 0) {
        return NULL;
    }
    unsigned long module = res;

    res = fpu_tx_init(&t_fpu_tx.instance, module);
    if (res < 0) {
        return NULL;
    }

    t_fpu_tx.is_initialized = true;

    return &t_fpu_tx.instance;
}

static struct fpu_tx*
get_non_null_fpu_tx(bool initialize)
{
    struct fpu_tx* fpu_tx = get_fpu_tx(initialize);
    if (!fpu_tx) {
        /* abort here as there's no legal way that fpu_tx could be NULL */
        abort();
    }
    return fpu_tx;
}

/*
 * Module interface
 */

int
fpu_undo_events(const struct event* event, size_t nevents,
                  struct fpu_tx_thread_state* t_fpu_tx)
{
    int res = fpu_tx_undo(&t_fpu_tx->instance);
    if (res < 0) {
        return res;
    }
    return 0;
}

int
fpu_finish(struct fpu_tx_thread_state* t_fpu_tx)
{
    int res = fpu_tx_finish(&t_fpu_tx->instance);
    if (res < 0) {
        return res;
    }
    return 0;
}

int
fpu_release(struct fpu_tx_thread_state* t_fpu_tx)
{
    fpu_tx_uninit(&t_fpu_tx->instance);
    t_fpu_tx->is_initialized = false;
    return 0;
}

/*
 * Public interface
 */

enum {
    SAVE_FENV = 0,
    SAVE_FEXCEPT
};

PICOTM_EXPORT
void
picotm_libm_save_fenv()
{
    struct fpu_tx* fpu_tx = get_non_null_fpu_tx(true);

    /* We have to save the floating-point enviroment
     * only once per transaction. */
    if (fpu_tx_fenv_saved(fpu_tx)) {
        return;
    }

    int res = fpu_tx_save_fenv(fpu_tx);
    if (res < 0) {
        picotm_recover_from_errno(0);
    }

    res = picotm_inject_event(fpu_tx->module, SAVE_FENV, 0);
    if (res < 0) {
        picotm_recover_from_errno(0);
    }
}

PICOTM_EXPORT
void
picotm_libm_save_fexcept()
{
    struct fpu_tx* fpu_tx = get_non_null_fpu_tx(true);

    /* We have to save the floating-point status
     * flags only once per transaction. */
    if (fpu_tx_fexcept_saved(fpu_tx)) {
        return;
    }

    int res  = fpu_tx_save_fexcept(fpu_tx);
    if (res < 0) {
        picotm_recover_from_errno(0);
    }

    res = picotm_inject_event(fpu_tx->module, SAVE_FEXCEPT, 0);
    if (res < 0) {
        picotm_recover_from_errno(0);
    }
}

