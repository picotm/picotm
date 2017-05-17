/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "module.h"
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <picotm/picotm-module.h>
#include "vmem.h"
#include "vmem_tx.h"

/*
 * Global data
 */

static struct tm_vmem* get_vmem(struct picotm_error* error);

static void
vmem_atexit_cb(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    struct tm_vmem* vmem = get_vmem(&error);
    tm_vmem_uninit(vmem);
}

static struct tm_vmem*
get_vmem(struct picotm_error* error)
{
    static bool           g_vmem_is_initialized;
    static struct tm_vmem g_vmem;

    if (__atomic_load_n(&g_vmem_is_initialized, __ATOMIC_ACQUIRE)) {
        return &g_vmem;
    }

    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    int res = pthread_mutex_lock(&lock);
    if (res) {
        picotm_error_set_errno(error, res);
        return NULL;
    }

    if (__atomic_load_n(&g_vmem_is_initialized, __ATOMIC_ACQUIRE)) {
        /* Another transaction initialized the vmem structure
         * concurrently; we're done. */
        goto out;
    }

    tm_vmem_init(&g_vmem);

    atexit(vmem_atexit_cb); /* ignore errors */

    __atomic_store_n(&g_vmem_is_initialized, true, __ATOMIC_RELEASE);

out:
    res = pthread_mutex_unlock(&lock);
    if (res) {
        picotm_error_set_errno(error, res);
        return NULL;
    }
    return &g_vmem;
};

/*
 * Module interface
 */

struct tm_module {
    struct tm_vmem_tx tx;

    /* True if module structure has been initialized, false otherwise */
    bool is_initialized;
};

static void
lock(struct tm_module* module, struct picotm_error* error)
{
    tm_vmem_tx_lock(&module->tx, error);
}

static void
unlock(struct tm_module* module, struct picotm_error* error)
{
    tm_vmem_tx_unlock(&module->tx, error);
}

static void
validate(struct tm_module* module, bool eotx, struct picotm_error* error)
{
    tm_vmem_tx_validate(&module->tx, eotx, error);
}

static void
apply(struct tm_module* module, const struct event* event, size_t nevents,
      struct picotm_error* error)
{
    tm_vmem_tx_apply(&module->tx, error);
}

static void
undo(struct tm_module* module, const struct event* event, size_t nevents,
     struct picotm_error* error)
{
    tm_vmem_tx_undo(&module->tx, error);
}

static void
finish(struct tm_module* module, struct picotm_error* error)
{
    tm_vmem_tx_finish(&module->tx, error);
}

static void
uninit(struct tm_module* module)
{
    tm_vmem_tx_release(&module->tx);
    module->is_initialized = false;
}

/*
 * Thread-local data
 */

static void
lock_cb(void* data, struct picotm_error* error)
{
    lock(data, error);
}

static void
unlock_cb(void* data, struct picotm_error* error)
{
    unlock(data, error);
}

static bool
is_valid_cb(void* data, int eotx, struct picotm_error* error)
{
    validate(data, !!eotx, error);
    if (picotm_error_is_set(error)) {
        return false;
    }
    return true;
}

static void
apply_event_cb(const struct event* event, size_t nevents, void* data,
               struct picotm_error* error)
{
    apply(data, event, nevents, error);
}

static void
undo_event_cb(const struct event* event, size_t nevents, void* data,
              struct picotm_error* error)
{
    undo(data, event, nevents, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    finish(data, error);
}

static void
uninit_cb(void* data)
{
    uninit(data);
}

static struct tm_vmem_tx*
get_vmem_tx(bool initialize, struct picotm_error* error)
{
    static __thread struct tm_module t_module;

    if (t_module.is_initialized) {
        return &t_module.tx;
    } else if (!initialize) {
        return NULL;
    }

    unsigned long module = picotm_register_module(lock_cb, unlock_cb,
                                                  is_valid_cb,
                                                  apply_event_cb,
                                                  undo_event_cb,
                                                  NULL, NULL,
                                                  finish_cb,
                                                  uninit_cb,
                                                  &t_module,
                                                  error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct tm_vmem* vmem = get_vmem(error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    tm_vmem_tx_init(&t_module.tx, vmem, module);

    t_module.is_initialized = true;

    return &t_module.tx;
}

static struct tm_vmem_tx*
get_non_null_vmem_tx(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    struct tm_vmem_tx* vmem_tx = get_vmem_tx(true, &error);
    if (picotm_error_is_set(&error)) {
        picotm_recover_from_error(&error);
    }
    assert(vmem_tx);
    return vmem_tx;
}

/*
 * Public interface
 */

enum {
    TM_LOAD = 0,
    TM_STORE,
    TM_LOADSTORE,
    TM_PRIVATIZE
};

void
tm_module_load(uintptr_t addr, void* buf, size_t siz)
{
    struct tm_vmem_tx* vmem_tx = get_non_null_vmem_tx();

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    bool is_conflicting = false;

    do {
        tm_vmem_tx_ld(vmem_tx, addr, buf, siz, &error);

        is_conflicting = picotm_error_is_conflicting(&error);
        if (is_conflicting) {
            picotm_recover_from_error(&error);
            picotm_error_clear(&error);
        }
    } while (is_conflicting);

    if (picotm_error_is_set(&error)) {
        picotm_recover_from_error(&error);
    }

    picotm_append_event(vmem_tx->module, TM_LOAD, 0, &error);
    if (picotm_error_is_set(&error)) {
        picotm_recover_from_error(&error);
    }
}

void
tm_module_store(uintptr_t addr, const void* buf, size_t siz)
{
    struct tm_vmem_tx* vmem_tx = get_non_null_vmem_tx();

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    bool is_conflicting = false;

    do {
        tm_vmem_tx_st(vmem_tx, addr, buf, siz, &error);

        is_conflicting = picotm_error_is_conflicting(&error);
        if (is_conflicting) {
            picotm_recover_from_error(&error);
            picotm_error_clear(&error);
        }
    } while (is_conflicting);

    picotm_append_event(vmem_tx->module, TM_STORE, 0, &error);
    if (picotm_error_is_set(&error)) {
        picotm_recover_from_error(&error);
    }
}

void
tm_module_loadstore(uintptr_t laddr, uintptr_t saddr, size_t siz)
{
    struct tm_vmem_tx* vmem_tx = get_non_null_vmem_tx();

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    bool is_conflicting = false;

    do {
        tm_vmem_tx_ldst(vmem_tx, laddr, saddr, siz, &error);

        is_conflicting = picotm_error_is_conflicting(&error);
        if (is_conflicting) {
            picotm_recover_from_error(&error);
            picotm_error_clear(&error);
        }
    } while (is_conflicting);

    picotm_append_event(vmem_tx->module, TM_LOADSTORE, 0, &error);
    if (picotm_error_is_set(&error)) {
        picotm_recover_from_error(&error);
    }
}

void
tm_module_privatize(uintptr_t addr, size_t siz, unsigned long flags)
{
    struct tm_vmem_tx* vmem_tx = get_non_null_vmem_tx();

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    bool is_conflicting = false;

    do {
        tm_vmem_tx_privatize(vmem_tx, addr, siz, flags, &error);

        is_conflicting = picotm_error_is_conflicting(&error);
        if (is_conflicting) {
            picotm_recover_from_error(&error);
            picotm_error_clear(&error);
        }
    } while (is_conflicting);

    picotm_append_event(vmem_tx->module, TM_PRIVATIZE, 0, &error);
    if (picotm_error_is_set(&error)) {
        picotm_recover_from_error(&error);
    }
}

void
tm_module_privatize_c(uintptr_t addr, int c, unsigned long flags)
{
    struct tm_vmem_tx* vmem_tx = get_non_null_vmem_tx();

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    bool is_conflicting = false;

    do {
        tm_vmem_tx_privatize_c(vmem_tx, addr, c, flags, &error);

        is_conflicting = picotm_error_is_conflicting(&error);
        if (is_conflicting) {
            picotm_recover_from_error(&error);
            picotm_error_clear(&error);
        }
    } while (is_conflicting);

    picotm_append_event(vmem_tx->module, TM_PRIVATIZE, 0, &error);
    if (picotm_error_is_set(&error)) {
        picotm_recover_from_error(&error);
    }
}
