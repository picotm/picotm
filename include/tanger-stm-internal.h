/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdint.h>

struct tanger_stm_tx {
    int fill;
};

typedef struct tanger_stm_tx tanger_stm_tx_t;

tanger_stm_tx_t* tanger_stm_get_tx(void);

void* tanger_stm_loadregionpre(tanger_stm_tx_t*, uint8_t*, uintptr_t);

void tanger_stm_loadregionpost(tanger_stm_tx_t*, uint8_t*, uintptr_t);
