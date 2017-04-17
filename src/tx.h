/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdbool.h>
#include <systx/systx.h>
#include "log.h"

struct tx_shared;

enum tx_mode {
    TX_MODE_REVOCABLE,
    TX_MODE_IRREVOCABLE
};

struct tx {
    struct __systx_tx public_state;
    struct log        log;
    struct tx_shared *shared;
    enum tx_mode      mode;
    bool              is_initialized;
};

int
tx_init(struct tx* self, struct tx_shared* tx_shared);

void
tx_release(struct tx* self);

struct log*
tx_log(struct tx* self);

bool
tx_is_irrevocable(const struct tx* self);

int
tx_begin(struct tx* self, enum tx_mode mode);

int
tx_commit(struct tx* self);

int
tx_rollback(struct tx* self);

bool
tx_is_valid(struct tx* self);
