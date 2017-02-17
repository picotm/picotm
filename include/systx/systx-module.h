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

SYSTX_END_DECLS
