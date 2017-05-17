/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm.h"
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
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
    static __thread struct picotm_error t_error = PICOTM_ERROR_INITIALIZER;

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

    /* We (re-)start a transaction. Clear the old error state. */
    struct picotm_error* error = get_non_null_error();
    memset(error, 0, sizeof(*error));

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
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    int res = tx_rollback(tx, &error);
    if (res < 0) {
        switch (error.status) {
        case PICOTM_CONFLICTING:
            /* Should be avoided, but no problem per se. */
            break;
        case PICOTM_ERROR_CODE:
        case PICOTM_ERRNO:
            /* If we were restarting before, we're now recovering. */
            mode = PICOTM_MODE_RECOVERY;
            break;
        }
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

    struct picotm_error* error = get_non_null_error();
    int res = tx_commit(tx, error);
    if (res < 0) {
        switch (error->status) {
        case PICOTM_CONFLICTING:
            restart_tx(tx, PICOTM_MODE_RETRY);
            break;
        case PICOTM_ERROR_CODE:
        case PICOTM_ERRNO:
            restart_tx(tx, PICOTM_MODE_RECOVERY);
            break;
        }
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

PICOTM_EXPORT
enum picotm_error_status
picotm_error_status()
{
    return get_non_null_error()->status;
}

PICOTM_EXPORT
bool
picotm_error_is_non_recoverable()
{
    return get_non_null_error()->is_non_recoverable;
}

PICOTM_EXPORT
enum picotm_error_code
picotm_error_as_error_code()
{
    return get_non_null_error()->value.error_hint;
}

PICOTM_EXPORT
int
picotm_error_as_errno()
{
    return get_non_null_error()->value.errno_hint;
}

/*
 * Module interface
 */

PICOTM_EXPORT
unsigned long
picotm_register_module(picotm_module_lock_function lock,
                       picotm_module_unlock_function unlock,
                       picotm_module_is_valid_function is_valid,
                       picotm_module_apply_events_function apply_events,
                       picotm_module_undo_events_function undo_events,
                       picotm_module_update_cc_function update_cc,
                       picotm_module_clear_cc_function clear_cc,
                       picotm_module_finish_function finish,
                       picotm_module_uninit_function uninit,
                       void* data,
                       struct picotm_error* error)
{
    return tx_register_module(get_non_null_tx(), lock, unlock, is_valid,
                              apply_events, undo_events, update_cc, clear_cc,
                              finish, uninit, data, error);
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

PICOTM_EXPORT
void
picotm_recover_from_error(const struct picotm_error* error)
{
    memcpy(get_non_null_error(), error, sizeof(*error));

    switch (error->status) {
    case PICOTM_CONFLICTING:
        restart_tx(get_non_null_tx(), PICOTM_MODE_RETRY);
        break;
    case PICOTM_ERROR_CODE:
        restart_tx(get_non_null_tx(), PICOTM_MODE_RECOVERY);
        break;
    case PICOTM_ERRNO:
        restart_tx(get_non_null_tx(), PICOTM_MODE_RECOVERY);
        break;
    }
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
