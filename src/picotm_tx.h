/*
 * picotm - A system-level transaction manager
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

#include "picotm/picotm.h" /* for __picotm_jmp_buf */
#include <stdbool.h>
#include "picotm_module.h"
#include "picotm_lock_owner.h"
#include "picotm_log.h"

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

#define MAX_NMODULES    (256)

struct picotm_error;
struct picotm_lock_manager;

enum picotm_tx_mode {
    TX_MODE_REVOCABLE,
    TX_MODE_IRREVOCABLE
};

struct picotm_tx {
    __picotm_jmp_buf*   env;
    struct picotm_log   log;
    enum picotm_tx_mode mode;
    unsigned long       nretries;

    /** The global lock manager for all transactions. */
    struct picotm_lock_manager* lm;

    /** The transaction's lock-owner instance. */
    struct picotm_lock_owner lo;

    unsigned long nmodules; /**< \brief Number allocated modules */
    struct picotm_module module[MAX_NMODULES]; /** \brief Registered modules */
};

void
picotm_tx_init(struct picotm_tx* self, struct picotm_lock_manager* lm,
               struct picotm_error* error);

void
picotm_tx_release(struct picotm_tx* self);

bool
picotm_tx_is_irrevocable(const struct picotm_tx* self);

unsigned long
picotm_tx_register_module(struct picotm_tx* self,
                          const struct picotm_module_ops* ops, void* data,
                          struct picotm_error* error);

void
picotm_tx_append_event(struct picotm_tx* self, unsigned long module,
                       uint16_t head, uintptr_t tail,
                       struct picotm_error* error);

void
picotm_tx_begin(struct picotm_tx* self, enum picotm_tx_mode mode,
                bool is_retry, __picotm_jmp_buf* env,
                struct picotm_error* error);

void
picotm_tx_commit(struct picotm_tx* self, struct picotm_error* error);

void
picotm_tx_rollback(struct picotm_tx* self, struct picotm_error* error);
