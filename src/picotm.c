/**
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
 *
  Permission is hereby granted, free of charge, to any person obtaining a
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
 *
 * SPDX-License-Identifier: MIT
 */

#include "picotm.h"
#include <assert.h>
#include <errno.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-spinlock.h"
#include "picotm_lock_manager.h"
#include "tx.h"

/*
 * Global data
 */

struct global_state {
    /**
     * The global lock manager for all transactional locks. Register your
     * transaction's lock-owner instance with this object.
     */
    struct picotm_lock_manager lm;
};

static void
init_global_state_fields(struct global_state* global,
                         struct picotm_error* error)
{
    assert(global);

    picotm_lock_manager_init(&global->lm, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
uninit_global_state_fields(struct global_state* global)
{
    assert(global);

    picotm_lock_manager_uninit(&global->lm);
}

static struct global_state*
get_global_state(bool initialize, struct picotm_error* error);

static void
atexit_global_state(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    struct global_state* global = get_global_state(false, &error);
    if (picotm_error_is_set(&error)) {
        return;
    } else if (global) {
        uninit_global_state_fields(global);
    }
}

static struct global_state*
get_global_state(bool initialize, struct picotm_error* error)
{
    static struct {
        atomic_bool is_initialized;
        struct picotm_spinlock lock;
        struct global_state global;
    } g_state = {
        .is_initialized = ATOMIC_VAR_INIT(false),
        .lock = PICOTM_SPINLOCK_INITIALIZER
    };

    bool is_initialized = atomic_load_explicit(&g_state.is_initialized,
                                               memory_order_acquire);
    if (is_initialized) {
        return &g_state.global;
    } else if (!initialize) {
        return NULL;
    }

    picotm_spinlock_lock(&g_state.lock);

    is_initialized = atomic_load_explicit(&g_state.is_initialized,
                                          memory_order_acquire);
    if (is_initialized) {
        /* Another transaction initialized the global state
         * concurrently; we're done. */
        goto out;
    }

    init_global_state_fields(&g_state.global, error);
    if (picotm_error_is_set(error)) {
        goto err_init_global_state_fields;
    }

    atexit(atexit_global_state);

    atomic_store_explicit(&g_state.is_initialized, true,
                          memory_order_release);

out:
    picotm_spinlock_unlock(&g_state.lock);

    return &g_state.global;

err_init_global_state_fields:
    picotm_spinlock_unlock(&g_state.lock);
    return NULL;
}

/*
 * Thread-local data
 */

struct thread_state {
    /** The thread-local transaction state. */
    struct tx tx;
};

static void
init_thread_state_fields(struct thread_state* thread,
                         struct global_state* global,
                         struct picotm_error* error)
{
    assert(thread);
    assert(global);

    tx_init(&thread->tx, &global->lm, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
uninit_thread_state_fields(struct thread_state* thread)
{
    assert(thread);

    tx_release(&thread->tx);
}

static struct thread_state*
get_thread_state(bool initialize, struct picotm_error* error)
{
    static __thread struct {
        bool is_initialized;
        struct thread_state thread;
    } t_state = {
        .is_initialized = false
    };

    if (t_state.is_initialized) {
        return &t_state.thread;
    } else if (!initialize) {
        return NULL;
    }

    struct global_state* global = get_global_state(true, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    init_thread_state_fields(&t_state.thread, global, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    t_state.is_initialized = true;

    return &t_state.thread;
}

static struct tx*
get_tx(bool initialize, struct picotm_error* error)
{
    struct thread_state* thread = get_thread_state(initialize, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    } else if (!thread) {
        return NULL;
    }
    return &thread->tx;
}

static struct tx*
get_non_null_tx(void)
{
    while (true) {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        struct tx* tx = get_tx(true, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            continue;
        }
        assert(tx);
        return tx;
    };
}

static struct picotm_error*
get_non_null_error(void)
{
    static __thread struct picotm_error t_error = PICOTM_ERROR_INITIALIZER;

    return &t_error;
}

/*
 * Lock-owner look-up functions
 */

struct picotm_lock_owner*
picotm_lock_owner_get_thread_local_instance()
{
    struct tx* tx = get_non_null_tx();

    return &tx->lo;
}

struct picotm_lock_manager*
picotm_lock_owner_get_lock_manager(struct picotm_lock_owner* lo)
{
    assert(lo);

    const struct tx* tx = picotm_containerof(lo, struct tx, lo);

    return tx->lm;
}

/*
 * Public interface
 */

PICOTM_EXPORT
_Bool
__picotm_begin(enum __picotm_mode mode, jmp_buf* env)
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

    struct tx* tx = get_tx(true, error);
    if (picotm_error_is_set(error)) {
        return false; /* Enter recovery mode. */
    }

    tx_begin(tx, tx_mode[mode], mode == PICOTM_MODE_RETRY, env, error);
    if (picotm_error_is_set(error)) {
        return false; /* Enter recovery mode. */
    }

    return true;
}

static void
restart_tx(struct tx* tx, enum __picotm_mode mode, bool do_rollback)
{
    assert(tx);

    if (do_rollback) {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        tx_rollback(tx, &error);
        if (picotm_error_is_set(&error)) {
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
            case PICOTM_KERN_RETURN_T:
                /* If we were restarting before, we're now recovering. */
                mode = PICOTM_MODE_RECOVERY;
                break;
            }
        }
    }

    /* Restarting the transaction here transfers control
     * to __picotm_begin(). */
    longjmp(*(tx->env), (int)mode);
}

PICOTM_EXPORT
void
__picotm_commit()
{
    struct tx* tx = get_non_null_tx();

    struct picotm_error* error = get_non_null_error();
    tx_commit(tx, error);
    if (picotm_error_is_set(error)) {
        switch (error->status) {
        case PICOTM_CONFLICTING:
            restart_tx(tx, PICOTM_MODE_RETRY, true);
            break;
        case PICOTM_REVOCABLE:
            restart_tx(tx, PICOTM_MODE_IRREVOCABLE, true);
            break;
        case PICOTM_ERROR_CODE:
        case PICOTM_ERRNO:
        case PICOTM_KERN_RETURN_T:
            restart_tx(tx, PICOTM_MODE_RECOVERY, true);
            break;
        }
    }
}

PICOTM_EXPORT
void
picotm_restart()
{
    restart_tx(get_non_null_tx(), PICOTM_MODE_RETRY, false);
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
    restart_tx(get_non_null_tx(), PICOTM_MODE_IRREVOCABLE, true);
}

PICOTM_EXPORT
bool
picotm_is_irrevocable()
{
    return tx_is_irrevocable(get_non_null_tx());
}

PICOTM_EXPORT
unsigned long
picotm_number_of_restarts()
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    struct tx* tx = get_tx(false, &error);
    if (picotm_error_is_set(&error)) {
        return 0;
    } else if (!tx) {
        return 0; /* thread executed no transaction; not an error */
    }
    return tx->nretries;
}

PICOTM_EXPORT
void
picotm_release()
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    struct thread_state* thread = get_thread_state(false, &error);
    if (picotm_error_is_set(&error)) {
        return;
    } else if (!thread) {
        return; /* thread executed no transaction; not an error */
    }
    uninit_thread_state_fields(thread);
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

#if defined(PICOTM_HAVE_TYPE_KERN_RETURN_T) && PICOTM_HAVE_TYPE_KERN_RETURN_T
PICOTM_EXPORT
kern_return_t
picotm_error_as_kern_return_t()
{
    return get_non_null_error()->value.kern_return_t_value;
}
#endif

/*
 * Module interface
 */

PICOTM_EXPORT
unsigned long
picotm_register_module(const struct picotm_module_ops* ops, void* data,
                       struct picotm_error* error)
{
    return tx_register_module(get_non_null_tx(), ops, data, error);
}

PICOTM_EXPORT
void
picotm_append_event(unsigned long module, uint16_t head, uintptr_t tail,
                    struct picotm_error* error)
{
    tx_append_event(get_non_null_tx(), module, head, tail, error);
}

PICOTM_EXPORT
void
picotm_resolve_conflict(struct picotm_rwlock* conflicting_lock)
{
    picotm_error_set_conflicting(get_non_null_error(), conflicting_lock);

    restart_tx(get_non_null_tx(), PICOTM_MODE_RETRY, true);
}

PICOTM_EXPORT
void
picotm_recover_from_error_code(enum picotm_error_code error_hint)
{
    picotm_error_set_error_code(get_non_null_error(), error_hint);

    /* Nothing we can do on errors; let's try to recover. */
    restart_tx(get_non_null_tx(), PICOTM_MODE_RECOVERY, true);
}

PICOTM_EXPORT
void
picotm_recover_from_errno(int errno_hint)
{
    picotm_error_set_errno(get_non_null_error(), errno_hint);

    /* Nothing we can do on errors; let's try to recover. */
    restart_tx(get_non_null_tx(), PICOTM_MODE_RECOVERY, true);
}

#if defined(PICOTM_HAVE_TYPE_KERN_RETURN_T) && PICOTM_HAVE_TYPE_KERN_RETURN_T
PICOTM_EXPORT
void
picotm_recover_from_kern_return_t(kern_return_t value)
{
    picotm_error_set_kern_return_t(get_non_null_error(), value);

    /* Nothing we can do on errors; let's try to recover. */
    restart_tx(get_non_null_tx(), PICOTM_MODE_RECOVERY, true);
}
#endif

PICOTM_EXPORT
void
picotm_recover_from_error(const struct picotm_error* error)
{
    assert(error);

    memcpy(get_non_null_error(), error, sizeof(*error));

    switch (error->status) {
    case PICOTM_CONFLICTING:
        restart_tx(get_non_null_tx(), PICOTM_MODE_RETRY, true);
        break;
    case PICOTM_REVOCABLE:
        restart_tx(get_non_null_tx(), PICOTM_MODE_IRREVOCABLE, true);
        break;
    case PICOTM_ERROR_CODE:
        restart_tx(get_non_null_tx(), PICOTM_MODE_RECOVERY, true);
        break;
    case PICOTM_ERRNO:
        restart_tx(get_non_null_tx(), PICOTM_MODE_RECOVERY, true);
        break;
    case PICOTM_KERN_RETURN_T:
        restart_tx(get_non_null_tx(), PICOTM_MODE_RECOVERY, true);
        break;
    }
}
