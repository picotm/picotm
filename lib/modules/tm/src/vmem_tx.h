/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/queue.h>

struct picotm_error;
struct tm_page;
struct tm_vmem;
struct tm_vmem_tx;

SLIST_HEAD(tm_page_list, tm_page);

/*
 * |struct tm_vmem_tx|
 */

/**
 * |struct tm_vmem_tx| represents a memory transaction.
 */
struct tm_vmem_tx {
    /* pointer to global vmem structure */
    struct tm_vmem* vmem;

    /* module index */
    unsigned long module;

    /* page-allocator fields */
    struct tm_page_list active_pages;
    struct tm_page_list alloced_pages;
};

/**
 * Initializes a memory transaction.
 */
int tm_vmem_tx_init(struct tm_vmem_tx* vmem_tx, struct tm_vmem* vmem,
                    unsigned long module);

/**
 * Releases all memory and resources allocated by the transaction. The
 * transaction's data structure will remain initialized and useable after
 * this call returned.
 */
void tm_vmem_tx_release(struct tm_vmem_tx* vmem_tx);

/**
 * Executes a load operation.
 */
int tm_vmem_tx_ld(struct tm_vmem_tx* vmem_tx, uintptr_t addr,
                  void* buf, size_t siz);

/**
 * Executes a store operation.
 */
int tm_vmem_tx_st(struct tm_vmem_tx* vmem_tx, uintptr_t addr,
                  const void* buf, size_t siz);

/**
 * Executes a load-store operation.
 */
int
tm_vmem_tx_ldst(struct tm_vmem_tx* vmem_tx, uintptr_t laddr, uintptr_t saddr,
                size_t siz);

/**
 * Privatizes a region of memory.
 */
int
tm_vmem_tx_privatize(struct tm_vmem_tx* vmem_tx, uintptr_t addr, size_t siz,
                     unsigned long flags);

/**
 * Privatizes a region of memory.
 */
int
tm_vmem_tx_privatize_c(struct tm_vmem_tx* vmem_tx, uintptr_t addr, int c,
                       unsigned long flags);

/**
 * Acquires exclusive access to the frames of this transaction.
 */
int
tm_vmem_tx_lock(struct tm_vmem_tx* vmem_tx, struct picotm_error* error);

/**
 * Releases exclusive access.
 */
int
tm_vmem_tx_unlock(struct tm_vmem_tx* vmem_tx, struct picotm_error* error);

/**
 * Validates consistency of the transactions state.
 */
int
tm_vmem_tx_validate(struct tm_vmem_tx* vmem_tx, bool eotx,
                    struct picotm_error* error);

/**
 * Applies all transaction-local changes to main memory.
 */
int
tm_vmem_tx_apply(struct tm_vmem_tx* vmem_tx, struct picotm_error* error);

/**
 * Reverts all transaction-local changes.
 */
int
tm_vmem_tx_undo(struct tm_vmem_tx* vmem_tx, struct picotm_error* error);

/**
 * Cleans up a transcation's resources.
 */
int
tm_vmem_tx_finish(struct tm_vmem_tx* vmem_tx, struct picotm_error* error);
