/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
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

#include "module.h"
#include <assert.h>
#include <errno.h>
#include <picotm/picotm-lib-spinlock.h>
#include <picotm/picotm-module.h>
#include <stdatomic.h>
#include <stdlib.h>
#include "cwd.h"
#include "cwd_tx.h"

/*
 * Global data
 */

static struct cwd* get_cwd(struct picotm_error* error);

static void
cwd_atexit_cb(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    struct cwd* cwd = get_cwd(&error);
    cwd_uninit(cwd);
}

static struct cwd*
get_cwd(struct picotm_error* error)
{
    static atomic_bool g_cwd_is_initialized;
    static struct cwd  g_cwd;

    if (atomic_load_explicit(&g_cwd_is_initialized, memory_order_acquire)) {
        return &g_cwd;
    }

    static struct picotm_spinlock lock = PICOTM_SPINLOCK_INITIALIZER;

    picotm_spinlock_lock(&lock);

    if (atomic_load_explicit(&g_cwd_is_initialized, memory_order_acquire)) {
        /* Another transaction initialized the cwd structure
         * concurrently; we're done. */
        goto out;
    }

    cwd_init(&g_cwd);

    atexit(cwd_atexit_cb); /* ignore errors */

    atomic_store_explicit(&g_cwd_is_initialized, true, memory_order_release);

out:
    picotm_spinlock_unlock(&lock);

    return &g_cwd;
};

/*
 * Module interface
 */

struct module {
    struct cwd_tx tx;

    /* True if module structure has been initialized, false otherwise */
    bool is_initialized;
};

static void
module_apply_event(struct module* module, unsigned short op, uintptr_t cookie,
                   struct picotm_error* error)
{
    cwd_tx_apply_event(&module->tx, op, cookie, error);
}

static void
module_undo_event(struct module* module, unsigned short op, uintptr_t cookie,
                  struct picotm_error* error)
{
    cwd_tx_undo_event(&module->tx, op, cookie, error);
}

static void
module_update_cc(struct module* module, struct picotm_error* error)
{
    cwd_tx_update_cc(&module->tx, error);
}

static void
module_clear_cc(struct module* module, struct picotm_error* error)
{
    cwd_tx_clear_cc(&module->tx, error);
}

static void
module_finish(struct module* module)
{
    cwd_tx_finish(&module->tx);
}

static void
module_uninit(struct module* module)
{
    cwd_tx_uninit(&module->tx);
    module->is_initialized = false;
}

/*
 * Thread-local data
 */

static void
apply_event_cb(unsigned short op, uintptr_t cookie, void* data,
               struct picotm_error* error)
{
    module_apply_event(data, op, cookie, error);
}

static void
undo_event_cb(unsigned short op, uintptr_t cookie, void* data,
              struct picotm_error* error)
{
    module_undo_event(data, op, cookie, error);
}

static void
update_cc_cb(void* data, int noundo, struct picotm_error* error)
{
    module_update_cc(data, error);
}

static void
clear_cc_cb(void* data, int noundo, struct picotm_error* error)
{
    module_clear_cc(data, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    module_finish(data);
}

static void
uninit_cb(void* data)
{
    module_uninit(data);
}

static struct cwd_tx*
get_cwd_tx(bool initialize, struct picotm_error* error)
{
    static __thread struct module t_module;

    if (t_module.is_initialized) {
        return &t_module.tx;
    } else if (!initialize) {
        return NULL;
    }

    unsigned long module = picotm_register_module(NULL, NULL,
                                                  NULL,
                                                  NULL, NULL,
                                                  apply_event_cb,
                                                  undo_event_cb,
                                                  update_cc_cb,
                                                  clear_cc_cb,
                                                  finish_cb,
                                                  uninit_cb,
                                                  &t_module,
                                                  error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct cwd* cwd = get_cwd(error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    cwd_tx_init(&t_module.tx, module, cwd);

    t_module.is_initialized = true;

    return &t_module.tx;
}

static struct cwd_tx*
get_non_null_cwd_tx(void)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        struct cwd_tx* cwd_tx = get_cwd_tx(true, &error);

        if (!picotm_error_is_set(&error)) {
            assert(cwd_tx);
            return cwd_tx;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

/*
 * Public interface
 */
int
cwd_module_chdir(const char* path, struct picotm_error* error)
{
    struct cwd_tx* cwd_tx = get_non_null_cwd_tx();

    return cwd_tx_chdir_exec(cwd_tx, path, error);
}

char*
cwd_module_getcwd(char* buf, size_t size, struct picotm_error* error)
{
    struct cwd_tx* cwd_tx = get_non_null_cwd_tx();

    return cwd_tx_getcwd_exec(cwd_tx, buf, size, error);
}

char*
cwd_module_realpath(const char* path, char* resolved_path,
                    struct picotm_error* error)
{
    struct cwd_tx* cwd_tx = get_non_null_cwd_tx();

    return cwd_tx_realpath_exec(cwd_tx, path, resolved_path, error);
}
