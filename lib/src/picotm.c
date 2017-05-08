/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm.h"
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "table.h"
#include "tx.h"
#include "tx_shared.h"

/*
 * Global data
 */

static struct tx_shared*
get_tx_shared(void)
{
    static struct tx_shared g_tx_shared;
    static bool             g_tx_shared_is_initialized;

    if (__atomic_load_n(&g_tx_shared_is_initialized, __ATOMIC_ACQUIRE)) {
        return &g_tx_shared;
    }

    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    int res = pthread_mutex_lock(&lock);
    if (res) {
        return NULL;
    }

    if (__atomic_load_n(&g_tx_shared_is_initialized, __ATOMIC_ACQUIRE)) {
        /* Another transaction initialized the tx_state structure
         * concurrently; we're done. */
        goto out;
    }

    res = tx_shared_init(&g_tx_shared);
    if (res < 0) {
        goto err_tx_shared_init;
    }

    __atomic_store_n(&g_tx_shared_is_initialized, true, __ATOMIC_RELEASE);

out:
    pthread_mutex_unlock(&lock);
    return &g_tx_shared;

err_tx_shared_init:
    pthread_mutex_unlock(&lock);
    return NULL;
}

/*
 * Thread-local data
 */

static struct tx*
get_tx(bool do_init)
{
    static __thread struct tx t_tx;

    if (t_tx.is_initialized) {
        return &t_tx;
    } else if (!do_init) {
        return NULL;
    }

    struct tx_shared* tx_shared = get_tx_shared();
    if (!tx_shared) {
        return NULL;
    }

    int res = tx_init(&t_tx, tx_shared);
    if (res < 0) {
        return NULL;
    }
    return &t_tx;
}

static struct tx*
get_non_null_tx(void)
{
    struct tx* tx = get_tx(true);
    if (!tx) {
        /* abort here as there's no legal way that tx could be NULL */
        abort();
    }
    return tx;
}

static struct picotm_error*
get_non_null_error(void)
{
    static __thread struct picotm_error t_error;

    return &t_error;
}

/*
 * Public interface
 */

PICOTM_EXPORT
struct __picotm_tx*
__picotm_get_tx()
{
    struct tx* tx = get_tx(true);
    if (!tx) {
        return NULL;
    }
    return &tx->public_state;
}

PICOTM_EXPORT
_Bool
__picotm_begin(enum __picotm_mode mode)
{
    static const unsigned char tx_mode[] = {
        TX_MODE_REVOCABLE,
        TX_MODE_REVOCABLE,
        TX_MODE_IRREVOCABLE
    };

    if (mode == PICOTM_MODE_RECOVERY) {
        /* We're recovering from an error. Returning 'false'
         * will invoke the transaction's recovery code. */
        return false;
    }

    struct tx* tx = get_tx(true);
    if (!tx) {
        return false;
    }

    int res = tx_begin(tx, tx_mode[mode]);
    if (res < 0) {
        /* TODO: error handling */
        return false;
    }

    return true;
}

static void
restart_tx(struct tx* tx, enum __picotm_mode mode)
{
    int res = tx_rollback(tx);
    if (res < 0) {
        /* TODO: warn and push forward */
    }

    /* Restarting the transaction here transfers control
     * to __picotm_begin(). */
    longjmp(tx->public_state.env, (int)mode);
}

PICOTM_EXPORT
void
__picotm_commit()
{
    struct tx* tx = get_non_null_tx();
    int res = tx_commit(tx);
    if (res < 0) {
        restart_tx(tx, PICOTM_MODE_RETRY);
    }
}

PICOTM_EXPORT
void
picotm_restart()
{
    restart_tx(get_non_null_tx(), PICOTM_MODE_RETRY);
}

PICOTM_EXPORT
bool
picotm_is_valid()
{
    return tx_is_valid(get_non_null_tx());
}

PICOTM_EXPORT
void
picotm_irrevocable()
{
    restart_tx(get_non_null_tx(), PICOTM_MODE_IRREVOCABLE);
}

PICOTM_EXPORT
bool
picotm_is_irrevocable()
{
    return tx_is_irrevocable(get_non_null_tx());
}

PICOTM_EXPORT
void
picotm_release()
{
    struct tx* tx = get_tx(false);
    if (!tx) {
        return;
    }
    tx_release(tx);
}

/*
 * Module interface
 */

PICOTM_EXPORT
long
picotm_register_module(int (*lock)(void*),
                      int (*unlock)(void*),
                      int (*validate)(void*, int),
                      int (*apply_event)(const struct event*, size_t, void*),
                      int (*undo_event)(const struct event*, size_t, void*),
                      int (*updatecc)(void*, int),
                      int (*clearcc)(void*, int),
                      int (*finish)(void*),
                      int (*uninit)(void*),
                      void* data)
{
    return tx_register_module(get_non_null_tx(), lock, unlock, validate,
                              apply_event, undo_event, updatecc, clearcc,
                              finish, uninit, data);
}

PICOTM_EXPORT
int
picotm_inject_event(unsigned long module, unsigned long op, uintptr_t cookie)
{
    int res = tx_inject_event(get_non_null_tx(), module, op, cookie);
    if (res < 0) {
        return res;
    }
    return 0;
}

PICOTM_EXPORT
void
picotm_resolve_conflict(struct picotm_tx* conflicting_tx)
{
    picotm_error_set_conflicting(get_non_null_error(), conflicting_tx);

    restart_tx(get_non_null_tx(), PICOTM_MODE_RETRY);
}

PICOTM_EXPORT
void
picotm_recover_from_error_code(enum picotm_error_code error_hint)
{
    picotm_error_set_error_code(get_non_null_error(), error_hint);

    /* Nothing we can do on errors; let's try to recover. */
    restart_tx(get_non_null_tx(), PICOTM_MODE_RECOVERY);
}

PICOTM_EXPORT
void
picotm_recover_from_errno(int errno_hint)
{
    picotm_error_set_errno(get_non_null_error(), errno_hint);

    /* Nothing we can do on errors; let's try to recover. */
    restart_tx(get_non_null_tx(), PICOTM_MODE_RECOVERY);
}

/* Tables
 */

PICOTM_EXPORT
void*
picotm_tabresize(void* base, size_t nelems, size_t newnelems, size_t siz)
{
    return tabresize(base, nelems, newnelems, siz);
}

PICOTM_EXPORT
void
picotm_tabfree(void* base)
{
    tabfree(base);
}

PICOTM_EXPORT
int
picotm_tabwalk_1(void* base, size_t nelems, size_t siz, int (*walk)(void*))
{
    return tabwalk_1(base, nelems, siz, walk);
}

PICOTM_EXPORT
int
picotm_tabwalk_2(void* base, size_t nelems, size_t siz,
                int (*walk)(void*, void*), void* data)
{
    return tabwalk_2(base, nelems, siz, walk, data);
}

PICOTM_EXPORT
int
picotm_tabwalk_3(void* base, size_t nelems, size_t siz,
                int (*walk)(void*, void*, void*), void* data1, void* data2)
{
    return tabwalk_3(base, nelems, siz, walk, data1, data2);
}

PICOTM_EXPORT
int
picotm_tabrwalk_1(void* base, size_t nelems, size_t siz, int (*walk)(void*))
{
    return tabrwalk_1(base, nelems, siz, walk);
}

PICOTM_EXPORT
int
picotm_tabrwalk_2(void* base, size_t nelems, size_t siz,
                 int (*walk)(void*, void*), void* data)
{
    return tabrwalk_2(base, nelems, siz, walk, data);
}

PICOTM_EXPORT
size_t
picotm_tabuniq(void* base, size_t nelems, size_t siz,
              int (*compare)(const void*, const void*))
{
    return tabuniq(base, nelems, siz, compare);
}
