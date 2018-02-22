/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "compiler.h"
#include "picotm-lib-ref.h"
#include "picotm-lib-spinlock.h"

/**
 * \ingroup group_modules
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

struct picotm_shared_ref16_obj {

    /** Internal lock. */
    struct picotm_spinlock lock;

    /** Reference counter */
    struct picotm_shared_ref16 ref;
};

/**
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
 * Initializes a statically allocated shared-ref16 object.
 */
#define PICOTM_SHARED_REF16_OBJ_INITIALIZER     \
{                                               \
    .lock = PICOTM_SPINLOCK_INITIALIZER,        \
    .ref = PICOTM_SHARED_REF_INITIALIZER        \
}

/**
 * Initializes a shared-ref16 object.
 * \param       self    The shared-ref16 object.
 * \param[out]  error   Returns an error to the caller.
 */
void
picotm_shared_ref16_obj_init(struct picotm_shared_ref16_obj* self,
                             struct picotm_error* error);

/**
 * Uninitializes a shared-ref16 object.
 * \param   self    The shared-ref16 object.
 */
void
picotm_shared_ref16_obj_uninit(struct picotm_shared_ref16_obj* self);

/**
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

PICOTM_END_DECLS
