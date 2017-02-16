/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "tanger-stm-internal.h"

enum validation_mode {
    VALIDATE_OP = 0,
    VALIDATE_DOMAIN,
    VALIDATE_FULL
};

int tanger_stm_is_noundo(tanger_stm_tx_t*);

void tanger_stm_set_data(tanger_stm_tx_t*, void*);

void* tanger_stm_get_data(tanger_stm_tx_t*);

void tanger_stm_set_ext_calls(tanger_stm_tx_t*, int (*lock)(void*),
                                                int (*unlock)(void*),
                                                int (*validate)(void*, int),
                                                int (*commit)(void*),
                                                int (*abort)(void*),
                                                int (*finish)(void*));

void tanger_stm_store_mark_written(tanger_stm_tx_t*, void*, size_t);

void tanger_stm_set_optcc(tanger_stm_tx_t*, int);

int tanger_stm_get_optcc(tanger_stm_tx_t*);

void tanger_stm_set_validation_mode(tanger_stm_tx_t*, enum validation_mode);

enum validation_mode tanger_stm_get_validation_mode(tanger_stm_tx_t*);

int tanger_stm_validate(tanger_stm_tx_t*);

void tanger_stm_abort_self(tanger_stm_tx_t*);

int tanger_stm_go_noundo(tanger_stm_tx_t *tx);
