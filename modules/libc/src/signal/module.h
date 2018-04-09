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
