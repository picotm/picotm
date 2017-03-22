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

SYSTX_NOTHROW
/**
 * Returns the internal transaction structure.
 * \warning This is an internal interface. Don't use it in application code.
 */
struct __systx_tx*
__systx_get_tx(void);

SYSTX_NOTHROW
/**
 * Begins or restarts a transaction, or handles an error.
 * \warning This is an internal interface. Don't use it in application code.
 */
void
__systx_begin(enum __systx_mode mode);

/**
 * Starts a new transaction.
 */
#define systx_begin() \
    __systx_begin((enum __systx_mode)setjmp(__systx_get_tx()->env));

SYSTX_NOTHROW
/**
 * Commits the current transaction.
 */
void
systx_commit(void);

SYSTX_NOTHROW
/**
 * Aborts the current transaction.
 */
void
systx_abort(void);

SYSTX_NOTHROW
void
/**
 * Releases all resources of systx on the current thread.
 */
systx_release(void);

SYSTX_END_DECLS
