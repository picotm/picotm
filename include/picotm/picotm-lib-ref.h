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

/**
 * \ingroup group_lib
 * \file
 */

#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include "compiler.h"

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_lib
 * A 16-bit thread-local reference counter.
 */
struct picotm_ref16 {
    uint_fast16_t count;
};

/**
 * \ingroup group_lib
 * \internal
 * Initializes the counter with the given value.
 * \warning This is an internal interface. Don't use it in application code.
 */
static inline void
__picotm_ref16_init(struct picotm_ref16* self, unsigned long count)
{
    self->count = count;
}

/**
 * \ingroup group_lib
 * \internal
 * Increments a reference counter.
 * \warning This is an internal interface. Don't use it in application code.
 */
static inline bool
__picotm_ref16_up(struct picotm_ref16* self)
{
    uint16_t old_ref = self->count++;
    assert(old_ref != (uint16_t)-1);

    return (old_ref == 0);
}

/**
 * \ingroup group_lib
 * \internal
 * Decrements a reference counter.
 * \warning This is an internal interface. Don't use it in application code.
 */
static inline bool
__picotm_ref16_down(struct picotm_ref16* self)
{
    uint16_t old_ref = self->count--;
    assert(old_ref != 0);

    return (old_ref == 1);
}

/**
 * \ingroup group_lib
 * \internal
 * Reads the reference counter's value.
 * \warning This is an internal interface. Don't use it in application code.
 */
static inline uint16_t
__picotm_ref16_count(const struct picotm_ref16* self)
{
    return self->count;
}

/**
 * \ingroup group_lib
 * A 16-bit shared reference counter.
 */
struct picotm_shared_ref16 {
    atomic_uint_fast16_t count;
};

/**
 * \ingroup group_lib
 * Initializes a static reference counter with the given value.
 * \param   count   The initial reference count
 */
#define PICOTM_SHARED_REF16_INITIALIZER(count)  \
{                                               \
    .count = ATOMIC_VAR_INIT(count)             \
}

/**
 * \ingroup group_lib
 * \internal
 * Initializes the counter with the given value.
 * \warning This is an internal interface. Don't use it in application code.
 */
static inline void
__picotm_shared_ref16_init(struct picotm_shared_ref16* self, uint16_t count)
{
    atomic_init(&self->count, count);
}

/**
 * \ingroup group_lib
 * \internal
 * Increments a reference counter.
 * \warning This is an internal interface. Don't use it in application code.
 */
static inline bool
__picotm_shared_ref16_up(struct picotm_shared_ref16* self)
{
    uint16_t old_ref = atomic_fetch_add_explicit(&self->count, 1,
                                                 memory_order_acq_rel);
    assert(old_ref != (uint16_t)-1);

    return (old_ref == 0);
}

/**
 * \ingroup group_lib
 * \internal
 * Decrements a reference counter.
 * \warning This is an internal interface. Don't use it in application code.
 */
static inline bool
__picotm_shared_ref16_down(struct picotm_shared_ref16* self)
{
    uint16_t old_ref = atomic_fetch_sub_explicit(&self->count, 1,
                                                 memory_order_acq_rel);
    assert(old_ref != 0);

    return (old_ref == 1);
}

/**
 * \ingroup group_lib
 * \internal
 * Reads the reference counter's value.
 * \warning This is an internal interface. Don't use it in application code.
 */
static inline uint16_t
__picotm_shared_ref16_count(const struct picotm_shared_ref16* self)
{
#if __clang
    return atomic_load_explicit(&self->count, memory_order_acquire);
#else
    return atomic_load_explicit((atomic_uint_fast16_t*)&self->count,
                                memory_order_acquire);
#endif
}

/**
 * \ingroup group_lib
 * Initializes a statically allocated reference counter with the given value.
 * \param   __count The initial reference count
 */
#define PICOTM_REF_INITIALIZER(__count) \
{                                       \
    .count = (__count)                  \
}

/**
 * \ingroup group_lib
 * Initializes a statically allocated reference counter with the given value.
 * \param   __count The initial reference count
 */
#define PICOTM_SHARED_REF_INITIALIZER(__count)  \
{                                               \
    .count = ATOMIC_VAR_INIT(__count)           \
}

/**
 * \ingroup group_lib
 * Initializes a reference counter with the given value.
 *
 * \param   self    A reference counter
 * \param   count   The initial reference count
 */
#define picotm_ref_init(self, count) _Generic(self,             \
    struct picotm_ref16*:           __picotm_ref16_init,        \
    struct picotm_shared_ref16*:    __picotm_shared_ref16_init) \
    (self, count)

/**
 * \ingroup group_lib
 * Increments a reference counter.
 *
 * \param   self    A reference counter
 * \returns True is this is the first reference, false otherwise.
 */
#define picotm_ref_up(self) _Generic(self,                      \
    struct picotm_ref16*:           __picotm_ref16_up,          \
    struct picotm_shared_ref16*:    __picotm_shared_ref16_up)   \
    (self)

/**
 * \ingroup group_lib
 * Decrements a reference counter.
 *
 * \param   self    A reference counter
 * \returns True is this is the final reference, false otherwise.
 */
#define picotm_ref_down(self) _Generic(self,                    \
    struct picotm_ref16*:           __picotm_ref16_down,        \
    struct picotm_shared_ref16*:    __picotm_shared_ref16_down) \
    (self)

/**
 * \ingroup group_lib
 * Reads a reference counter's value.
 *
 * \param   self     reference counter
 * \returns The current value of the reference counter.
 */
#define picotm_ref_count(self) _Generic(self,                           \
    const struct picotm_ref16*:         __picotm_ref16_count,           \
          struct picotm_ref16*:         __picotm_ref16_count,           \
    const struct picotm_shared_ref16*:  __picotm_shared_ref16_count,    \
          struct picotm_shared_ref16*:  __picotm_shared_ref16_count)    \
    (self)

PICOTM_END_DECLS
