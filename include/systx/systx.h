/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <setjmp.h>
#include "compiler.h"

SYSTX_BEGIN_DECLS

/**
 * The transaction start mode.
 * \warning This is an internal interface. Don't use it in application code.
 */
enum __systx_mode {
    SYSTX_MODE_START = 0,
    SYSTX_MODE_RETRY,
    SYSTX_MODE_IRREVOCABLE
};

/**
 * Internal transaction structure; touch this to mess up badly.
 * \warning This is an internal interface. Don't use it in application code.
 */
struct __systx_tx {
    jmp_buf env;
};

SYSTX_END_DECLS
