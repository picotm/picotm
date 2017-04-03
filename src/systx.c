/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systx.h"
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
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

/*
 * Public interface
 */

SYSTX_EXPORT
struct __systx_tx*
__systx_get_tx()
{
    struct tx* tx = get_tx(true);
    if (!tx) {
        return NULL;
    }
    return &tx->public_state;
}

SYSTX_EXPORT
void
__systx_begin(enum __systx_mode mode)
{
    struct tx* tx = get_tx(true);
    if (!tx) {
        return;
    }

    int res = tx_begin(tx, mode);
    if (res < 0) {
        /* TODO: error handling */
    }
}

static void
restart_tx(struct tx* tx, enum __systx_mode mode)
{
    int res = tx_rollback(tx);
    if (res < 0) {
        /* TODO: warn and push forward */
    }

    /* Restarting the transaction here transfers control
     * to __systx_begin(). */
    longjmp(tx->public_state.env, (int)mode);
}

SYSTX_EXPORT
void
systx_commit()
{
    struct tx* tx = get_non_null_tx();
    int res = tx_commit(tx);
    if (res < 0) {
        restart_tx(tx, SYSTX_MODE_RETRY);
    }
}

SYSTX_EXPORT
void
systx_abort()
{
    restart_tx(get_non_null_tx(), SYSTX_MODE_RETRY);
}

SYSTX_EXPORT
void
systx_irrevocable()
{
    restart_tx(get_non_null_tx(), SYSTX_MODE_IRREVOCABLE);
}

SYSTX_EXPORT
void
systx_release()
{
    struct tx* tx = get_tx(false);
    if (!tx) {
        return;
    }
    tx_release(tx);
}
