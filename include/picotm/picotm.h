/* Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <setjmp.h>
#include "compiler.h"
#include "picotm-error.h"

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_lib
 * \file
 *
 * \brief Main header file for picotm.
 *
 * This is the main header file for picotm. It contains the entry points
 * for starting, committing and ending a transaction; for restarting, and
 * for error handling.
 */

/**
 * Marks a non-transactional variable in a function.
 *
 * When restarting a transaction, picotm uses non-local goto, based on
 * the `sigjmp()` and `longjmp()` functions provided by the C Standard
 * Library. These functions save and restore the thread's instruction
 * and stack pointer, but don't save any variables. This can result in
 * program errors if a variable is held in a register that changes its
 * value between invocations of `sigjmp()` and `longjmp()`. The call
 * to `longjmp()` will not restore the variable's original value.
 *
 * To avoid this problem, mark local, non-transactional variables with
 * ::picotm_safe as shown below.
 *
 * ~~~{.c}
 *  picotm_safe int var = 0;
 *
 *  picotm_begin
 *
 *      [...]
 *
 *  picotm_commit
 *
 *  picotm_end
 * ~~~
 *
 * Even with ::picotm_safe, you still have to privatize the variable when
 * using it within the transaction.
 *
 * With gcc, the command-line option '-Wclobbered' enables a warning about
 * this problem.
 */
#define picotm_safe volatile

/* The transaction start mode.
 * \warning This is an internal interface. Don't use it in application code.
 */
enum __picotm_mode {
    PICOTM_MODE_START = 0,
    PICOTM_MODE_RETRY,
    PICOTM_MODE_IRREVOCABLE,
    PICOTM_MODE_RECOVERY
};

PICOTM_NOTHROW
/* Begins or restarts a transaction, or handles an error.
 * \warning This is an internal interface. Don't use it in application code.
 */
_Bool
__picotm_begin(enum __picotm_mode mode, jmp_buf* env);

/**
 * Starts a new transaction.
 *
 * Invoking ::picotm_begin starts a new transaction. Any code between
 * this macro and ::picotm_commit is considered part of the transaction's
 * execution phase. If the transaction aborts, it will restart from where
 * the ::picotm_begin had been invoked.
 */
#define picotm_begin                                                    \
    {                                                                   \
        jmp_buf __env;                                                  \
        if (__picotm_begin((enum __picotm_mode)setjmp(__env), &__env))  \
        {

PICOTM_NOTHROW
/*
 * Commits the current transaction.
 * \warning This is an internal interface. Don't use it in application code.
 */
void
__picotm_commit(void);

/**
 * Commits the current transaction.
 *
 * This macro commits the currently running transaction. Picotm will validate
 * the consistency of all of the transaction's resources and, if successful,
 * apply all outstanding operations. If validation fails, the transaction will
 * be restarted from ::picotm_begin.
 *
 * \attention Calling this macro is only valid *after* ::picotm_begin and
 *            *before* ::picotm_end.
 */
#define picotm_commit           \
            __picotm_commit();  \
        } else {

/**
 * Ends the current transaction.
 */
#define picotm_end  \
        }           \
    }

PICOTM_NOTHROW
/**
 * Restarts the current transaction.
 */
void
picotm_restart(void);

PICOTM_NOTHROW
/**
 * Returns the number of restarts of the thread's most recent transaction.
 */
unsigned long
picotm_number_of_restarts(void);

PICOTM_NOTHROW
void
/**
 * Releases all resources of picotm on the current thread.
 */
picotm_release(void);

PICOTM_NOTHROW
/**
 * Returns the current error status.
 *
 * \attention This function is only valid during the transaction's recovery
 *            phase.
 *
 * \returns The current error status.
 */
enum picotm_error_status
picotm_error_status();

PICOTM_NOTHROW
/**
 * Returns the current error's recoverable status.
 *
 * \attention This function is only valid during the transaction's recovery
 *            phase.
 *
 * \returns The current error status.
 */
bool
picotm_error_is_non_recoverable(void);

PICOTM_NOTHROW
/**
 * Returns the current picotm error code.
 *
 * \attention This function is only valid during the transaction's recovery
 *            phase, and if ::picotm_error_status() is ::PICOTM_ERROR_CODE.
 *
 * \returns The current picotm error code.
 */
enum picotm_error_code
picotm_error_as_error_code(void);

PICOTM_NOTHROW
/**
 * Returns the current picotm errno code.
 *
 * \attention This function is only valid during the transaction's recovery
 *            phase, and if ::picotm_error_status() is ::PICOTM_ERRNO.
 *
 * \returns The current picotm errno code.
 */
int
picotm_error_as_errno(void);

PICOTM_END_DECLS

/**
 * \defgroup group_lib The picotm Programming Interface
 *
 * \brief This section provides information for users of picotm. It explains
 *        the concept of transactions and how to run a transaction with
 *        picotm.
 *
 * # Properties of a Transaction {#lib_properties_of_a_transaction}
 *
 * The transaction concept is a powerful metaphor for implementing concurrent
 * and fault-tolerant software. A transaction system provides two services
 * to it's users. These are
 *
 *  -   concurrency control (i.e., thread-safe access to shared resources), and
 *  -   error handling.
 *
 * # Writing Transactions       {#lib_writing_transactions}
 *
 * Each transactions contains at least the statements
 *
 *  1.  ::picotm_begin,
 *  2.  ::picotm_commit, and
 *  3.  ::picotm_end
 *
 * in this exact order. In the source code, this looks like this
 *
 * ~~~{.c}
 * void
 * func()
 * {
 *      // non-transactional code
 *
 *      int x = 0;
 *
 *      picotm_begin
 *          // transactional code
 *      picotm_commit
 *      picotm_end
 *
 *      // more non-transactional code
 * }
 *
 * ~~~
 *
 * Transactional operations are invoked between ::picotm_begin and
 * ::picotm_commit. This is called the transaction's *execution phase.*
 * Operations listed in the execution phase are subject to speculative
 * execution. This means that an operation might be executed, but it's
 * effects are not be permanent until the transaction commits.
 *
 * The transaction performs a commit when the user invokes ::picotm_commit.
 * This is called the *commit phase.* It is completely implemented by
 * picotm. No intervention by the user is required.
 *
 * Upon entering the commit phase, picotm validates the consistency of
 * all resources that the transaction uses. If validation succeeds, the
 * transaction's effects are applied to become permanent. The program then
 * continuous with the next operation after ::picotm_end.
 * If validation fails, the transaction's effects are reverted and the
 * transaction restarts from ::picotm_begin. The roll-back is transparent
 * to the program.
 *
 * If an operation fails, picotm provides error handling for it's operations.
 * Error handling consists of
 *
 *  -   error detection, and
 *  -   error recovery.
 *
 * Error detection is completely implemented by picotm an it's module. When
 * picotm invokes an operation it tests for reported errors. No intervention
 * by the user is required.
 *
 * Error recovery is partially provided by picotm. For picotm, it is possible
 * to recover from some errors. For example, if a write operation to a file
 * temporarily fails, picotm can retry.
 *
 * Some errors require special program logic to recover. For example, if the
 * program runs out of memory, it might free up memory by invoking a garbage
 * collector. This cannot generally be implemented by picotm, as it depends
 * on program-specific constraints. This is called the *recovery phase.*
 *
 * Error-recovery code is located between ::picotm_commit and ::picotm_end.
 * If picotm detects an error that is cannot recover from, it rolls back the
 * transaction's effects and jumps to the first instruction *after*
 * ::picotm_commit. The program now has the chance of performing error
 * recovery and, if successful, restart the transaction by calling
 * picotm_restart().
 */

/**
 * \mainpage The picotm System-Level Transaction Manager
 *
 * Picotm is a system-level transaction manager. It provides transactional
 * semantics for low-level and operating-system functionality. It’s flexible
 * and extensible to cover exactly your requirements. **Error handling** and
 * **thread isolation** are provided by picotm, all you have to implement is
 * the application logic.
 *
 * Picotm is implemented in plain C and is well-suited for implementing
 * applications and firmware that is secure, reliable and thread-safe; yet
 * easy to develop. This makes picotm well-suited for **multi-threaded**
 * and **fault-tolerant** software.
 *
 * Picotm is Open Source under the terms of the [MIT License][license:mit];
 * viable for use in free and proprietary software.
 *
 * ## Table of Contents
 *
 *  -# \ref group_lib \n
 *      \copybrief group_lib
 *  -# \ref group_modules \n
 *      \copybrief group_modules
 *  -# \ref group_tm \n
 *      \copybrief group_tm
 *  -# \ref group_txlib \n
 *      \copybrief group_txlib
 *  -# \ref group_libc \n
 *      \copybrief group_libc
 *  -# \ref group_libm \n
 *      \copybrief group_libm
 *  -# \ref group_libpthread \n
 *      \copybrief group_libpthread
 *
 * [license:mit]:   http://opensource.org/licenses/MIT
 */
