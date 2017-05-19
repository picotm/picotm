/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "compiler.h"
#include "picotm-error.h"

PICOTM_BEGIN_DECLS

struct picotm_tx;

struct event {
    unsigned long  module;
    unsigned short call;
    uintptr_t      cookie;
};

/**
 * Invoked by picotm to lock a module's resources at the beginning
 * if a commit.
 * \param       data    The pointer to module-specific data.
 * \param[out]  error   Returns an error from the module.
 */
typedef void (*picotm_module_lock_function)(void* data,
                                            struct picotm_error* error);

/**
 * Invoked by picotm to unlock a module's resources. This is the inverse
 * of picotm_module_lock_function.
 * \param       data    The pointer to module-specific data.
 * \param[out]  error   Returns an error from the module.
 */
typedef void (*picotm_module_unlock_function)(void* data,
                                              struct picotm_error* error);

/**
 * Invoked by picotm to validate a module's resources.
 * \param       data            The pointer to module-specific data.
 * \param       is_irrevocable  True if the transaction is irrevocable, false otherwise.
 * \param[out]  error           Returns an error from the module.
 * \returns True if valid, or flase otherwise.
 */
typedef bool (*picotm_module_is_valid_function)(void* data,
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
 * Invoked by picotm during the commit phase to apply a number of
 * consecutive events.
 * \param       events  An array of events.
 * \param       nevents The number of elements in events.
 * \param       data    The pointer to module-specific data.
 * \param[out]  error   Returns an error from the module.
 */
typedef void (*picotm_module_apply_events_function)(const struct event* events,
                                                    size_t nevents,
                                                    void* data,
                                                    struct picotm_error* error);

/**
 * Invoked by picotm during the roll-back phase to revert a number of
 * consecutive events.
 * \param       events  An array of events.
 * \param       nevents The number of elements in events.
 * \param       data    The pointer to module-specific data.
 * \param[out]  error   Returns an error from the module.
 */
typedef void (*picotm_module_undo_events_function)(const struct event* events,
                                                   size_t nevents,
                                                   void* data,
                                                   struct picotm_error* error);

/**
 * Invoked by picotm to update a module's concurrency control during a commit.
 * \param       data            The pointer to module-specific data.
 * \param       is_irrevocable  True if the transaction is irrevocable, false otherwise.
 * \param[out]  error           Returns an error from the module.
 */
typedef void (*picotm_module_update_cc_function)(void* data,
                                                 int is_irrevocable,
                                                 struct picotm_error* error);

/**
 * Invoked by picotm to clear a module's concurrency control during an abort.
 * \param       data            The pointer to module-specific data.
 * \param       is_irrevocable  True if the transaction is irrevocable, false otherwise.
 * \param[out]  error           Returns an error from the module.
 */
typedef void (*picotm_module_clear_cc_function)(void* data,
                                                int is_irrevocable,
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
typedef void (*picotm_module_uninit_function)(void* data);

PICOTM_NOTHROW
/**
 * Registers a new module with the transaction management system.
 * \param       lock            The lock call-back function.
 * \param       unlock          The unlock call-back function.
 * \param       is_valid        The is-valid call-back function.
 * \param       apply           The apply call-back function.
 * \param       undo            The undo call-back function.
 * \param       apply_events    The apply-events call-back function.
 * \param       undo_events     The undo-events call-back function.
 * \param       update_cc       The update-CC call-back function.
 * \param       clear_cc        The clear-CC call-back function.
 * \param       finish          The finish call-back function.
 * \param       uninit          The uninit call-back function.
 * \param       cbdata          A pointer to module-specific data.
 * \param[out]  error           Returns an error.
 * \returns A module number on success.
 */
unsigned long
picotm_register_module(picotm_module_lock_function lock,
                       picotm_module_unlock_function unlock,
                       picotm_module_is_valid_function is_valid,
                       picotm_module_apply_function apply,
                       picotm_module_undo_function undo,
                       picotm_module_apply_events_function apply_events,
                       picotm_module_undo_events_function undo_events,
                       picotm_module_update_cc_function update_cc,
                       picotm_module_clear_cc_function clear_cc,
                       picotm_module_finish_function finish,
                       picotm_module_uninit_function uninit,
                       void* cbdata,
                       struct picotm_error* error);

PICOTM_NOTHROW
/**
 * Appends an event to the transaction's event log.
 * \param       module  The module number
 * \param       op      A module-specific operation.
 * \param       cookie  A module-specific cookie.
 * \param[out]  error   Returns an error.
 */
void
picotm_append_event(unsigned long module, unsigned long op, uintptr_t cookie,
                    struct picotm_error* error);

PICOTM_NOTHROW
/**
 * Instructs the transaction management system to resolve a
 * conflict with another transaction.
 * \param   conflicting_tx  The conflicting transaction, or NULL if unknown.
 *
 * \attention   This function will not return if picotm aborts the
 *              transaction. If the function returns, the caller shall restart
 *              the operation that triggered the conflict.
 */
void
picotm_resolve_conflict(struct picotm_tx* conflicting_tx);

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
 * Validates the transaction state.
 * \returns True if the transaction state is valid, false otherwise.
 */
bool
picotm_is_valid(void);

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

/*
 * Tables
 */

/**
 * Invoked by table functions when walking over the elements of a table.
 * \param   data0   The table element.
 * \returns The number of element to advance on success, or a negative value
 *          otherwise.
 */
typedef int (*picotm_tabwalk_1_function)(void* data0);

/**
 * Invoked by table functions when walking over the elements of a table.
 * \param   data0   The table element.
 * \param   data1   An additional argument.
 * \returns The number of element to advance on success, or a negative value
 *          otherwise.
 */
typedef int (*picotm_tabwalk_2_function)(void* data0, void* data1);

/**
 * Invoked by table functions when walking over the elements of a table.
 * \param   data0   The table element.
 * \param   data1   An additional argument.
 * \param   data2   An additional argument.
 * \returns The number of element to advance on success, or a negative value
 *          otherwise.
 */
typedef int (*picotm_tabwalk_3_function)(void* data0, void* data1,
                                         void* data2);

/**
 * Invoked by table functions for comparing two elements.
 * \param   data0   A table element.
 * \param   data1   A table element.
 * \returns -1, 0, or 1 if data0 is less, equal, or greatet than data1.
 */
typedef int (*picotm_tab_compare_function)(const void* data0,
                                           const void* data1);

PICOTM_NOTHROW
/**
 * Allocate or resize table.
 * \param   base        A pointer to the table's first element.
 * \param   nelems      The number of elements in the table.
 * \param   newnelems   The new number of elements in the table.
 * \param   siz         The number of bytes per element.
 * \returns A pointer to the resized table on success, or NULL otherwise.
 */
void*
picotm_tabresize(void* base, size_t nelems, size_t newnelems, size_t siz);

PICOTM_NOTHROW
/**
 * Free table memory.
 * \param   base    A pointer to the table's first element.
 */
void
picotm_tabfree(void* base);

PICOTM_NOTHROW
/**
 * Walk over table elements.
 * \param   base    A pointer to the table's first element.
 * \param   nelems  The number of elements in the table.
 * \param   siz     The number of bytes per element.
 * \param   walk    The call-back function.
 * \returns 0 on success, or a negative value otherwise.
 */
int
picotm_tabwalk_1(void* base, size_t nelems, size_t siz,
                 picotm_tabwalk_1_function walk);

PICOTM_NOTHROW
/**
 * Walk over table elements.
 * \param   base    A pointer to the table's first element.
 * \param   nelems  The number of elements in the table.
 * \param   siz     The number of bytes per element.
 * \param   walk    The call-back function.
 * \param   data    An additional second argument to the call-back function.
 * \returns 0 on success, or a negative value otherwise.
 */
int
picotm_tabwalk_2(void* base, size_t nelems, size_t siz,
                 picotm_tabwalk_2_function walk, void* data);

PICOTM_NOTHROW
/**
 * Walk over table elements.
 * \param   base    A pointer to the table's first element.
 * \param   nelems  The number of elements in the table.
 * \param   siz     The number of bytes per element.
 * \param   walk    The call-back function.
 * \param   data1   An additional second argument to the call-back function.
 * \param   data2   An additional third argument to the call-back function.
 * \returns 0 on success, or a negative value otherwise.
 */
int
picotm_tabwalk_3(void* base, size_t nelems, size_t siz,
                 picotm_tabwalk_3_function walk, void* data1, void* data2);

PICOTM_NOTHROW
/**
 * Walk over table in reversed order.
 * \param   base    A pointer to the table's first element.
 * \param   nelems  The number of elements in the table.
 * \param   siz     The number of bytes per element.
 * \param   walk    The call-back function.
 * \returns 0 on success, or a negative value otherwise.
 */
int
picotm_tabrwalk_1(void* base, size_t nelems, size_t siz,
                  picotm_tabwalk_1_function walk);

PICOTM_NOTHROW
/**
 * Walk over table in reversed order.
 * \param   base    A pointer to the table's first element.
 * \param   nelems  The number of elements in the table.
 * \param   siz     The number of bytes per element.
 * \param   walk    The call-back function.
 * \param   data    An additional second argument to the call-back function.
 * \returns 0 on success, or a negative value otherwise.
 */
int
picotm_tabrwalk_2(void* base, size_t nelems, size_t siz,
                  picotm_tabwalk_2_function walk, void* data);

PICOTM_NOTHROW
/**
 * Filters out duplicate elements
 * \param   base    A pointer to the table's first element.
 * \param   nelems  The number of elements in the table.
 * \param   siz     The number of bytes per element.
 * \param   compare The call-back function for comparing elements.
 * \returns The new number of elements in the table.
 */
size_t
picotm_tabuniq(void* base, size_t nelems, size_t siz,
               picotm_tab_compare_function compare);

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
 * To register a module, call picotm_register_module(). The call receives a
 * number of module-specific call-back functions and module data, and returns
 * a unique module number. The call-back functions are invoked by picotm to
 * instruct the module during different phases of a transaction. The returned
 * module number is later required when inserting events into the transaction
 * log.
 *
 * For an example, let's look at a transactional allocator. Registering the
 * module might look like this.
 *
 * ~~~{.c}
 *  long res = picotm_register_module(NULL,
 *                                    NULL,
 *                                    NULL,
 *                                    NULL,
 *                                    NULL,
 *                                    apply, // See below.
 *                                    undo,  // See below.
 *                                    NULL,
 *                                    NULL,
 *                                    NULL,
 *                                    NULL);
 *  if (res < 0) {
 *      // abort with an error
 *  }
 *  unsigned int module_number = res;
 * ~~~
 *
 * Modules are registered per-thread. You have to register your module on
 * each thread where it is used.
 *
 * # Injecting Events into the Transaction Log.
 *
 * When the user invokes an operation of your module, the module might want
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
 *  apply(const struct event* event, size_t nevents, void* data, picotm_error* error)
 *  {
 *      while (nevents) {
 *          if (event->call == CMD_MALLOC) {
 *              // Nothing to do.
 *          }
 *          if (event->call == CMD_FREE) {
 *              // Apply free().
 *              free((void*)event->cookie);
 *          }
 *          ++event;
 *          --nevents;
 *      }
 *      return 0;
 *  }
 *
 *  int
 *  undo(const struct event* event, size_t nevents, void* data, picotm_error* error)
 *  {
 *      events += nevents;
 *      while (nevents) {
 *          --event;
 *          if (event->call == CMD_MALLOC) {
 *              // Revert malloc().
 *              free((void*)event->cookie);
 *          }
 *          if (event->call == CMD_FREE) {
 *              // Nothing to do.
 *          }
 *          --nevents;
 *      }
 *      return 0;
 *  }
 * ~~~
 *
 * In our example we have registered a module for a transactional allocator.
 * The malloc operation allocates memory, but reverts this allocation during
 * an undo. The free operation doesn't actually free the memory immediately,
 * be creates an event to free the memory during a commit.
 */
