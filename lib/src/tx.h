/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdbool.h>
#include <picotm/picotm.h>
#include "component.h"
#include "log.h"

#define MAX_NCOMPONENTS (256)

struct tx_shared;

enum tx_mode {
    TX_MODE_REVOCABLE,
    TX_MODE_IRREVOCABLE
};

struct tx {
    struct __picotm_tx public_state;
    struct log        log;
    struct tx_shared *shared;
    enum tx_mode      mode;

    unsigned long    nmodules; /**< \brief Number allocated modules */
    struct component com[MAX_NCOMPONENTS]; /** \brief Registered components */

    bool              is_initialized;
};

int
tx_init(struct tx* self, struct tx_shared* tx_shared);

void
tx_release(struct tx* self);

bool
tx_is_irrevocable(const struct tx* self);

long
tx_register_module(struct tx* self,
                   int (*lock)(void*),
                   int (*unlock)(void*),
                   int (*validate)(void*, int),
                   int (*apply_event)(const struct event*, size_t, void*),
                   int (*undo_event)(const struct event*, size_t, void*),
                   int (*updatecc)(void*, int),
                   int (*clearcc)(void*, int),
                   int (*finish)(void*),
                   int (*uninit)(void*),
                   void* data);

int
tx_inject_event(struct tx* self, unsigned long module, unsigned long op,
                uintptr_t cookie);

int
tx_begin(struct tx* self, enum tx_mode mode);

int
tx_commit(struct tx* self);

int
tx_rollback(struct tx* self);

bool
tx_is_valid(struct tx* self);
