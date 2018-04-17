/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
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
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "picotm/config/picotm-libc-config.h"
#include "picotm/compiler.h"
#include <signal.h>

PICOTM_BEGIN_DECLS

struct picotm_error;

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Public interfaces of picotm's libc module.
 */

/*
 * Error handling
 */

/**
 * \ingroup group_libc
 * The error-recovery strategy for system calls.
 *
 * Each module detects errors and initiates recovery. Sometimes reported
 * errors are not failures of the component, but expected corner cases.
 * For example 'read()' on non-blocking file descriptors signals `EAGAIN`
 * if there's no data available.
 *
 * The libc modules has to distiguish such cases from actual errors to
 * decide when to initiate recovery. `enum picotm_libc_error_recovery` is
 * a list of possible strategies.
 *
 * It is recommended to use `PICOTM_LIBC_ERROR_RECOVERY_AUTO`.
 */
enum picotm_libc_error_recovery {
    /** Use heuristics to decide which errors to recover from. This is
     * the default. */
    PICOTM_LIBC_ERROR_RECOVERY_AUTO,
    /** Recover from all errors. */
    PICOTM_LIBC_ERROR_RECOVERY_FULL
};

PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Sets the strategy for deciding when to recover from errors.
 * \param   recovery    The error-recovering strategy.
 */
void
picotm_libc_set_error_recovery(enum picotm_libc_error_recovery recovery);

PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Returns the strategy for deciding when to recover from errors.
 * \returns The current error-recovering strategy.
 */
enum picotm_libc_error_recovery
picotm_libc_get_error_recovery(void);

/*
 * File I/O
 */

/**
 * \ingroup group_libc
 * File-type constants.
 */
enum picotm_libc_file_type {
    PICOTM_LIBC_FILE_TYPE_CHRDEV = 0,   /**< \brief Character device */
    PICOTM_LIBC_FILE_TYPE_FIFO,         /**< \brief FIFO */
    PICOTM_LIBC_FILE_TYPE_REGULAR,      /**< \brief Regular file */
    PICOTM_LIBC_FILE_TYPE_DIR,          /**< \brief Directory */
    PICOTM_LIBC_FILE_TYPE_SOCKET        /**< \brief Socket */
};

/**
 * \ingroup group_libc
 * Concurrency-control mode for file-descriptor I/O.
 */
enum picotm_libc_cc_mode {
    /** Set CC mode to irrevocablilty */
    PICOTM_LIBC_CC_MODE_NOUNDO = 0,
    /** Set CC mode to pessimistic two-phase locking */
    PICOTM_LIBC_CC_MODE_2PL
};

/**
 * \ingroup group_libc
 * File-descriptor I/O write mode.
 */
enum picotm_libc_write_mode {
    /** \brief Write-back mode. */
    PICOTM_LIBC_WRITE_BACK,
    /** \brief Write-through mode */
    PICOTM_LIBC_WRITE_THROUGH
};

PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Saves the value of 'errno' during a transaction. Module authors should
 * call this function before invoking a function that might modify errno's
 * value.
 */
void
picotm_libc_save_errno(void);

PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Sets the preferred mode of concurrency control for I/O on a specific
 * file type.
 * \param   file_type   The file type to set the CC mode for.
 * \param   cc_mode     The CC mode.
 */
void
picotm_libc_set_file_type_cc_mode(enum picotm_libc_file_type file_type,
                                  enum picotm_libc_cc_mode cc_mode);

PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Returns the currently preferred mode of concurrency control for I/O on
 * a specific file type.
 * \param   file_type   The file type to get the CC mode from.
 * \returns             The current CC mode for the given file type.
 */
enum picotm_libc_cc_mode
picotm_libc_get_file_type_cc_mode(enum picotm_libc_file_type file_type);

/*
 * Signal handling
 */

PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * \brief Acquires the signal handler from the process.
 * \param       signum          The number of the signal to acquire.
 * \param       nontx_sigaction The signal-handler function for
 *                              non-transactional code.
 * \param[out]  error           Returns an error to the caller.
 *
 * A call to `picotm_libc_acquire_proc_signal()` takes over the
 * handling of a signal from the process. After a call to this function,
 * picotm handles the acquired signal.
 */
void
picotm_libc_acquire_proc_signal(int signum,
                                void (*nontx_sigaction)(int, siginfo_t*,
                                                        void*),
                                struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * \brief Releases a previously acquired signal to the process.
 * \param       signum  The number of the signal to release.
 * \param[out]  error   Returns an error to the caller.
 *
 * A call to `picotm_libc_release_proc_signal()` returns an acquired
 * signal back to the process. It restores the signal's signal-handler
 * settings that have been saved by `picotm_libc_acquire_proc_signal()`.
 */
void
picotm_libc_release_proc_signal(int signum, struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * \brief Enable recovery for a UNIX signal.
 * \param       signum          The signal number.
 * \param       is_recoverable  Send a recoverable error to the transaction.
 * \param[out]  error           Returns an error to the caller.
 *
 * A call to `picotm_libc_add_signal()` enables transaction-safe signal
 * handling for the specified signal *on the current thread.* Enabled
 * signals have to be acquired with a call to
 * `picotm_libc_acquire_proc_signal()` first.
 *
 * \attention The second parameter, `is_recoverable`, marks the
 *            signal-generated error as recoverable. Because signals can
 *            happen at any time, it is generally *unsafe* to treat signal
 *            errors as recoverable. This parameter should be set to
 *            *false.*
 */
void
picotm_libc_add_signal(int signum, _Bool is_recoverable,
                       struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * \brief Disable recovery for a UNIX signal.
 * \param   signum  The signal number.
 */
void
picotm_libc_remove_signal(int signum);

PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * \brief Disable recovery for all UNIX signals.
 */
void
picotm_libc_clear_signals(void);

PICOTM_END_DECLS

/**
 * \defgroup group_libc The C Standard Library Module
 *
 * \brief Covers the C Standard Library module. This module offers many
 *        features of the C Standard Library and POSIX standard, such as
 *        string and memory functions, memory allocation, and file-descriptor
 *        I/O.
 *
 * The C Standard Library Module covers many features of the C Standard
 * Library and the POSIX standard.
 *
 * # Error Handling
 *
 * The C Standard Library module provides transaction-safe errno as errno_tx.
 * With a few exceptions, the transactional functions will handle errors for
 * you. Only rarely you should have a need to examine the errno status. If
 * accessed, the errno value is saved and restored during aborts.
 *
 * # Signal Handling
 *
 * The C Standard library module can handle signals automatically and execute
 * recovery for a signal.
 *
 * # Memory allocation
 *
 * You can allocate memory with malloc_tx(), calloc_tx() or
 * posix_memalign_tx(). The allocation is transaction-safe. On aborts,
 * allocated memory blocks will automatically be released. To explicitly
 * release a block of memory call free_tx(). If the transaction aborts
 * afterwards, the released memory block will be available again. For
 * reallocations call realloc_tx(). This call combines memory allocation
 * and releases in a single function.
 *
 * # String and Memory functions.
 *
 * The C Standard Library module supports a wide range of string and memory
 * helpers, such as strcpy(), strdup(), memmove(), or memcmp().
 *
 * # File-Descriptor I/O
 *
 * File descriptors can be used transactionally. The module protects file
 * descriptors, open file description, and file buffer; if possible. Call
 * open_tx() and close_tx() to open and close files within a transaction.
 *
 * File content is read with read_tx() or pread_tx(), and written with
 * write_tx() or pwrite_tx(). Depending on the operation, the file position
 * is updated.
 *
 * The module also supports FIFO and socket buffers; although some operations
 * require irrevocability.
 *
 * # File-System Support
 *
 * The file system is already a shared resource. Since the module only
 * supports  process-local transactions, at least POSIX-like semantics are
 * available. For example, link_tx() is performed immediately and
 * atomically, or temporary files created with mkstemp_tx() are removed
 * automatically on aborts.
 */
