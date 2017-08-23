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

#include "module.h"
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
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
    static atomic_bool    g_vmem_is_initialized;
    static struct tm_vmem g_vmem;

    if (atomic_load_explicit(&g_vmem_is_initialized, memory_order_acquire)) {
        return &g_vmem;
    }

    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    int res = pthread_mutex_lock(&lock);
    if (res) {
        picotm_error_set_errno(error, res);
        return NULL;
    }

    if (atomic_load_explicit(&g_vmem_is_initialized, memory_order_acquire)) {
        /* Another transaction initialized the vmem structure
         * concurrently; we're done. */
        goto out;
    }

    tm_vmem_init(&g_vmem);

    atexit(vmem_atexit_cb); /* ignore errors */

    atomic_store_explicit(&g_vmem_is_initialized, true, memory_order_release);

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
apply(struct tm_module* module, struct picotm_error* error)
{
    tm_vmem_tx_apply(&module->tx, error);
}

static void
undo(struct tm_module* module, struct picotm_error* error)
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

static void
validate_cb(void* data, int eotx, struct picotm_error* error)
{
    validate(data, !!eotx, error);
}

static void
apply_cb(void* data, struct picotm_error* error)
{
    apply(data, error);
}

static void
undo_cb(void* data, struct picotm_error* error)
{
    undo(data, error);
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
                                                  validate_cb,
                                                  apply_cb, undo_cb,
                                                  NULL, NULL,
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
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        struct tm_vmem_tx* vmem_tx = get_vmem_tx(true, &error);

        if (!picotm_error_is_set(&error)) {
            assert(vmem_tx);
            return vmem_tx;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

/*
 * Public interface
 */

void
tm_module_load(uintptr_t addr, void* buf, size_t siz)
{
    struct tm_vmem_tx* vmem_tx = get_non_null_vmem_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        tm_vmem_tx_ld(vmem_tx, addr, buf, siz, &error);

        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

void
tm_module_store(uintptr_t addr, const void* buf, size_t siz)
{
    struct tm_vmem_tx* vmem_tx = get_non_null_vmem_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        tm_vmem_tx_st(vmem_tx, addr, buf, siz, &error);

        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

void
tm_module_loadstore(uintptr_t laddr, uintptr_t saddr, size_t siz)
{
    struct tm_vmem_tx* vmem_tx = get_non_null_vmem_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        tm_vmem_tx_ldst(vmem_tx, laddr, saddr, siz, &error);

        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

void
tm_module_privatize(uintptr_t addr, size_t siz, unsigned long flags)
{
    struct tm_vmem_tx* vmem_tx = get_non_null_vmem_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        tm_vmem_tx_privatize(vmem_tx, addr, siz, flags, &error);

        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

void
tm_module_privatize_c(uintptr_t addr, int c, unsigned long flags)
{
    struct tm_vmem_tx* vmem_tx = get_non_null_vmem_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        tm_vmem_tx_privatize_c(vmem_tx, addr, c, flags, &error);

        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}
