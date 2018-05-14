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
#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-shared-ref-obj.h"
#include "picotm/picotm-lib-spinlock.h"
#include "picotm/picotm-module.h"
#include "cwd.h"
#include "cwd_log.h"
#include "cwd_tx.h"

/*
 * Shared state
 */

struct cwd_shared_state {
    struct picotm_shared_ref16_obj ref_obj;
    struct cwd cwd;
};

#define CWD_SHARED_STATE_INITIALIZER                \
{                                                   \
    .ref_obj = PICOTM_SHARED_REF16_OBJ_INITIALIZER  \
}

static void
init_cwd_shared_state_fields(struct cwd_shared_state* shared)
{
    cwd_init(&shared->cwd);
}

static void
uninit_cwd_shared_state_fields(struct cwd_shared_state* shared)
{
    cwd_uninit(&shared->cwd);
}

static void
first_ref_cwd_shared_state_cb(struct picotm_shared_ref16_obj* ref_obj,
                              void* data, struct picotm_error* error)
{
    struct cwd_shared_state* shared =
        picotm_containerof(ref_obj, struct cwd_shared_state, ref_obj);
    init_cwd_shared_state_fields(shared);
}

static void
cwd_shared_state_ref(struct cwd_shared_state* self,
                     struct picotm_error* error)
{
    picotm_shared_ref16_obj_up(&self->ref_obj, NULL, NULL,
                               first_ref_cwd_shared_state_cb,
                               error);
    if (picotm_error_is_set(error)) {
        return;
    }
};

static void
final_ref_cwd_shared_state_cb(struct picotm_shared_ref16_obj* ref_obj,
                              void* data, struct picotm_error* error)
{
    struct cwd_shared_state* shared =
        picotm_containerof(ref_obj, struct cwd_shared_state, ref_obj);
    uninit_cwd_shared_state_fields(shared);
}

static void
cwd_shared_state_unref(struct cwd_shared_state* self)
{
    picotm_shared_ref16_obj_down(&self->ref_obj, NULL, NULL,
                                 final_ref_cwd_shared_state_cb);
}

/*
 * Global data
 */

/* Returns the statically allocated global state. Callers *must* already
 * hold a reference. */
static struct cwd_shared_state*
get_cwd_global_state(void)
{
    static struct cwd_shared_state s_global = CWD_SHARED_STATE_INITIALIZER;
    return &s_global;
}

static struct cwd_shared_state*
ref_cwd_global_state(struct picotm_error* error)
{
    struct cwd_shared_state* global = get_cwd_global_state();

    cwd_shared_state_ref(global, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return global;
};

static void
unref_cwd_global_state(void)
{
    struct cwd_shared_state* global = get_cwd_global_state();
    cwd_shared_state_unref(global);
}

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

    unref_cwd_global_state();
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

    unsigned long module = picotm_register_module(&g_ops, &t_module, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct cwd_shared_state* global = ref_cwd_global_state(error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    cwd_log_init(&t_module.log, module);
    cwd_tx_init(&t_module.tx, &t_module.log, &global->cwd);

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
