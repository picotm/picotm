/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann
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

#include "libc.h"
#include <stdatomic.h>
#include "error/module.h"
#include "picotm/picotm-module.h"
#include "signal/module.h"

/*
 * Error handling
 */

PICOTM_EXPORT
void
picotm_libc_save_errno()
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        error_module_save_errno(&error);
        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);
    } while (true);
}

PICOTM_EXPORT
void
picotm_libc_set_error_recovery(enum picotm_libc_error_recovery recovery)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        error_module_set_error_recovery(recovery, &error);
        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);
    } while (true);
}

PICOTM_EXPORT
enum picotm_libc_error_recovery
picotm_libc_get_error_recovery()
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        enum picotm_libc_error_recovery recovery =
            error_module_get_error_recovery(&error);
        if (!picotm_error_is_set(&error)) {
            return recovery;
        }
        picotm_recover_from_error(&error);
    } while (true);
}

/*
 * File I/O
 */

static _Atomic enum picotm_libc_cc_mode g_file_type_cc_mode[] = {
    PICOTM_LIBC_CC_MODE_2PL,
    PICOTM_LIBC_CC_MODE_2PL,
    PICOTM_LIBC_CC_MODE_2PL,
    PICOTM_LIBC_CC_MODE_2PL,
    PICOTM_LIBC_CC_MODE_2PL,
    PICOTM_LIBC_CC_MODE_2PL,
    PICOTM_LIBC_CC_MODE_2PL
};

PICOTM_EXPORT
void
picotm_libc_set_file_type_cc_mode(enum picotm_libc_file_type file_type,
                                  enum picotm_libc_cc_mode cc_mode)
{
    atomic_store_explicit(g_file_type_cc_mode + file_type, cc_mode,
                          memory_order_release);
}

PICOTM_EXPORT
enum picotm_libc_cc_mode
picotm_libc_get_file_type_cc_mode(enum picotm_libc_file_type file_type)
{
    return atomic_load_explicit(g_file_type_cc_mode + file_type,
                                memory_order_acquire);
}

/*
 * Signal handling
 */

PICOTM_EXPORT
void
picotm_libc_acquire_proc_signal(int signum,
                                void (*nontx_sigaction)(int, siginfo_t*,
                                                        void*),
                                struct picotm_error* error)
{
    signal_module_acquire_proc_signal(signum, nontx_sigaction, error);
}

PICOTM_EXPORT
void
picotm_libc_release_proc_signal(int signum, struct picotm_error* error)
{
    signal_module_release_proc_signal(signum, error);
}

PICOTM_EXPORT
void
picotm_libc_add_signal(int signum, _Bool is_recoverable,
                       struct picotm_error* error)
{
    signal_module_add_signal(signum, is_recoverable, error);
}

PICOTM_EXPORT
void
picotm_libc_remove_signal(int signum)
{
    signal_module_remove_signal(signum);
}

PICOTM_EXPORT
void
picotm_libc_clear_signals()
{
    signal_module_clear_signals();
}
