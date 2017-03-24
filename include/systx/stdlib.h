/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdlib.h>
#include "compiler.h"

/*
 * Programm termination
 */

SYSTX_NOTHROW SYSTX_NORETURN
void abort_tx(void);

SYSTX_NOTHROW SYSTX_NORETURN
void exit_tx(int status);

SYSTX_NOTHROW SYSTX_NORETURN
void _Exit_tx(int status);
