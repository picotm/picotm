/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include "compiler.h"

SYSTX_BEGIN_DECLS

struct systx_tx;

struct event {
    unsigned long  module;
    unsigned short call;
    uintptr_t      cookie;
};

SYSTX_NOTHROW
/**
 * Registers a new module with the transaction management system.
 */
long
systx_register_module(int (*lock)(void*),
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

SYSTX_NOTHROW
/**
 * Injects an event into the transaction's event log.
 */
int
systx_inject_event(unsigned long module, unsigned long op, uintptr_t cookie);

SYSTX_NOTHROW
/**
 * Instructs the transaction management system to resolve a
 * conflict with another transaction.
 */
void
systx_resolve_conflict(struct systx_tx* conflicting_tx);

SYSTX_NOTHROW
/**
 * Instructs the transaction management system to resolve an error.
 */
void
systx_resolve_error(int errno_hint);

SYSTX_NOTHROW
/**
 * Makes the current transaction irrevocable.
 */
void
systx_irrevocable(void);

/* Tables
 */

SYSTX_NOTHROW
/**
 * Allocate or resize table
 */
void*
systx_tabresize(void* base, size_t nelems, size_t newnelems, size_t siz);

SYSTX_NOTHROW
/**
 * Free table memory
 */
void
systx_tabfree(void* base);

SYSTX_NOTHROW
/**
 * Walk over table elements
 */
int
systx_tabwalk_1(void* base, size_t nelems, size_t siz,
                int (*walk)(void*));

SYSTX_NOTHROW
/**
 * Walk over table elements
 */
int
systx_tabwalk_2(void* base, size_t nelems, size_t siz,
                int (*walk)(void*, void*), void* data);

SYSTX_NOTHROW
/**
 * Walk over table elements
 */
int
systx_tabwalk_3(void* base, size_t nelems, size_t siz,
                int (*walk)(void*, void*, void*),
                void* data1, void* data2);

SYSTX_NOTHROW
/**
 * Walk over table in reversed order
 */
int
systx_tabrwalk_1(void* base, size_t nelems, size_t siz,
                 int (*walk)(void*));

SYSTX_NOTHROW
/**
 * Walk over table in reversed order
 */
int
systx_tabrwalk_2(void* base, size_t nelems, size_t siz,
                 int (*walk)(void*, void*), void* data);

SYSTX_NOTHROW
/**
 * Filters out duplicate elements
 * \return New length
 */
size_t
systx_tabuniq(void* base, size_t nelems, size_t siz,
              int (*compare)(const void*, const void*));

SYSTX_END_DECLS
