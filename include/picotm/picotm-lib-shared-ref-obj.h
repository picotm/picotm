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

#include "compiler.h"
#include "picotm-lib-ref.h"
#include "picotm-lib-spinlock.h"

/**
 * \ingroup group_lib
 * \file
 * \brief Contains `struct picotm_shared_ref16_obj` and helper functions.
 *
 * The data structure `struct picotm_shared_ref16_obj` is the base for
 * objects with shared reference counting. It maintains the counter and
 * guarantees synchronization with the initializer and finalizer code.
 * The counter of `picotm_shared_ref16_obj` is 16-bit wide of type
 * `struct picotm_shared_ref16`. Additional shared-reference-counter types
 * can be added for use cases with different requirements.
 *
 * Instances of `struct picotm_shared_ref16` are typically stored in
 * reference-counted data structures like this.
 *
 * ~~~ c
 *      struct ref_counted {
 *          struct picotm_shared_ref16_obj ref_obj;
 *
 *          // additional data
 *      };
 * ~~~
 *
 * An instance of `picotm_shared_ref16_obj` is initialized with a call
 * to `picotm_shared_ref16_obj_init()` and cleaned up with a call to
 * `picotm_shared_ref16_obj_uninit()`. These functions receive an instance
 * of `struct picotm_shared_ref16_obj` and the init function also receives
 * an instance of `struct picotm_error`, which returns error state. These
 * functions are called in the init and un-init functions of the parent
 * class.
 *
 * ~~~ c
 *      void
 *      ref_counted_init(struct ref_counted* self, struct picotm_error* error)
 *      {
 *          picotm_shared_ref16_obj_init(&self->ref_obj, error);
 *          if (picotm_error_is_set(error)) {
 *              return;
 *          }
 *
 *          // additional initialization
 *      }
 * ~~~
 *
 * At this point, the reference counter is initialized to zero and references
 * to the object can be acquired. Next is the clean-up code.
 *
 * ~~~ c
 *      void
 *      ref_counted_uninit(struct ref_counted* self)
 *      {
 *          // additional clean-up code
 *
 *          picotm_shared_ref16_obj_uninit(&self->ref_obj);
 *          if (picotm_error_is_set(error)) {
 *              return;
 *          }
 *      }
 * ~~~
 *
 * References to an object are acquired with a call to
 * `picotm_shared_ref16_obj_up()`. The parameters of the function consist
 * of and instance of `struct picotm_shared_ref16_obj`, additional user data,
 * an optional conditional function, and optional initializer function and
 * in instance of `struct picotm_error`, which returns an error state to the
 * caller.
 *
 * If supplied, the optional conditional function can perform a test before
 * the reference is acquired. If the test fails with a value of `false`, the
 * reference is not acquired.
 *
 * If supplied, the optional initializer function initializes the object if,
 * and only if, the first reference is acquired.
 *
 * The reference-counted object using `struct picotm_shared_ref16_obj` will
 * typically provide wrapper functions around `picotm_shared_ref16_obj_up()`.
 *
 * ~~~ c
 *      static bool
 *      up_cond(struct picotm_shared_ref16_obj* ref_obj, void* data,
 *              struct picotm_error* error)
 *      {
 *          struct ref_counted* self = picotm_containerof(ref_obj,
 *                                                        struct ref_counted,
 *                                                        ref_obj);
 *
 *          return true; // Perform a test and return success or failure.
 *      }
 *
 *      static void
 *      first_ref(struct picotm_shared_ref16_obj* ref_obj, void* data,
 *                struct picotm_error* error)
 *      {
 *          struct ref_counted* self = picotm_containerof(ref_obj,
 *                                                        struct ref_counted,
 *                                                        ref_obj);
 *
 *          // Perform additional initalization when acquiring the
 *          // first reference.
 *      }
 *
 *      void
 *      ref_counted_up(struct ref_counted* self, struct picotm_error* error)
 *      {
 *          picotm_shared_ref16_obj_up(&self->ref_obj, NULL, up_cond,
 *                                     first_ref, error);
 *      }
 * ~~~
 *
 * With the example code above, users of `struct ref_counted` call
 * `ref_counted_up()` to acquire a reference on an instance of the data
 * structure.
 *
 * The code for releasing a reference is very similar to code for acquiring
 * one. This time the function `picotm_shared_ref16_obj_down()` is used.
 *
 * ~~~ c
 *      static bool
 *      down_cond(struct picotm_shared_ref16_obj* ref_obj, void* data,
 *                struct picotm_error* error)
 *      {
 *          struct ref_counted* self = picotm_containerof(ref_obj,
 *                                                        struct ref_counted,
 *                                                        ref_obj);
 *
 *          return true; // Perform a test and return success or failure.
 *      }
 *
 *      static void
 *      final_ref(struct picotm_shared_ref16_obj* ref_obj, void* data,
 *                struct picotm_error* error)
 *      {
 *          struct ref_counted* self = picotm_containerof(ref_obj,
 *                                                        struct ref_counted,
 *                                                        ref_obj);
 *
 *          // Perform additional cleanup when releasing the
 *          // final reference.
 *      }
 *
 *      void
 *      ref_counted_down(struct ref_counted* self)
 *      {
 *          picotm_shared_ref16_obj_down(&self->ref_obj, NULL, down_cond,
 *                                       final_ref);
 *      }
 * ~~~
 */

PICOTM_BEGIN_DECLS

struct picotm_error;

/**
 * \ingroup group_lib
 * A shared reference-counted object.
 */
struct picotm_shared_ref16_obj {

    /** Internal lock. */
    struct picotm_spinlock lock;

    /** Reference counter */
    struct picotm_shared_ref16 ref;
};

/**
 * \ingroup group_lib
 * Invoked by picotm's shared-ref16 object to test if a reference shall
 * be acquired ore released.
 * \param   ref_obj     The shared-ref16 object.
 * \param   data        User data.
 * \param   error[out]  Returns an error to the caller.
 * \returns True if the performed test succeeds, or false otherwise.
 */
typedef bool (*picotm_shared_ref16_obj_condition_function)(
    struct picotm_shared_ref16_obj* ref_obj,
    void* data,
    struct picotm_error* error);

/**
 * \ingroup group_lib
 * Invoked by picotm's shared-ref16 object to initialize the object after
 * acquiring the first refrence.
 * \param       ref_obj The shared-ref16 object.
 * \param       data    User data.
 * \param[out]  error   Returns an error to the caller.
 */
typedef void (*picotm_shared_ref16_obj_first_ref_function)(
    struct picotm_shared_ref16_obj* ref_obj,
    void* data,
    struct picotm_error* error);

/**
 * \ingroup group_lib
 * Invoked by picotm's shared-ref16 object to finalize an object after
 * releasing the final refrence.
 * \param       ref_obj The shared-ref16 object.
 * \param       data    User data.
 * \param[out]  error   Returns an error to the caller.
 */
typedef void (*picotm_shared_ref16_obj_final_ref_function)(
    struct picotm_shared_ref16_obj* ref_obj,
    void* data,
    struct picotm_error* error);

/**
 * \ingroup group_lib
 * Initializes a statically allocated shared-ref16 object.
 */
#define PICOTM_SHARED_REF16_OBJ_INITIALIZER     \
{                                               \
    .lock = PICOTM_SPINLOCK_INITIALIZER,        \
    .ref = PICOTM_SHARED_REF_INITIALIZER(0)     \
}

/**
 * \ingroup group_lib
 * Initializes a shared-ref16 object.
 * \param       self    The shared-ref16 object.
 * \param[out]  error   Returns an error to the caller.
 */
void
picotm_shared_ref16_obj_init(struct picotm_shared_ref16_obj* self,
                             struct picotm_error* error);

/**
 * \ingroup group_lib
 * Uninitializes a shared-ref16 object.
 * \param   self    The shared-ref16 object.
 */
void
picotm_shared_ref16_obj_uninit(struct picotm_shared_ref16_obj* self);

/**
 * \ingroup group_lib
 * Acquires a reference on the shared-ref16 object.
 * \param       self        The shared-ref16 object.
 * \param       data        User data.
 * \param       cond        An optional condition to test if the reference should
 *                          be acquired.
 * \param       first_ref   An optional function to initialize the object when
 *                          the first reference gets acquired.
 * \param[out]  error       Returns an error to the caller.
 *
 * The conditional and initializer functions are synchronized with the
 * reference counter. The up function internally locks the shared-ref16
 * object while performing these operations.
 */
void
picotm_shared_ref16_obj_up(struct picotm_shared_ref16_obj* self, void* data,
                           picotm_shared_ref16_obj_condition_function cond,
                           picotm_shared_ref16_obj_first_ref_function first_ref,
                           struct picotm_error* error);

/**
 * \ingroup group_lib
 * Releases a reference on the shared-ref16 object.
 * \param   self        The shared-ref16 object.
 * \param   data        User data.
 * \param   cond        An optional condition to test if the reference should
 *                      be released.
 * \param   final_ref   An optional function to finalize the object when
 *                      the final reference gets released.
 *
 * The conditional and finalizer functions are synchronized with the
 * reference counter. The down function internally locks the shared-ref16
 * object while performing these operations.
 */
void
picotm_shared_ref16_obj_down(struct picotm_shared_ref16_obj* self, void* data,
                             picotm_shared_ref16_obj_condition_function cond,
                             picotm_shared_ref16_obj_final_ref_function final_ref);

/**
 * \ingroup group_lib
 * Reads the value of a shared-ref16 object's counter.
 * \param   self    The shared-ref16 object.
 * \returns The current value of the reference counter.
 */
uint16_t
picotm_shared_ref16_obj_count(struct picotm_shared_ref16_obj* self);

PICOTM_END_DECLS
