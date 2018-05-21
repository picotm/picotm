/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
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
