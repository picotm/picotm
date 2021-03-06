/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

/**
 * \ingroup group_modules
 * \file
 *
 * \brief Picotm's module interface.
 *
 * Picotm is extensible. The header file `picotm-module.h` contains the
 * declaration of picotm's module interface. The interface allows for
 * implementing support for additional features, libraries and functionality.
 * All existing modules are written on top of this interface as well.
 */

#include "picotm/config/picotm-config.h"
#if defined(__MACH__)
#include <mach/mach.h>
#endif
#if defined(PICOTM_HAVE_SIGNAL_H) && PICOTM_HAVE_SIGNAL_H
#include <signal.h>
#endif
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "compiler.h"
#include "picotm-error.h"

PICOTM_BEGIN_DECLS

struct picotm_tx;

/**
 * Invoked by picotm at the beginning of a transaction.
 * \param       data    The pointer to module-specific data.
 * \param[out]  error   Returns an error from the module.
 */
typedef void (*picotm_module_begin_function)(void* data,
                                             struct picotm_error* error);

/**
 * Invoked by picotm to prepare a module's resources for commit.
 * \param       data            The pointer to module-specific data.
 * \param       is_irrevocable  True if the transaction is irrevocable, false otherwise.
 * \param[out]  error           Returns an error from the module.
 */
typedef void (*picotm_module_prepare_commit_function)(
    void* data,
    int is_irrevocable,
    struct picotm_error* error);

/**
 * Invoked by picotm during the commit phase to apply changes of a
 * module.
 * \param       data    The pointer to module-specific data.
 * \param[out]  error   Returns an error from the module.
 */
typedef void (*picotm_module_apply_function)(void* data,
                                             struct picotm_error* error);

/**
 * Invoked by picotm during the roll-back phase to revert changes of a
 * module.
 * \param       data    The pointer to module-specific data.
 * \param[out]  error   Returns an error from the module.
 */
typedef void (*picotm_module_undo_function)(void* data,
                                            struct picotm_error* error);

/**
 * Invoked by picotm during the commit phase to apply an event.
 * \param       head    The event's head data.
 * \param       tail    The event's tail data.
 * \param       data    The pointer to module-specific data.
 * \param[out]  error   Returns an error from the module.
 */
typedef void (*picotm_module_apply_event_function)(
    uint16_t head,
    uintptr_t tail,
    void* data,
    struct picotm_error* error);

/**
 * Invoked by picotm during the roll-back phase to revert an event.
 * \param       head    The event's head data.
 * \param       tail    The event's tail data.
 * \param       data    The pointer to module-specific data.
 * \param[out]  error   Returns an error from the module.
 */
typedef void (*picotm_module_undo_event_function)(
    uint16_t head,
    uintptr_t tail,
    void* data,
    struct picotm_error* error);

/**
 * Invoked by picotm to clean up a module's resources at the end of a
 * transaction.
 * \param       data    The pointer to module-specific data.
 * \param[out]  error   Returns an error from the module.
 */
typedef void (*picotm_module_finish_function)(void* data,
                                              struct picotm_error* error);

/**
 * Invoked by picotm to clean up a module's resources when the thread exists.
 * \param   data    The pointer to module-specific data.
 */
typedef void (*picotm_module_release_function)(void* data);

/**
 * The module structure contains call-back functions for each
 * module.
 */
struct picotm_module_ops {
    picotm_module_begin_function begin;
    picotm_module_prepare_commit_function prepare_commit;
    picotm_module_apply_function apply;
    picotm_module_undo_function undo;
    picotm_module_apply_event_function apply_event;
    picotm_module_undo_event_function undo_event;
    picotm_module_finish_function finish;
    picotm_module_release_function release;
};

PICOTM_NOTHROW
/**
 * Registers a new module with the transaction management system.
 * \param       ops     The module-operations call-back structure.
 * \param       data    A pointer to module-specific data.
 * \param[out]  error   Returns an error.
 * \returns A module number on success.
 */
unsigned long
picotm_register_module(const struct picotm_module_ops* ops, void* data,
                       struct picotm_error* error);

PICOTM_NOTHROW
/**
 * Appends an event to the transaction's event log.
 * \param       module  The module number
 * \param       head    Module-specific head data.
 * \param       tail    Module-specific tail data, or a pointer to tail data.
 * \param[out]  error   Returns an error.
 */
void
picotm_append_event(unsigned long module, uint16_t head, uintptr_t tail,
                    struct picotm_error* error);

PICOTM_NOTHROW
/**
 * Instructs the transaction management system to resolve a conflict between
 * transactions.
 *
 * \param   conflicting_lock  The conflicting lock, or NULL if unknown.
 *
 * \attention   This function will not return if picotm aborts the
 *              transaction. If the function returns, the caller shall restart
 *              the operation that triggered the conflict.
 */
void
picotm_resolve_conflict(struct picotm_rwlock* conflicting_lock);

PICOTM_NOTHROW
/**
 * Instructs the transaction management system to recover from an error. The
 * error code is given as a hint.
 * \param   error_hint  The error code of the detected error, or
 *                      PICOTM_GENERAL_ERROR if unknown.
 *
 * \attention   This function will not return if picotm aborts the
 *              transaction. If the function returns, the caller shall restart
 *              the operation that triggered the conflict.
 */
void
picotm_recover_from_error_code(enum picotm_error_code error_hint);

PICOTM_NOTHROW
/**
 * Instructs the transaction management system to recover from an error. The
 * errno code is given as a hint.
 * \param   errno_hint  The errno code of the detected error, or 0 if unknown.
 *
 * \attention   This function will not return if picotm aborts the
 *              transaction. If the function returns, the caller shall restart
 *              the operation that triggered the conflict.
 */
void
picotm_recover_from_errno(int errno_hint);

#if defined(PICOTM_HAVE_TYPE_KERN_RETURN_T) && \
        PICOTM_HAVE_TYPE_KERN_RETURN_T || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Instructs the transaction management system to recover from an error
 * given as value of type 'kern_return_t'.
 * \param   value The kern_return_t value of the detected error.
 *
 * \attention   This function will not return if picotm aborts the
 *              transaction. If the function returns, the caller shall restart
 *              the operation that triggered the conflict.
 */
void
picotm_recover_from_kern_return_t(kern_return_t value);
#endif

#if defined(PICOTM_HAVE_TYPE_SIGINFO_T) && \
            PICOTM_HAVE_TYPE_SIGINFO_T || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Instructs the transaction management system to recover from an signal.
 * Informaton about the signal is provided as value of type 'siginfo_t'.
 * \param[in]   info    The siginfo_t value of the received signal.
 *
 * \attention   This function does not return.
 */
void
picotm_recover_from_siginfo_t(const siginfo_t* info);
#endif

PICOTM_NOTHROW
/**
 * Instructs the transaction management system to recover from an error. The
 * error is supplied as an argument.
 * \param   error   The detected error.
 *
 * \attention   This function will not return if picotm aborts the
 *              transaction. If the function returns, the caller shall restart
 *              the operation that triggered the conflict.
 */
void
picotm_recover_from_error(const struct picotm_error* error);

PICOTM_NOTHROW
/**
 * Makes the current transaction irrevocable.
 */
void
picotm_irrevocable(void);

PICOTM_NOTHROW
/**
 * Returns the transaction irrevocability status.
 * \return True if the transaction is irrevocable, false otherwise.
 */
bool
picotm_is_irrevocable(void);

PICOTM_END_DECLS

/**
 * \defgroup group_modules The Module Interface
 *
 * \brief Picotm is extensible. This section explains the module
 *        interface and how to implement your own module on top.
 *
 * Picotm is only a transaction manager. It maintains transactions but
 * doesn't provide their functionality. All actual transactional operations
 * are provided by additional modules. Picotm can be extended with arbitrary
 * functionality. The interface is available to external libraries, so no
 * modification to picotm is required.
 *
 * # Registering Your Module
 *
 * To register a module, call `picotm_register_module()`. The call receives
 * an instance of `struct picotm_module_ops`, which stores a number of
 * module-specific call-back functions, and variable module data. It returns
 * a unique module number. The call-back functions are invoked by picotm to
 * instruct the module during different phases of a transaction. The returned
 * module number is later required when inserting events into the transaction
 * log.
 *
 * For an example, let's look at a transactional allocator. Registering the
 * module might look like this.
 *
 * ~~~{.c}
 *  static const struct picotm_module_ops ops = {
 *      .apply_event = apply, // See below.
 *      .undo_event = undo  // See below.
 *  };
 *
 *  struct picotm_error error = PICOTM_ERROR_INITIALIZER;
 *
 *  unsigned long module_number = picotm_register_module(&ops, NULL, &error);
 *  if (picotm_error_is_set(&error)) {
 *      // abort with an error
 *  }
 * ~~~
 *
 * Modules are registered per-thread. You have to register your module on
 * each thread where it is used.
 *
 * # Injecting Events into the Transaction Log.
 *
 * When the user invokes an operation of your module, the module might wants
 * to store an event in the transaction log. This is done by invoking
 * picotm_inject_event(). During the transaction's commit, events in the
 * transaction log are applied in the order they appear in the log. During
 * a roll-back, events are reverted in opposite order.
 *
 * In the case of our transactional allocator, we have two functions,
 * `malloc()` and `free()`.
 *
 * ~~~{.c}
 *  enum {
 *      CMD_MALLOC,
 *      CMD_FREE
 *  };
 *
 *  // Public interface called from with transaction.
 *  void*
 *  malloc_tx(size_t siz)
 *  {
 *      void* ptr = malloc(siz);
 *      if (!ptr) {
 *          // In real-world code, do error handling here!
 *          return NULL;
 *      }
 *      int res = picotm_inject_event(module_number, CMD_MALLOC, ptr);
 *      if (res < 0) {
 *          // In real-world code, do error handling here!
 *          return NULL;
 *      }
 *      return ptr;
 *  }
 *
 *  // Public interface called from with transaction.
 *  void
 *  free_tx(void* ptr)
 *  {
 *      int res = picotm_inject_event(module_number, CMD_FREE, ptr);
 *      if (res < 0) {
 *          // In real-world code, do error handling here!
 *          return NULL;
 *      }
 *      return ptr;
 *  }
 * ~~~
 *
 * # Applying and Reverting Events
 *
 * For applying or reverting events, your module must provide the respective
 * callbacks to picotm_register_module(). These are invoked by picotm during
 * the life time of a transaction.
 *
 * For our example of the transactional allocator, these call-back functions
 * will look like this.
 *
 * ~~~{.c}
 *  int
 *  apply(uint16_t head, uintptr_t tail, void* data, struct picotm_error* error)
 *  {
 *      if (head == CMD_MALLOC) {
 *          // Nothing to do.
 *      }
 *      if (head == CMD_FREE) {
 *          // Apply free().
 *          free((void*)tail);
 *      }
 *      return 0;
 *  }
 *
 *  int
 *  undo(uint16_t head, uintptr_t tail, void* data, struct picotm_error* error)
 *  {
 *      if (head == CMD_MALLOC) {
 *          // Revert malloc().
 *          free((void*)tail);
 *      }
 *      if (head == CMD_FREE) {
 *          // Nothing to do.
 *      }
 *      return 0;
 *  }
 * ~~~
 *
 * In our example we have registered a module for a transactional allocator.
 * The malloc operation allocates memory, but reverts this allocation during
 * an undo. The free operation doesn't actually free the memory immediately,
 * but creates an event to free the memory during a commit.
 */

/**
 * \defgroup group_lib The Module Helper Library
 *
 * \brief Different modules often share a number of fundamental requirements
 *        and concepts. Picotm comes with a large number of data structures
 *        and functions for modules to re-use. This includes locks, tree
 *        maps, arrays, reference-counting and more.
 */
