/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <picotm/picotm-module.h>
#include "picotm/picotm-tm.h"
#include "vmem.h"
#include "vmem_tx.h"

/*
 * Global data
 */

static struct tm_vmem* get_vmem(void);

static void
vmem_atexit_cb(void)
{
    struct tm_vmem* vmem = get_vmem();
    tm_vmem_uninit(vmem);
}

static struct tm_vmem*
get_vmem()
{
    static bool           g_vmem_is_initialized;
    static struct tm_vmem g_vmem;

    if (__atomic_load_n(&g_vmem_is_initialized, __ATOMIC_ACQUIRE)) {
        return &g_vmem;
    }

    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    int res = pthread_mutex_lock(&lock);
    if (res) {
        return NULL;
    }

    if (__atomic_load_n(&g_vmem_is_initialized, __ATOMIC_ACQUIRE)) {
        /* Another transaction initialized the vmem structure
         * concurrently; we're done. */
        goto out;
    }

    res = tm_vmem_init(&g_vmem);
    if (res < 0) {
        goto err_tm_vmem_init;
    }

    atexit(vmem_atexit_cb); /* ignore errors */

    __atomic_store_n(&g_vmem_is_initialized, true, __ATOMIC_RELEASE);

out:
    pthread_mutex_unlock(&lock);
    return &g_vmem;

err_tm_vmem_init:
    pthread_mutex_unlock(&lock);
    return NULL;
};

/*
 * Module interface
 */

struct tm_module {
    struct tm_vmem_tx tx;

    /* True if module structure has been initialized, false otherwise */
    bool is_initialized;
};

static int
lock(struct tm_module* module, struct picotm_error* error)
{
    int res = tm_vmem_tx_lock(&module->tx);
    if (res < 0) {
        if (res == -EBUSY) {
            picotm_error_set_conflicting(error, NULL);
        } else {
            picotm_error_set_errno(error, -res);
        }
        return -1;
    }
    return 0;
}

static int
unlock(struct tm_module* module, struct picotm_error* error)
{
    int res = tm_vmem_tx_unlock(&module->tx);
    if (res < 0) {
        if (res == -EBUSY) {
            picotm_error_set_conflicting(error, NULL);
        } else {
            picotm_error_set_errno(error, -res);
        }
        return -1;
    }
    return 0;
}

static int
validate(struct tm_module* module, bool eotx, struct picotm_error* error)
{
    int res = tm_vmem_tx_validate(&module->tx, eotx);
    if (res < 0) {
        if (res == -EBUSY) {
            picotm_error_set_conflicting(error, NULL);
        } else {
            picotm_error_set_errno(error, -res);
        }
        return -1;
    }
    return 0;
}

static int
apply(struct tm_module* module, const struct event* event, size_t nevents,
      struct picotm_error* error)
{
    int res = tm_vmem_tx_apply(&module->tx);
    if (res < 0) {
        if (res == -EBUSY) {
            picotm_error_set_conflicting(error, NULL);
        } else {
            picotm_error_set_errno(error, -res);
        }
        return -1;
    }
    return 0;
}

static int
undo(struct tm_module* module, const struct event* event, size_t nevents,
     struct picotm_error* error)
{
    int res = tm_vmem_tx_undo(&module->tx);
    if (res < 0) {
        if (res == -EBUSY) {
            picotm_error_set_conflicting(error, NULL);
        } else {
            picotm_error_set_errno(error, -res);
        }
        return -1;
    }
    return 0;
}

static int
finish(struct tm_module* module, struct picotm_error* error)
{
    int res = tm_vmem_tx_finish(&module->tx);
    if (res < 0) {
        if (res == -EBUSY) {
            picotm_error_set_conflicting(error, NULL);
        } else {
            picotm_error_set_errno(error, -res);
        }
        return -1;
    }
    return 0;
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

static int
lock_cb(void* data, struct picotm_error* error)
{
    return lock(data, error);
}

static int
unlock_cb(void* data, struct picotm_error* error)
{
    return unlock(data, error);
}

static int
validate_cb(void* data, int eotx, struct picotm_error* error)
{
    return validate(data, !!eotx, error);
}

static int
apply_event_cb(const struct event* event, size_t nevents, void* data,
               struct picotm_error* error)
{
    return apply(data, event, nevents, error);
}

static int
undo_event_cb(const struct event* event, size_t nevents, void* data,
              struct picotm_error* error)
{
    return undo(data, event, nevents, error);
}

static int
finish_cb(void* data, struct picotm_error* error)
{
    return finish(data, error);
}

static void
uninit_cb(void* data)
{
    uninit(data);
}

static struct tm_vmem_tx*
get_vmem_tx(void)
{
    static __thread struct tm_module t_module;

    if (t_module.is_initialized) {
        return &t_module.tx;
    }

    long res = picotm_register_module(lock_cb, unlock_cb, validate_cb,
                                      apply_event_cb, undo_event_cb,
                                      NULL, NULL,
                                      finish_cb,
                                      uninit_cb,
                                      &t_module);
    if (res < 0) {
        return NULL;
    }
    unsigned long module = res;

    struct tm_vmem* vmem = get_vmem();
    if (!vmem) {
        return NULL;
    }

    res = tm_vmem_tx_init(&t_module.tx, vmem, module);
    if (res < 0) {
        return NULL;
    }

    t_module.is_initialized = true;

    return &t_module.tx;
}

/*
 * Public interface
 */

enum {
    PICOTM_TM_LOAD = 0,
    PICOTM_TM_STORE,
    PICOTM_TM_LOADSTORE,
    PICOTM_TM_PRIVATIZE
};

PICOTM_EXPORT
void
__picotm_tm_load(uintptr_t addr, void* buf, size_t siz)
{
    struct tm_vmem_tx* vmem_tx = get_vmem_tx();
    if (!vmem_tx) {
        picotm_recover_from_errno(ENOMEM);
    }
    int res;
    while ((res = tm_vmem_tx_ld(vmem_tx, addr, buf, siz)) == -EBUSY) {
        picotm_resolve_conflict(NULL);
    }
    if (res < 0) {
        picotm_recover_from_errno(-res);
    }
    res = picotm_inject_event(vmem_tx->module, PICOTM_TM_LOAD, 0);
    if (res < 0) {
        picotm_recover_from_errno(0);
    }
}

PICOTM_EXPORT
void
__picotm_tm_store(uintptr_t addr, const void* buf, size_t siz)
{
    struct tm_vmem_tx* vmem_tx = get_vmem_tx();
    if (!vmem_tx) {
        picotm_recover_from_errno(ENOMEM);
    }
    int res;
    while ((res = tm_vmem_tx_st(vmem_tx, addr, buf, siz)) == -EBUSY) {
        picotm_resolve_conflict(NULL);
    }
    if (res < 0) {
        picotm_recover_from_errno(-res);
    }
    res = picotm_inject_event(vmem_tx->module, PICOTM_TM_STORE, 0);
    if (res < 0) {
        picotm_recover_from_errno(0);
    }
}

PICOTM_EXPORT
void
__picotm_tm_loadstore(uintptr_t laddr, uintptr_t saddr, size_t siz)
{
    struct tm_vmem_tx* vmem_tx = get_vmem_tx();
    if (!vmem_tx) {
        picotm_recover_from_errno(ENOMEM);
    }
    int res;
    while ((res = tm_vmem_tx_ldst(vmem_tx, laddr, saddr, siz)) == -EBUSY) {
        picotm_resolve_conflict(NULL);
    }
    if (res < 0) {
        picotm_recover_from_errno(-res);
    }
    res = picotm_inject_event(vmem_tx->module, PICOTM_TM_LOADSTORE, 0);
    if (res < 0) {
        picotm_recover_from_errno(0);
    }
}

PICOTM_EXPORT
void
__picotm_tm_privatize(uintptr_t addr, size_t siz, unsigned long flags)
{
    struct tm_vmem_tx* vmem_tx = get_vmem_tx();
    if (!vmem_tx) {
        picotm_recover_from_errno(ENOMEM);
    }
    int res;
    while ((res = tm_vmem_tx_privatize(vmem_tx, addr, siz, flags)) == -EBUSY) {
        picotm_resolve_conflict(NULL);
    }
    if (res < 0) {
        picotm_recover_from_errno(res);
    }
    res = picotm_inject_event(vmem_tx->module, PICOTM_TM_PRIVATIZE, 0);
    if (res < 0) {
        picotm_recover_from_errno(0);
    }
}

PICOTM_EXPORT
void
__picotm_tm_privatize_c(uintptr_t addr, int c, unsigned long flags)
{
    struct tm_vmem_tx* vmem_tx = get_vmem_tx();
    if (!vmem_tx) {
        picotm_recover_from_errno(ENOMEM);
    }
    int res;
    while ((res = tm_vmem_tx_privatize_c(vmem_tx, addr, c, flags)) == -EBUSY) {
        picotm_resolve_conflict(NULL);
    }
    if (res < 0) {
        picotm_recover_from_errno(-res);
    }
    res = picotm_inject_event(vmem_tx->module, PICOTM_TM_PRIVATIZE, 0);
    if (res < 0) {
        picotm_recover_from_errno(0);
    }
}
