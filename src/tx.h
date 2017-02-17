/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdbool.h>
#include <systx/systx.h>
#include "log.h"

struct tx {
    struct __systx_tx public_state;
    struct log        log;
    enum __systx_mode mode;
    bool              is_initialized;
};

int
tx_init(struct tx* self);

void
tx_release(struct tx* self);

struct log*
tx_log(struct tx* self);

bool
tx_is_irrevocable(const struct tx* self);

int
tx_begin(struct tx* self, enum __systx_mode mode);

int
tx_commit(struct tx* self);

int
tx_rollback(struct tx* self);
