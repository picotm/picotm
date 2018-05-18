/*
 * MIT License
 * Copyright (c) 2018   Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include <stddef.h>
#include "compiler.h"
#include "picotm-lib-ptr.h"
#include "picotm-lib-shared-ref-obj.h"

/**
 * \ingroup group_lib
 * \file
 * \brief Contains shared-state helper macros.
 *
 * Picotm provides a number of helper macros to automate management of
 * shared state. The state is stored in an additional structure, of which
 * threads can acquire and release references.
 *
 * The macro `PICOTM_SHARED_STATE()` declares shared state. It receives
 * the name and the type of the shared state and expands to a type
 * definition. For this type, the macro `PICOTM_SHARED_STATE_STATIC_IMPL()`
 * generates an implementation. For the first and final reference to the
 * shared state, the generated code invokes an initializer or clean-up
 * function. both are given as arguments to
 * `PICOTM_SHARED_STATE_STATIC_IMPL()`. The example below illustrates this
 * for a state of type `struct shared`.
 *
 * ~~~{.c}
 *  struct shared {
 *      int data1;
 *      int data2;
 *  };
 *
 *  void
 *  init_shared_fields(struct shared* shared, struct picotm_error* error)
 *  {
 *      shared->data1 = 0;
 *      shared->data2 = 0;
 *  }
 *
 *  void
 *  uninit_shared_fields(struct shared* shared)
 *  {
 *      // nothing to do
 *  }
 *
 *  PICOTM_SHARED_STATE(shared, struct shared);
 *  PICOTM_SHARED_STATE_STATIC_IMPL(shared,
 *                                  init_shared_fields,
 *                                  uninit_shared_fields)
 * ~~~
 *
 * A shared-state variable is declared with `PICOTM_SHARED_STATE_TYPE()`. It
 * has to be initalized by assigning `PICOTM_SHARED_STATE_INITIALIZER`.
 *
 * ~~~{.c}
 *  static PICOTM_SHARED_STATE_TYPE(shared) shared_var =
 *      PICOTM_SHARED_STATE_INITIALIZER;
 * ~~~
 *
 * A call to `PICOTM_SHARED_STATE_REF()` acquires a reference, a call to
 * `PICOTM_SHARED_STATE_UNREF()` releases a previously acquired reference.
 * For convenience, `PICOTM_SHARED_STATE_REF()` returns a pointer to the
 * shared state.
 *
 * ~~~{.c}
 *  struct picotm_error error = PICOTM_ERROR_INITIALIZER;
 *
 *  struct shared* shared =
 *      PICOTM_SHARED_STATE_REF(shared, &shared_var, &error);
 *  if (picotm_error_is_set(&error)) {
 *      // perform error handling
 *  }
 *
 *  // ...
 *
 *  // do something with 'shared'
 *
 *  // ...
 *
 *  // In thread-local cleanup code
 *  PICOTM_SHARED_STATE_UNREF(shared, &shared_var);
 * ~~~
 *
 * Acquiring and releasing a reference to a shared state is thread-safe.
 * Both calls are serialized with each other and the provided initializer
 * and clean-up functions. Accessing the shared-state data fields requires
 * additional concurrency control.
 */

PICOTM_BEGIN_DECLS

struct picotm_error;

/**
 * \ingroup group_lib
 * \internal
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_SHARED_STATE_NAME(_name)   \
    __ ## _name ## _shared_state

/**
 * \ingroup group_lib
 * \internal
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_SHARED_STATE_FIRST_REF_CB(_name)   \
    first_ref_ ## _name ## _shared_state_cb

/**
 * \ingroup group_lib
 * \internal
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_SHARED_STATE_FINAL_REF_CB(_name)   \
    final_ref_ ## _name ## _shared_state_cb

/**
 * \ingroup group_lib
 * \internal
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_SHARED_STATE_REF(_name)    \
    _name ## _shared_state_ref

/**
 * \ingroup group_lib
 * \internal
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_SHARED_STATE_UNREF(_name)  \
    _name ## _shared_state_unref

/**
 * \ingroup group_lib
 * Expands to the name of the shared-state structure.
 * \param   _name   The state name.
 */
#define PICOTM_SHARED_STATE_TYPE(_name)         \
    struct __PICOTM_SHARED_STATE_NAME(_name)

/**
 * \ingroup group_lib
 * Defines a shared-state structure.
 * \param   _name   The state name.
 * \param   _type   The state type.
 */
#define PICOTM_SHARED_STATE(_name, _type)       \
    PICOTM_SHARED_STATE_TYPE(_name) {           \
        struct picotm_shared_ref16_obj ref_obj; \
        _type _name;                            \
    };

/**
 * \ingroup group_lib
 * Initializes a shared-state structure.
 */
#define PICOTM_SHARED_STATE_INITIALIZER             \
{                                                   \
    .ref_obj = PICOTM_SHARED_REF16_OBJ_INITIALIZER  \
}

/**
 * \ingroup group_lib
 * \internal
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_SHARED_STATE_IMPL(_static, _name, _type, _init, _uninit)   \
    _static void                                                            \
    __PICOTM_SHARED_STATE_FIRST_REF_CB(_name)(                              \
        struct picotm_shared_ref16_obj* ref_obj, void* data,                \
        struct picotm_error* error)                                         \
    {                                                                       \
        PICOTM_SHARED_STATE_TYPE(_name)* shared =                           \
            picotm_containerof(ref_obj,                                     \
                               PICOTM_SHARED_STATE_TYPE(_name),             \
                               ref_obj);                                    \
        (_init)(&(shared->_name), error);                                   \
    }                                                                       \
    _static _type*                                                          \
    __PICOTM_SHARED_STATE_REF(_name)(                                       \
        PICOTM_SHARED_STATE_TYPE(_name)* self,                              \
        struct picotm_error* error)                                         \
    {                                                                       \
        picotm_shared_ref16_obj_up(                                         \
            &self->ref_obj, NULL, NULL,                                     \
            __PICOTM_SHARED_STATE_FIRST_REF_CB(_name),                      \
            error);                                                         \
        if (picotm_error_is_set(error)) {                                   \
            return NULL;                                                    \
        }                                                                   \
        return &(self->_name);                                              \
    }                                                                       \
    _static void                                                            \
    __PICOTM_SHARED_STATE_FINAL_REF_CB(_name)(                              \
        struct picotm_shared_ref16_obj* ref_obj, void* data,                \
        struct picotm_error* error)                                         \
    {                                                                       \
        PICOTM_SHARED_STATE_TYPE(_name)* shared =                           \
            picotm_containerof(ref_obj,                                     \
                               PICOTM_SHARED_STATE_TYPE(_name),             \
                               ref_obj);                                    \
        (_uninit)(&(shared->_name));                                        \
    }                                                                       \
    _static void                                                            \
    __PICOTM_SHARED_STATE_UNREF(_name)(                                     \
        PICOTM_SHARED_STATE_TYPE(_name)* self)                              \
    {                                                                       \
        picotm_shared_ref16_obj_down(                                       \
            &self->ref_obj, NULL, NULL,                                     \
            __PICOTM_SHARED_STATE_FINAL_REF_CB(_name));                     \
    }

/**
 * \ingroup group_lib
 * Expands to the implementation of a shared state.
 * \param   _name   The state name.
 * \param   _type   The state type.
 * \param   _init   The state's initializer function.
 * \param   _uninit The state's clean-up function.
 */
#define PICOTM_SHARED_STATE_STATIC_IMPL(_name, _type, _init, _uninit)   \
    __PICOTM_SHARED_STATE_IMPL(static, _name, _type, _init, _uninit)

/**
 * \ingroup group_lib
 * Acquires a reference to an instance of a shared state.
 * \param       _name   The state name.
 * \param       _self   The state's instance.
 * \param[out]  _error  Returns an error to the caller.
 * \returns The acquired state on success, or NULL on error.
 */
#define PICOTM_SHARED_STATE_REF(_name, _self, _error)   \
    __PICOTM_SHARED_STATE_REF(_name)(_self, _error)

/**
 * \ingroup group_lib
 * Releases a reference to an instance of a shared state.
 * \param   _name   The state name.
 * \param   _self   The state's instance.
 */
#define PICOTM_SHARED_STATE_UNREF(_name, _self) \
    __PICOTM_SHARED_STATE_UNREF(_name)(_self)

PICOTM_END_DECLS
