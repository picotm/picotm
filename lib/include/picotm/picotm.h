/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <setjmp.h>
#include "compiler.h"

PICOTM_BEGIN_DECLS

/**
 * Marks non-transactional variables in a function.
 *
 * When restarting a transaction, picotm uses non-local goto, based on
 * the `sigjmp()` and `longjmp()` functions provided by the C Standard
 * Library. These functions save and restore the thread's instruction
 * and stack pointer, but don't save any variables. This can lead to
 * program errors, if a variable is held in a register that changes its
 * value between invocations of `sigjmp()` and `longjmp()`. The call
 * to `longjmp()` will not restore the variable's original value.
 *
 * To avoid this problem, mark local, non-transactional variables with
 * `picotm_safe` as shown below.
 * ```
 *      picotm_safe int var = 0;
 *
 *      picotm_begin
 *          ...
 *      picotm_commit
 *      picotm_end
 * ```
 *
 * Even with `picotm_save`, you still have to privatize the variable when
 * using it within the transaction.
 *
 * With gcc, the command-line option '-Wclobbered' will enable a warning
 * about this problem.
 */
#define picotm_safe volatile

/**
 * The transaction start mode.
 * \warning This is an internal interface. Don't use it in application code.
 */
enum __picotm_mode {
    PICOTM_MODE_START = 0,
    PICOTM_MODE_RETRY,
    PICOTM_MODE_IRREVOCABLE,
    PICOTM_MODE_RECOVERY
};

/**
 * Internal transaction structure; touch this to mess up badly.
 * \warning This is an internal interface. Don't use it in application code.
 */
struct __picotm_tx {
    jmp_buf env;
};

PICOTM_NOTHROW
/**
 * Returns the internal transaction structure.
 * \warning This is an internal interface. Don't use it in application code.
 */
struct __picotm_tx*
__picotm_get_tx(void);

PICOTM_NOTHROW
/**
 * Begins or restarts a transaction, or handles an error.
 * \warning This is an internal interface. Don't use it in application code.
 */
_Bool
__picotm_begin(enum __picotm_mode mode);

/**
 * Starts a new transaction.
 */
#define picotm_begin                                                        \
    if (__picotm_begin((enum __picotm_mode)setjmp(__picotm_get_tx()->env))) \
    {

PICOTM_NOTHROW
/**
 * Commits the current transaction.
 * \warning This is an internal interface. Don't use it in application code.
 */
void
__picotm_commit(void);

/**
 * Commits the current transaction.
 */
#define picotm_commit       \
        __picotm_commit();  \
    } else {

/**
 * Ends the current transaction.
 */
#define picotm_end  \
    }

PICOTM_NOTHROW
/**
 * Aborts the current transaction.
 */
void
picotm_abort(void);

PICOTM_NOTHROW
void
/**
 * Releases all resources of picotm on the current thread.
 */
picotm_release(void);

PICOTM_END_DECLS
