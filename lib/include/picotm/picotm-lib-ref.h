/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

/**
 * \ingroup group_modules
 * \file
 */

#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include "compiler.h"

PICOTM_BEGIN_DECLS

/**
 * A 16-bit thread-local reference counter.
 */
struct picotm_ref16 {
    uint_fast16_t count;
};

/* Initializes the counter with the given value.
 * \warning This is an internal interface. Don't use it in application code.
 */
static inline void
__picotm_ref16_init(struct picotm_ref16* self, unsigned long count)
{
    self->count = count;
}

/* Increments a reference counter.
 * \warning This is an internal interface. Don't use it in application code.
 */
static inline bool
__picotm_ref16_up(struct picotm_ref16* self)
{
    uint16_t old_ref = self->count++;
    assert(old_ref != (uint16_t)-1);

    return (old_ref == 0);
}

/* Decrements a reference counter.
 * \warning This is an internal interface. Don't use it in application code.
 */
static inline bool
__picotm_ref16_down(struct picotm_ref16* self)
{
    uint16_t old_ref = self->count--;
    assert(old_ref != 0);

    return (old_ref == 1);
}

/* Reads the reference counter's value.
 * \warning This is an internal interface. Don't use it in application code.
 */
static inline uint16_t
__picotm_ref16_count(const struct picotm_ref16* self)
{
    return self->count;
}

/**
 * A 16-bit shared reference counter.
 */
struct picotm_shared_ref16 {
    atomic_uint_fast16_t count;
};

/* Initializes the counter with the given value.
 * \warning This is an internal interface. Don't use it in application code.
 */
static inline void
__picotm_shared_ref16_init(struct picotm_shared_ref16* self, uint16_t count)
{
    atomic_init(&self->count, count);
}

/* Increments a reference counter.
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

/* Decrements a reference counter.
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

/* Reads the reference counter's value.
 * \warning This is an internal interface. Don't use it in application code.
 */
static inline uint16_t
__picotm_shared_ref16_count(const struct picotm_shared_ref16* self)
{
    return atomic_load_explicit(&self->count, memory_order_acquire);
}

/**
 * Initializes a reference counter with the given value.
 *
 * \param   self    A reference counter
 * \param   count   The initial reference count
 */
#define picotm_ref_init(_ref, _count) _Generic(_ref,            \
    struct picotm_ref16*:           __picotm_ref16_init,        \
    struct picotm_shared_ref16*:    __picotm_shared_ref16_init) \
    (_ref, _count)

/**
 * Increments a reference counter.
 *
 * \param   self    A reference counter
 * \returns True is this is the first reference, false otherwise.
 */
#define picotm_ref_up(_ref) _Generic(_ref,                      \
    struct picotm_ref16*:           __picotm_ref16_up,          \
    struct picotm_shared_ref16*:    __picotm_shared_ref16_up)   \
    (_ref)

/**
 * Decrements a reference counter.
 *
 * \param   self    A reference counter
 * \returns True is this is the final reference, false otherwise.
 */
#define picotm_ref_down(_ref) _Generic(_ref,                    \
    struct picotm_ref16*:           __picotm_ref16_down,        \
    struct picotm_shared_ref16*:    __picotm_shared_ref16_down) \
    (_ref)

/**
 * Reads a reference counter's value.
 *
 * \param   self    A reference counter
 * \returns The current value of the reference counter.
 */
#define picotm_ref_count(_ref) _Generic(_ref,                           \
    const struct picotm_ref16*:         __picotm_ref16_count,           \
          struct picotm_ref16*:         __picotm_ref16_count,           \
    const struct picotm_shared_ref16*:  __picotm_shared_ref16_count,    \
          struct picotm_shared_ref16*:  __picotm_shared_ref16_count)    \
    (_ref)

PICOTM_END_DECLS
