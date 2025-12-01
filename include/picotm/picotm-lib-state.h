/*
 * picotm - A system-level transaction manager
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

#include <stddef.h>
#include "compiler.h"

/**
 * \ingroup group_lib
 * \file
 * \brief Contains state helper macros.
 *
 * Picotm provides helpers for managing state. State can be acquired and
 * released as needed. When acquiring state, an initializer function sets
 * up the state fields; when releasing the state, a clean-up function
 * release the state fields. The example below declares state of type
 * `struct state`
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
 * The macro `PICOTM_STATE()` defines state. Callers pass the state's
 * name and data structure. The macro `PICOTM_STATE_STATIC_IMPL()` defines
 * the corresponding implementation with the initializer and clean-up
 * functions supplied as arguments.
 *
 * At this point, `PICOTM_STATE_TYPE()` declares a variable of the state's
 * type. State variables must be initialized with `PICOTM_STATE_INITIALIZER`.
 *
 * ~~~{.c}
 *  PICOTM_STATE_TYPE(state) state_var = PICOTM_STATE_INITIALIZER;
 * ~~~
 *
 * The macro `PICOTM_STATE_ACQUIRE()` returns the state of a state variable.
 * On it's first invocation, it runs the provided initializer function. Later
 * invocations return the already initialized state; until the state is
 * released with `PICOTM_STATE_RELEASE()`. This macro runs the provided
 * clean-up function and releases the state.
 *
 * ~~~{.c}
 *  struct picotm_error error = PICOTM_ERROR_INITIALIZER;
 *
 *  struct state* state = PICOTM_STATE_ACQUIRE(state, state_var, true, &error);
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
 *  PICOTM_STATE_RELEASE(state, state_var);
 * ~~~
 *
 * State handling is not thread safe. For a thread-safe implemenation,
 * `PICOTM_SHARED_STATE()` and its helpers are available.
 */

PICOTM_BEGIN_DECLS

struct picotm_error;

/**
 * \ingroup group_lib
 * \internal
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_STATE_NAME(_name)      \
    __ ## _name ## _state

/**
 * \ingroup group_lib
 * Expands to the name of the state structure.
 * \param   _name   The state name.
 */
#define PICOTM_STATE_TYPE(_name)        \
    struct __PICOTM_STATE_NAME(_name)

/**
 * \ingroup group_lib
 * Defines a state structure.
 * \param   _name   The state name.
 * \param   _type   The state type.
 */
#define PICOTM_STATE(_name, _type)      \
    PICOTM_STATE_TYPE(_name) {          \
        _type   _name;                  \
        _Bool   is_initialized;         \
    }

/**
 * \ingroup group_lib
 * Initializes a state structure.
 */
#define PICOTM_STATE_INITIALIZER        \
{                                       \
    .is_initialized = 0                 \
}

/**
 * \ingroup group_lib
 * \internal
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_STATE_ACQUIRE(_name)   \
    __ ## _name ## _state_acquire

/**
 * \ingroup group_lib
 * \internal
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_STATE_RELEASE(_name)   \
    __ ## _name ## _state_release

/**
 * \ingroup group_lib
 * \internal
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_STATE_DECL(_static, _name, _type)                      \
    _static _type*                                                      \
    __PICOTM_STATE_ACQUIRE(_name)(PICOTM_STATE_TYPE(_name)* self,       \
                                  _Bool initialize,                     \
                                  struct picotm_error* error);          \
    _static void                                                        \
    __PICOTM_STATE_RELEASE(_name)(PICOTM_STATE_TYPE(_name)* self);

/**
 * \ingroup group_lib
 * \internal
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_STATE_IMPL(_static, _name, _type, _init, _uninit)      \
    _static _type*                                                      \
    __PICOTM_STATE_ACQUIRE(_name)(PICOTM_STATE_TYPE(_name)* self,       \
                                  _Bool initialize,                     \
                                  struct picotm_error* error)           \
    {                                                                   \
        if (self->is_initialized) {                                     \
            goto out;                                                   \
        } else if (!initialize) {                                       \
            return NULL;                                                \
        } else {                                                        \
            (_init)(&(self->_name), error);                             \
            if (picotm_error_is_set(error)) {                           \
                return NULL;                                            \
            }                                                           \
        }                                                               \
        self->is_initialized = 1;                                       \
    out:                                                                \
        return &(self->_name);                                          \
    }                                                                   \
    _static void                                                        \
    __PICOTM_STATE_RELEASE(_name)(PICOTM_STATE_TYPE(_name)* self)       \
    {                                                                   \
        if (!self->is_initialized) {                                    \
            return;                                                     \
        } else {                                                        \
            (_uninit)(&(self->_name));                                  \
        }                                                               \
        self->is_initialized = 0;                                       \
    }

/**
 * \ingroup group_lib
 * Expands to the forward declaration of a state.
 * \param   _name   The state name.
 * \param   _type   The state type.
 */
#define PICOTM_STATE_STATIC_DECL(_name, _type)  \
    __PICOTM_STATE_DECL(static, _name, _type)

/**
 * \ingroup group_lib
 * Expands to the implementation of a state.
 * \param   _name   The state name.
 * \param   _type   The state type.
 * \param   _init   The state's initializer function.
 * \param   _uninit The state's clean-up function.
 */
#define PICOTM_STATE_STATIC_IMPL(_name, _type, _init, _uninit)  \
    __PICOTM_STATE_IMPL(static, _name, _type, _init, _uninit)

/**
 * \ingroup group_lib
 * Acquires a state.
 * \param       _name       The state name.
 * \param       _initialize True to initialize if necessary.
 * \param       _self       The state's instance.
 * \param[out]  _error      Returns an error to the caller.
 * \returns The acquired state on success, or NULL if not
 *          initialized or on error.
 */
#define PICOTM_STATE_ACQUIRE(_name, _self, _initialize, _error) \
    __PICOTM_STATE_ACQUIRE(_name)(_self, _initialize, _error)

/**
 * \ingroup group_lib
 * Releases a previously acquired state.
 * \param   _name   The state name.
 * \param   _self   The state's instance.
 */
#define PICOTM_STATE_RELEASE(_name, _self)  \
    __PICOTM_STATE_RELEASE(_name)(_self)

PICOTM_END_DECLS
