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
#include "picotm-lib-state.h"

/**
 * \ingroup group_lib
 * \file
 * \brief Contains thread-state helper macros.
 *
 * On top of the set of state helper macros, picotm provides thread-state
 * helper macros. These macros provide a single per-thread instance of a
 * state variable.
 *
 * Thread-local state requires the definition of a regular state variable.
 *
 * ~~~{.c}
 *  struct state {
 *      int data1;
 *      int data2;
 *  };
 *
 *  void
 *  init_state_fields(struct state* state, struct picotm_error* error)
 *  {
 *      state->data1 = 0;
 *      state->data2 = 0;
 *  }
 *
 *  void
 *  uninit_state_fields(struct state* state)
 *  {
 *      // nothing to do
 *  }
 *
 *  PICOTM_STATE(state, struct state);
 *  PICOTM_STATE_STATIC_IMPL(state, struct state,
 *                           init_state_fields,
 *                           uninit_state_fields)
 * ~~~
 *
 * `PICOTM_THREAD_STATE_STATIC_IMPL()` defines the thread-local state for the
 * state variable.
 *
 * ~~~{.c}
 *  PICOTM_THREAD_STATE_STATIC_IMPL(state)
 * ~~~
 *
 * The macros `PICOTM_THREAD_STATE_ACQUIRE()` and
 * `PICOTM_THREAD_STATE_RELEASE()` acquire and release the state variable
 * in the same way as their state counterparts. The difference is that
 * there's only one instance of the state variable for each thread.
 *
 * ~~~{.c}
 *  struct picotm_error error = PICOTM_ERROR_INITIALIZER;
 *
 *  struct state* state = PICOTM_THREAD_STATE_ACQUIRE(state, true, &error);
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
 *  PICOTM_THREAD_STATE_RELEASE(state, state_var);
 * ~~~
 */

PICOTM_BEGIN_DECLS

struct picotm_error;

#define __PICOTM_THREAD_STATE_GET(_name)    \
    __ ## _name ## _thread_state_get

#define __PICOTM_THREAD_STATE_DECL(_static, _name)              \
    _static PICOTM_STATE_TYPE(_name)*                           \
    __PICOTM_THREAD_STATE_GET(_name)(void);                     \

#define __PICOTM_THREAD_STATE_IMPL(_static, _name)              \
    _static PICOTM_STATE_TYPE(_name)*                           \
    __PICOTM_THREAD_STATE_GET(_name)(void)                      \
    {                                                           \
        static __thread PICOTM_STATE_TYPE(_name) s_thread =     \
            PICOTM_STATE_INITIALIZER;                           \
        return &s_thread;                                       \
    }                                                           \

#define PICOTM_THREAD_STATE_STATIC_DECL(_name)  \
    __PICOTM_THREAD_STATE_DECL(static, _name)

#define PICOTM_THREAD_STATE_STATIC_IMPL(_name)  \
    __PICOTM_THREAD_STATE_IMPL(static, _name)

#define PICOTM_THREAD_STATE_ACQUIRE(_name, _initialize, _error)     \
    PICOTM_STATE_ACQUIRE(_name, __PICOTM_THREAD_STATE_GET(_name)(), \
                         _initialize, _error)

#define PICOTM_THREAD_STATE_RELEASE(_name)                          \
    PICOTM_STATE_RELEASE(_name, __PICOTM_THREAD_STATE_GET(_name)())

PICOTM_END_DECLS
