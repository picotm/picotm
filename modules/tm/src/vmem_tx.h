/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "picotm/picotm-lib-slist.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * \cond impl || tm_impl
 * \ingroup tm_impl
 * \file
 * \endcond
 */

struct picotm_error;
struct tm_page;
struct tm_vmem;
struct tm_vmem_tx;

/**
 * |struct tm_vmem_tx| represents a memory transaction.
 */
struct tm_vmem_tx {
    /* pointer to global vmem structure */
    struct tm_vmem* vmem;

    /* module index */
    unsigned long module;

    /* page-allocator fields */
    struct picotm_slist active_pages;
    struct picotm_slist alloced_pages;
};

/**
 * Initializes a memory transaction.
 */
void
tm_vmem_tx_init(struct tm_vmem_tx* vmem_tx, struct tm_vmem* vmem,
                unsigned long module);

/**
 * Releases all memory and resources allocated by the transaction.
 */
void
tm_vmem_tx_uninit(struct tm_vmem_tx* vmem_tx);

/**
 * Executes a load operation.
 */
void
tm_vmem_tx_ld(struct tm_vmem_tx* vmem_tx, uintptr_t addr, void* buf,
              size_t siz, struct picotm_error* error);

/**
 * Executes a store operation.
 */
void
tm_vmem_tx_st(struct tm_vmem_tx* vmem_tx, uintptr_t addr, const void* buf,
              size_t siz, struct picotm_error* error);

/**
 * Executes a load-store operation.
 */
void
tm_vmem_tx_ldst(struct tm_vmem_tx* vmem_tx, uintptr_t laddr, uintptr_t saddr,
                size_t siz, struct picotm_error* error);

/**
 * Privatizes a region of memory.
 */
void
tm_vmem_tx_privatize(struct tm_vmem_tx* vmem_tx, uintptr_t addr, size_t siz,
                     unsigned long flags, struct picotm_error* error);

/**
 * Privatizes a region of memory.
 */
void
tm_vmem_tx_privatize_c(struct tm_vmem_tx* vmem_tx, uintptr_t addr, int c,
                       unsigned long flags, struct picotm_error* error);

/**
 * Applies all transaction-local changes to main memory.
 */
void
tm_vmem_tx_apply(struct tm_vmem_tx* vmem_tx, struct picotm_error* error);

/**
 * Reverts all transaction-local changes.
 */
void
tm_vmem_tx_undo(struct tm_vmem_tx* vmem_tx, struct picotm_error* error);

/**
 * Cleans up a transcation's resources. The transaction's data structure
 * will remain initialized and useable after this call returned.
 */
void
tm_vmem_tx_finish(struct tm_vmem_tx* vmem_tx, struct picotm_error* error);
