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

#pragma once

#include <signal.h>
#include <stdbool.h>

/**
 * \cond impl || libc_impl || libc_impl_signal
 * \ingroup libc_impl
 * \ingroup libc_impl_signal
 * \file
 * \endcond
 */

struct picotm_error;

void
signal_module_acquire_proc_signal(int signum,
                                  void (*nontx_sigaction)(int, siginfo_t*,
                                                          void*),
                                  struct picotm_error* error);

void
signal_module_release_proc_signal(int signum, struct picotm_error* error);

void
signal_module_add_signal(int signum, bool is_recoverable,
                         struct picotm_error* error);

void
signal_module_remove_signal(int signum);

void
signal_module_clear_signals(void);
