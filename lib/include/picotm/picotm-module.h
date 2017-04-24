/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "compiler.h"

PICOTM_BEGIN_DECLS

struct picotm_tx;

struct event {
    unsigned long  module;
    unsigned short call;
    uintptr_t      cookie;
};

PICOTM_NOTHROW
/**
 * Registers a new module with the transaction management system.
 */
long
picotm_register_module(int (*lock)(void*),
                       int (*unlock)(void*),
                       int (*validate)(void*, int),
                       int (*apply_event)(const struct event*, size_t, void*),
                       int (*undo_event)(const struct event*, size_t, void*),
                       int (*updatecc)(void*, int),
                       int (*clearcc)(void*, int),
                       int (*finish)(void*),
                       int (*uninit)(void*),
                       int (*tpc_request)(void*, int),
                       int (*tpc_success)(void*, int),
                       int (*tpc_failure)(void*, int),
                       int (*tpc_noundo)(void*, int),
                       void *cbdata);

PICOTM_NOTHROW
/**
 * Injects an event into the transaction's event log.
 */
int
picotm_inject_event(unsigned long module, unsigned long op, uintptr_t cookie);

PICOTM_NOTHROW
/**
 * Instructs the transaction management system to resolve a
 * conflict with another transaction.
 */
void
picotm_resolve_conflict(struct picotm_tx* conflicting_tx);

PICOTM_NOTHROW
/**
 * Instructs the transaction management system to recover from an error. The
 * errno code is given as a hint.
 */
void
picotm_recover_from_errno(int errno_hint);

PICOTM_NOTHROW
/**
 * Validates the transaction state.
 */
bool
picotm_is_valid(void);

PICOTM_NOTHROW
/**
 * Makes the current transaction irrevocable.
 */
void
picotm_irrevocable(void);

PICOTM_NOTHROW
/**
 * Returns the transaction irrevocability status.
 * \return True if the transaction is irrevocable, false otherwise.
 */
bool
picotm_is_irrevocable(void);

/* Tables
 */

PICOTM_NOTHROW
/**
 * Allocate or resize table
 */
void*
picotm_tabresize(void* base, size_t nelems, size_t newnelems, size_t siz);

PICOTM_NOTHROW
/**
 * Free table memory
 */
void
picotm_tabfree(void* base);

PICOTM_NOTHROW
/**
 * Walk over table elements
 */
int
picotm_tabwalk_1(void* base, size_t nelems, size_t siz,
                int (*walk)(void*));

PICOTM_NOTHROW
/**
 * Walk over table elements
 */
int
picotm_tabwalk_2(void* base, size_t nelems, size_t siz,
                int (*walk)(void*, void*), void* data);

PICOTM_NOTHROW
/**
 * Walk over table elements
 */
int
picotm_tabwalk_3(void* base, size_t nelems, size_t siz,
                int (*walk)(void*, void*, void*),
                void* data1, void* data2);

PICOTM_NOTHROW
/**
 * Walk over table in reversed order
 */
int
picotm_tabrwalk_1(void* base, size_t nelems, size_t siz,
                 int (*walk)(void*));

PICOTM_NOTHROW
/**
 * Walk over table in reversed order
 */
int
picotm_tabrwalk_2(void* base, size_t nelems, size_t siz,
                 int (*walk)(void*, void*), void* data);

PICOTM_NOTHROW
/**
 * Filters out duplicate elements
 * \return New length
 */
size_t
picotm_tabuniq(void* base, size_t nelems, size_t siz,
              int (*compare)(const void*, const void*));

PICOTM_END_DECLS
