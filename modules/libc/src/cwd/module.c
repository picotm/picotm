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
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-global-state.h"
#include "picotm/picotm-lib-shared-state.h"
#include "picotm/picotm-module.h"
#include "cwd.h"
#include "cwd_log.h"
#include "cwd_tx.h"

/*
 * Shared state
 */

static void
init_cwd_shared_state_fields(struct cwd* cwd, struct picotm_error* error)
{
    cwd_init(cwd);
}

static void
uninit_cwd_shared_state_fields(struct cwd* cwd)
{
    cwd_uninit(cwd);
}

PICOTM_SHARED_STATE(cwd, struct cwd);
PICOTM_SHARED_STATE_STATIC_IMPL(cwd, struct cwd,
                                init_cwd_shared_state_fields,
                                uninit_cwd_shared_state_fields)

/*
 * Global state
 */

PICOTM_GLOBAL_STATE_STATIC_IMPL(cwd)

/*
 * Module interface
 */

struct module {
    struct cwd_log log;
    struct cwd_tx tx;

    /* True if module structure has been initialized, false otherwise */
    bool is_initialized;
};

static void
module_apply_event(struct module* module, uint16_t head, uintptr_t tail,
                   struct picotm_error* error)
{
    assert(module);

    const struct cwd_event* event = cwd_log_at(&module->log, tail);

    cwd_tx_apply_event(&module->tx, head, event->alloced, error);
}

static void
module_undo_event(struct module* module, uint16_t head, uintptr_t tail,
                  struct picotm_error* error)
{
    assert(module);

    const struct cwd_event* event = cwd_log_at(&module->log, tail);

    cwd_tx_undo_event(&module->tx, head, event->alloced, error);
}

static void
module_finish(struct module* module)
{
    cwd_tx_finish(&module->tx);
    cwd_log_clear(&module->log);
}

static void
module_uninit(struct module* module)
{
    cwd_tx_uninit(&module->tx);
    cwd_log_uninit(&module->log);

    module->is_initialized = false;

    PICOTM_GLOBAL_STATE_UNREF(cwd);
}

/*
 * Thread-local data
 */

static void
apply_event_cb(uint16_t head, uintptr_t tail, void* data,
               struct picotm_error* error)
{
    module_apply_event(data, head, tail, error);
}

static void
undo_event_cb(uint16_t head, uintptr_t tail, void* data,
              struct picotm_error* error)
{
    module_undo_event(data, head, tail, error);
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
    static const struct picotm_module_ops g_ops = {
        .apply_event = apply_event_cb,
        .undo_event = undo_event_cb,
        .finish = finish_cb,
        .uninit = uninit_cb
    };
    static __thread struct module t_module;

    if (t_module.is_initialized) {
        return &t_module.tx;
    } else if (!initialize) {
        return NULL;
    }

    struct cwd* cwd = PICOTM_GLOBAL_STATE_REF(cwd, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    unsigned long module = picotm_register_module(&g_ops, &t_module, error);
    if (picotm_error_is_set(error)) {
        goto err_picotm_register_module;
    }

    cwd_log_init(&t_module.log, module);
    cwd_tx_init(&t_module.tx, &t_module.log, cwd);

    t_module.is_initialized = true;

    return &t_module.tx;

err_picotm_register_module:
    PICOTM_GLOBAL_STATE_UNREF(cwd);
    return NULL;
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
