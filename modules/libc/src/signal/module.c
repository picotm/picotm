/*
 * MIT License
 * Copyright (c) 2018   Thomas Zimmermann <tdz@users.sourceforge.net>
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
#include "picotm/picotm-lib-state.h"
#include "picotm/picotm-lib-thread-state.h"
#include "picotm/picotm-module.h"
#include "signal_tx.h"
#include "sigstate.h"

/*
 * Module interface
 */

struct module {
    struct signal_tx tx;
};

static void
module_begin(struct module* module, struct picotm_error* error)
{
    signal_tx_begin(&module->tx, error);
}

static void
module_finish(struct module* module, struct picotm_error* error)
{
    signal_tx_finish(&module->tx, error);
}

static void
module_uninit(struct module* module)
{
    signal_tx_uninit(&module->tx);
}

/*
 * Thread-local data
 */

PICOTM_STATE(signal_module, struct module);
PICOTM_STATE_STATIC_DECL(signal_module, struct module)
PICOTM_THREAD_STATE_STATIC_DECL(signal_module)

static void
begin_cb(void* data, struct picotm_error* error)
{
    module_begin(data, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    module_finish(data, error);
}

static void
uninit_cb(void* data)
{
    PICOTM_THREAD_STATE_RELEASE(signal_module);
}

static void
init_signal_module(struct module* module, struct picotm_error* error)
{
    static const struct picotm_module_ops s_ops = {
        .begin = begin_cb,
        .finish = finish_cb,
        .uninit = uninit_cb
    };

    unsigned long module_id = picotm_register_module(&s_ops, module, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    signal_tx_init(&module->tx, module_id);
}

static void
uninit_signal_module(struct module* module)
{
    module_uninit(module);
}

PICOTM_STATE_STATIC_IMPL(signal_module, struct module,
                         init_signal_module,
                         uninit_signal_module)
PICOTM_THREAD_STATE_STATIC_IMPL(signal_module)

static struct signal_tx*
get_signal_tx(bool initialize, struct picotm_error* error)
{
    struct module* module = PICOTM_THREAD_STATE_ACQUIRE(signal_module,
                                                        initialize,
                                                        error);
    if (picotm_error_is_set(error)) {
        return NULL;
    } else if (!module) {
        return NULL; /* not yet initialized */
    }
    return &module->tx;
}

/*
 * Public interface
 */

void
tx_signal_handler(int signum, siginfo_t* info, void* ucontext)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    struct signal_tx* tx = get_signal_tx(false, &error);
    if (picotm_error_is_set(&error)) {  /* This should not be possible. */
        return;
    } else if (!tx) {
        return;
    }

    signal_tx_recover_from_signal(tx, info);
}

void
signal_module_acquire_proc_signal(int signum,
                                  void (*nontx_sigaction)(int, siginfo_t*,
                                                          void*),
                                  struct picotm_error* error)
{
    sigstate_acquire_proc_signal(signum, nontx_sigaction, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
signal_module_release_proc_signal(int signum, struct picotm_error* error)
{
    sigstate_release_proc_signal(signum, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
signal_module_add_signal(int signum, bool is_recoverable,
                         struct picotm_error* error)
{
    struct signal_tx* tx = get_signal_tx(true, error);
    if (picotm_error_is_set(error)) {
        return;
    }
    signal_tx_add_signal(tx, signum, is_recoverable, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
signal_module_remove_signal(int signum)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    struct signal_tx* tx = get_signal_tx(false, &error);
    /* no errors possible */

    if (!tx) {
        return;
    }
    signal_tx_remove_signal(tx, signum);
}

void
signal_module_clear_signals()
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    struct signal_tx* tx = get_signal_tx(false, &error);
    /* no errors possible */

    if (!tx) {
        return;
    }
    signal_tx_clear_signals(tx);
}
