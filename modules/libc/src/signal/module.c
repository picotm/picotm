/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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

struct signal_module {
    struct signal_tx tx;
};

static void
signal_module_init(struct signal_module* self, unsigned long module_id)
{
    assert(self);

    signal_tx_init(&self->tx, module_id);
}

static void
signal_module_uninit(struct signal_module* self)
{
    assert(self);

    signal_tx_uninit(&self->tx);
}

static void
signal_module_begin(struct signal_module* self, struct picotm_error* error)
{
    assert(self);

    signal_tx_begin(&self->tx, error);
}

static void
signal_module_finish(struct signal_module* self, struct picotm_error* error)
{
    assert(self);

    signal_tx_finish(&self->tx, error);
}

/*
 * Thread-local data
 */

PICOTM_STATE(signal_module, struct signal_module);
PICOTM_STATE_STATIC_DECL(signal_module, struct signal_module)
PICOTM_THREAD_STATE_STATIC_DECL(signal_module)

static void
begin_cb(void* data, struct picotm_error* error)
{
    struct signal_module* module = data;
    signal_module_begin(module, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    struct signal_module* module = data;
    signal_module_finish(module, error);
}

static void
release_cb(void* data)
{
    PICOTM_THREAD_STATE_RELEASE(signal_module);
}

static void
init_signal_module(struct signal_module* module, struct picotm_error* error)
{
    static const struct picotm_module_ops s_ops = {
        .begin = begin_cb,
        .finish = finish_cb,
        .release = release_cb
    };

    unsigned long module_id = picotm_register_module(&s_ops, module, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    signal_module_init(module, module_id);
}

static void
uninit_signal_module(struct signal_module* module)
{
    signal_module_uninit(module);
}

PICOTM_STATE_STATIC_IMPL(signal_module, struct signal_module,
                         init_signal_module,
                         uninit_signal_module)
PICOTM_THREAD_STATE_STATIC_IMPL(signal_module)

static struct signal_tx*
get_signal_tx(bool initialize, struct picotm_error* error)
{
    struct signal_module* module = PICOTM_THREAD_STATE_ACQUIRE(signal_module,
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
