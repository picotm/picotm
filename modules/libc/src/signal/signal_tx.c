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

#include "signal_tx.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"
#include "picotm/picotm-module.h"
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>

void
signal_tx_init(struct signal_tx* self, unsigned long module)
{
    self->module = module;

    self->tx_is_running = false;

    volatile sig_atomic_t* beg = picotm_arraybeg(self->signal_state);
    volatile sig_atomic_t* end = picotm_arrayend(self->signal_state);

    while (beg < end) {
        *beg = 0;
        ++beg;
    }
}

void
signal_tx_uninit(struct signal_tx* self)
{ }

void
signal_tx_add_signal(struct signal_tx* self, int signum, bool is_recoverable,
                     struct picotm_error* error)
{
    if (signum < 0) {
        picotm_error_set_errno(error, EINVAL);
        return;
    } else if (signum >= picotm_arraylen(self->signal_state)) {
        picotm_error_set_errno(error, EINVAL);
        return;
    }
    if (is_recoverable) {
        self->signal_state[signum] = 1;
    } else {
        self->signal_state[signum] = 2;
    }
}

void
signal_tx_remove_signal(struct signal_tx* self, int signum)
{
    if (signum < 0) {
        return;
    } else if (signum >= picotm_arraylen(self->signal_state)) {
        return;
    }
    self->signal_state[signum] = 0;
}

void
signal_tx_clear_signals(struct signal_tx* self)
{
    volatile sig_atomic_t* beg = picotm_arraybeg(self->signal_state);
    volatile sig_atomic_t* end = picotm_arrayend(self->signal_state);

    while (beg < end) {
        *beg = 0;
        ++beg;
    }
}

void
signal_tx_recover_from_signal(struct signal_tx* self, const siginfo_t* info)
{
    if (!self->tx_is_running) {
        return;
    } else if (info->si_signo < 0) {
        return;
    } else if (info->si_signo >= picotm_arraylen(self->signal_state)) {
        return;
    } else if (!self->signal_state[info->si_signo]) {
        return;
    }

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    picotm_error_set_siginfo_t(&error, info);
    if (self->signal_state[info->si_signo] == 2) {
        picotm_error_mark_as_non_recoverable(&error);
    }
    picotm_recover_from_error(&error);
}

void
signal_tx_begin(struct signal_tx* self, struct picotm_error* error)
{
    self->tx_is_running = true;
}

void
signal_tx_finish(struct signal_tx* self, struct picotm_error* error)
{
    self->tx_is_running = false;
}
