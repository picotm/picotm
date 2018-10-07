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
