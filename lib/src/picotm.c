/* Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "picotm.h"
#include <errno.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "tx.h"
#include "tx_shared.h"

/*
 * Global data
 */

static struct tx_shared*
get_tx_shared(void)
{
    static struct tx_shared g_tx_shared;
    static atomic_bool      g_tx_shared_is_initialized;

    if (atomic_load_explicit(&g_tx_shared_is_initialized, memory_order_acquire)) {
        return &g_tx_shared;
    }

    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    int res = pthread_mutex_lock(&lock);
    if (res) {
        return NULL;
    }

    if (atomic_load_explicit(&g_tx_shared_is_initialized, memory_order_acquire)) {
        /* Another transaction initialized the tx_state structure
         * concurrently; we're done. */
        goto out;
    }

    res = tx_shared_init(&g_tx_shared);
    if (res < 0) {
        goto err_tx_shared_init;
    }

    atomic_store_explicit(&g_tx_shared_is_initialized, true, memory_order_release);

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
        case PICOTM_REVOCABLE:
            /* This should not happen. */
            mode = PICOTM_MODE_IRREVOCABLE;
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
        case PICOTM_REVOCABLE:
            restart_tx(tx, PICOTM_MODE_IRREVOCABLE);
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
                       picotm_module_apply_function apply,
                       picotm_module_undo_function undo,
                       picotm_module_apply_event_function apply_event,
                       picotm_module_undo_event_function undo_event,
                       picotm_module_update_cc_function update_cc,
                       picotm_module_clear_cc_function clear_cc,
                       picotm_module_finish_function finish,
                       picotm_module_uninit_function uninit,
                       void* data,
                       struct picotm_error* error)
{
    return tx_register_module(get_non_null_tx(), lock, unlock, is_valid,
                              apply, undo, apply_event, undo_event,
                              update_cc, clear_cc,
                              finish, uninit, data, error);
}

PICOTM_EXPORT
void
picotm_append_event(unsigned long module, unsigned long op, uintptr_t cookie,
                    struct picotm_error* error)
{
    tx_append_event(get_non_null_tx(), module, op, cookie, error);
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
    case PICOTM_REVOCABLE:
        restart_tx(get_non_null_tx(), PICOTM_MODE_IRREVOCABLE);
        break;
    case PICOTM_ERROR_CODE:
        restart_tx(get_non_null_tx(), PICOTM_MODE_RECOVERY);
        break;
    case PICOTM_ERRNO:
        restart_tx(get_non_null_tx(), PICOTM_MODE_RECOVERY);
        break;
    }
}
