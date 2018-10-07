/*
 * MIT License
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
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
#include "picotm-lib-shared-state.h"

/**
 * \ingroup group_lib
 * \file
 * \brief Contains global-state helper macros.
 *
 * Picotm provides helper macros for maintaining shared state. Oftentimes
 * a single shared state is used throughout a module. For this case, picotm
 * provides additional helper macros that maintain such global state. The
 * example below declares shared state of type `struct shared`.
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
 * A single global state variable for an existing shared state is declared
 * with `PICOTM_GLOBAL_STATE_STATIC_IMPL()`. This macro receives the name
 * of the shared state and expands to an implementation.
 *
 * ~~~{.c}
 *  PICOTM_GLOBAL_STATE_STATIC_IMPL(shared)
 * ~~~
 *
 * Users acquire a reference to the global state with
 * `PICOTM_GLOBAL_STATE_REF()` and release a previously acquired reference
 * with a call to `PICOTM_GLOBAL_STATE_UNREF()`.
 *
 * ~~~{.c}
 *  struct picotm_error error = PICOTM_ERROR_INITIALIZER;
 *
 *  struct shared* state = PICOTM_GLOBAL_STATE_REF(shared, &error);
 *  if (picotm_error_is_set(&error)) {
 *      // perform error handling
 *  }
 *
 *  // ...
 *
 *  // do something with 'state'
 *
 *  // ...
 *
 *  // In thread-local cleanup code
 *  PICOTM_GLOBAL_STATE_UNREF(shared);
 * ~~~
 *
 * Global state is implemented on top of shared state, so the same rules
 * for thread-safety apply. All references and releases are serialized with
 * each other and the initializer and clean-up functions. Concurrent access
 * to shared data files requires additional concurrency control.
 */

PICOTM_BEGIN_DECLS

struct picotm_error;

/**
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_GLOBAL_STATE_GET(_name)    \
    _name ## _global_state_get

/**
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_GLOBAL_STATE_REF(_name)    \
    _name ## _global_state_ref

/**
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_GLOBAL_STATE_UNREF(_name)  \
    _name ## _global_state_unref

/**
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_GLOBAL_STATE_IMPL(_static, _name)          \
    _static PICOTM_SHARED_STATE_TYPE(_name)*                \
    __PICOTM_GLOBAL_STATE_GET(_name)(void)                  \
    {                                                       \
        static PICOTM_SHARED_STATE_TYPE(_name) s_global =   \
            PICOTM_SHARED_STATE_INITIALIZER;                \
        return &s_global;                                   \
    }

/**
 * \ingroup group_lib
 * Expands to the implementation of a global state.
 * \param   _name   The state name.
 */
#define PICOTM_GLOBAL_STATE_STATIC_IMPL(_name)  \
    __PICOTM_GLOBAL_STATE_IMPL(static, _name)

/**
 * \ingroup group_lib
 * Returns the statically allocated global state. Callers *must* already
 * hold a reference.
 * \param   _name   The state name.
 * \returns The state.
 */
#define PICOTM_GLOBAL_STATE_GET(_name)      \
    (&(__PICOTM_GLOBAL_STATE_GET(_name)()->_name))

/**
 * \ingroup group_lib
 * Acquires a reference to an instance of a global state.
 * \param       _name   The state name.
 * \param[out]  _error  Returns an error to the caller.
 * \returns The acquired state on success, or NULL on error.
 */
#define PICOTM_GLOBAL_STATE_REF(_name, _error)                          \
    PICOTM_SHARED_STATE_REF(_name, __PICOTM_GLOBAL_STATE_GET(_name)(),  \
                            _error)

/**
 * \ingroup group_lib
 * Releases a reference to an instance of a global state.
 * \param   _name   The state name.
 */
#define PICOTM_GLOBAL_STATE_UNREF(_name)                                    \
    PICOTM_SHARED_STATE_UNREF(_name, __PICOTM_GLOBAL_STATE_GET(_name)())

PICOTM_END_DECLS
