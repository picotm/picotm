/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tm.h"
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <systx/systx-module.h>
#include <systx/systx-tm.h>
#include "vmem.h"
#include "vmem_tx.h"

static int
lock_cb(void* data)
{
    return systx_tm_lock();
}

static int
unlock_cb(void* data)
{
    return systx_tm_unlock();
}

static int
validate_cb(void* data, int eotx)
{
    return systx_tm_validate(!!eotx);
}

static int
apply_event_cb(const struct event* event, size_t nevents, void* data)
{
    return systx_tm_apply();
}

static int
undo_event_cb(const struct event* event, size_t nevents, void* data)
{
    return systx_tm_undo();
}

static int
finish_cb(void* data)
{
    return systx_tm_finish();
}

static int
release_cb(void* data)
{
    return systx_tm_release();
}

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
 * Thread-local data
 */

struct tm_vmem_tx*
get_vmem_tx(void)
{
    static __thread struct tm_vmem_tx t_vmem_tx;

    if (t_vmem_tx.is_initialized) {
        return &t_vmem_tx;
    }

    long res = systx_register_module(lock_cb, unlock_cb, validate_cb,
                                     apply_event_cb, undo_event_cb,
                                     NULL, NULL,
                                     finish_cb,
                                     release_cb,
                                     NULL, NULL, NULL, NULL,
                                     NULL);
    if (res < 0) {
        return NULL;
    }
    unsigned long module = res;

    struct tm_vmem* vmem = get_vmem();
    if (!vmem) {
        return NULL;
    }

    res = tm_vmem_tx_init(&t_vmem_tx, vmem, module);
    if (res < 0) {
        return NULL;
    }

    return &t_vmem_tx;
}

/*
 * Module interface
 */

int
systx_tm_lock()
{
    struct tm_vmem_tx* vmem_tx = get_vmem_tx();
    if (!vmem_tx) {
        /* TODO: hard error */
        return -EAGAIN;
    }
    int res = tm_vmem_tx_lock(vmem_tx);
    if (res < 0) {
        return res;
    }
    return 0;
}

int
systx_tm_unlock()
{
    struct tm_vmem_tx* vmem_tx = get_vmem_tx();
    if (!vmem_tx) {
        /* TODO: hard error */
        return -EAGAIN;
    }
    int res = tm_vmem_tx_unlock(vmem_tx);
    if (res < 0) {
        return res;
    }
    return 0;
}

int
systx_tm_validate(bool eotx)
{
    struct tm_vmem_tx* vmem_tx = get_vmem_tx();
    if (!vmem_tx) {
        /* TODO: hard error */
        return -EAGAIN;
    }
    int res = tm_vmem_tx_validate(vmem_tx, eotx);
    if (res < 0) {
        return res;
    }
    return 0;
}

int
systx_tm_apply()
{
    struct tm_vmem_tx* vmem_tx = get_vmem_tx();
    if (!vmem_tx) {
        /* TODO: hard error */
        return -EAGAIN;
    }
    int res = tm_vmem_tx_apply(vmem_tx);
    if (res < 0) {
        return res;
    }
    return 0;
}

int
systx_tm_undo()
{
    struct tm_vmem_tx* vmem_tx = get_vmem_tx();
    if (!vmem_tx) {
        /* TODO: hard error */
        return -EAGAIN;
    }
    int res = tm_vmem_tx_undo(vmem_tx);
    if (res < 0) {
        return res;
    }
    return 0;
}

int
systx_tm_finish()
{
    struct tm_vmem_tx* vmem_tx = get_vmem_tx();
    if (!vmem_tx) {
        /* TODO: hard error */
        return -EAGAIN;
    }
    int res = tm_vmem_tx_finish(vmem_tx);
    if (res < 0) {
        return res;
    }
    return 0;
}

int
systx_tm_release()
{
    struct tm_vmem_tx* vmem_tx = get_vmem_tx();
    if (!vmem_tx) {
        /* TODO: hard error */
        return -EAGAIN;
    }
    tm_vmem_tx_release(vmem_tx);
    return 0;
}

/*
 * Public interface
 */

enum {
    SYSTX_TM_LOAD = 0,
    SYSTX_TM_STORE,
    SYSTX_TM_LOADSTORE,
    SYSTX_TM_PRIVATIZE
};

SYSTX_EXPORT
void
__systx_tm_load(uintptr_t addr, void* buf, size_t siz)
{
    struct tm_vmem_tx* vmem_tx = get_vmem_tx();
    if (!vmem_tx) {
        systx_resolve_error(ENOMEM);
    }
    int res;
    while ((res = tm_vmem_tx_ld(vmem_tx, addr, buf, siz)) == -EBUSY) {
        systx_resolve_conflict(NULL);
    }
    if (res < 0) {
        systx_resolve_error(-res);
    }
    res = systx_inject_event(vmem_tx->module, SYSTX_TM_LOAD, 0);
    if (res < 0) {
        systx_resolve_error(-res);
    }
}

SYSTX_EXPORT
void
__systx_tm_store(uintptr_t addr, const void* buf, size_t siz)
{
    struct tm_vmem_tx* vmem_tx = get_vmem_tx();
    if (!vmem_tx) {
        systx_resolve_error(-ENOMEM);
    }
    int res;
    while ((res = tm_vmem_tx_st(vmem_tx, addr, buf, siz)) == -EBUSY) {
        systx_resolve_conflict(NULL);
    }
    if (res < 0) {
        systx_resolve_error(-res);
    }
    res = systx_inject_event(vmem_tx->module, SYSTX_TM_STORE, 0);
    if (res < 0) {
        systx_resolve_error(-res);
    }
}

SYSTX_EXPORT
void
__systx_tm_loadstore(uintptr_t laddr, uintptr_t saddr, size_t siz)
{
    struct tm_vmem_tx* vmem_tx = get_vmem_tx();
    if (!vmem_tx) {
        systx_resolve_error(-ENOMEM);
    }
    int res;
    while ((res = tm_vmem_tx_ldst(vmem_tx, laddr, saddr, siz)) == -EBUSY) {
        systx_resolve_conflict(NULL);
    }
    if (res < 0) {
        systx_resolve_error(-res);
    }
    res = systx_inject_event(vmem_tx->module, SYSTX_TM_LOADSTORE, 0);
    if (res < 0) {
        systx_resolve_error(-res);
    }
}

SYSTX_EXPORT
void
__systx_tm_privatize(uintptr_t addr, size_t siz, unsigned long flags)
{
    struct tm_vmem_tx* vmem_tx = get_vmem_tx();
    if (!vmem_tx) {
        systx_resolve_error(-ENOMEM);
    }
    int res;
    while ((res = tm_vmem_tx_privatize(vmem_tx, addr, siz, flags)) == -EBUSY) {
        systx_resolve_conflict(NULL);
    }
    if (res < 0) {
        systx_resolve_error(-res);
    }
    res = systx_inject_event(vmem_tx->module, SYSTX_TM_PRIVATIZE, 0);
    if (res < 0) {
        systx_resolve_error(-res);
    }
}

SYSTX_EXPORT
void
__systx_tm_privatize_c(uintptr_t addr, int c, unsigned long flags)
{
    struct tm_vmem_tx* vmem_tx = get_vmem_tx();
    if (!vmem_tx) {
        systx_resolve_error(-ENOMEM);
    }
    int res;
    while ((res = tm_vmem_tx_privatize_c(vmem_tx, addr, c, flags)) == -EBUSY) {
        systx_resolve_conflict(NULL);
    }
    if (res < 0) {
        systx_resolve_error(-res);
    }
    res = systx_inject_event(vmem_tx->module, SYSTX_TM_PRIVATIZE, 0);
    if (res < 0) {
        systx_resolve_error(-res);
    }
}
