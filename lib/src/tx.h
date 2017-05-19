/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdbool.h>
#include "log.h"
#include "module.h"
#include "picotm/picotm.h"

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

#define MAX_NMODULES    (256)

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

    unsigned long nmodules; /**< \brief Number allocated modules */
    struct module module[MAX_NMODULES]; /** \brief Registered modules */

    bool is_initialized;
};

int
tx_init(struct tx* self, struct tx_shared* tx_shared);

void
tx_release(struct tx* self);

bool
tx_is_irrevocable(const struct tx* self);

unsigned long
tx_register_module(struct tx* self,
                   void (*lock)(void*, struct picotm_error*),
                   void (*unlock)(void*, struct picotm_error*),
                   bool (*is_valid)(void*, int, struct picotm_error*),
                   void (*apply)(void*, struct picotm_error*),
                   void (*undo)(void*, struct picotm_error*),
                   void (*apply_events)(const struct picotm_event*, size_t,
                                        void*, struct picotm_error*),
                   void (*undo_events)(const struct picotm_event*, size_t,
                                       void*, struct picotm_error*),
                   void (*updatecc)(void*, int, struct picotm_error*),
                   void (*clearcc)(void*, int, struct picotm_error*),
                   void (*finish)(void*, struct picotm_error*),
                   void (*uninit)(void*),
                   void* data,
                   struct picotm_error* error);

void
tx_append_event(struct tx* self, unsigned long module, unsigned long op,
                uintptr_t cookie, struct picotm_error* error);

int
tx_begin(struct tx* self, enum tx_mode mode);

int
tx_commit(struct tx* self, struct picotm_error* error);

int
tx_rollback(struct tx* self, struct picotm_error* error);

bool
tx_is_valid(struct tx* self);
