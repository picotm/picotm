/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018-2019  Thomas Zimmermann <contact@tzimmermann.org>
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

#include "ptrdata.h"
#include "picotm/picotm-error.h"
#include <assert.h>
#include <limits.h>
#include <stdatomic.h>
#include <stdlib.h>

static unsigned long long
key_for_ptr(const void* ptr)
{
    return (unsigned long long)ptr;
}

struct value {
    atomic_uintptr_t data;
};

static uintptr_t
create_value(unsigned long long key,
             struct picotm_shared_treemap* treemap,
             struct picotm_error* error)
{
    struct value* val = malloc(sizeof(*val));
    if (!val) {
        picotm_error_set_error_code(error, PICOTM_OUT_OF_MEMORY);
        return 0;
    }
    atomic_init(&val->data, 0);

    return (uintptr_t)val;
}

static void
destroy_value(uintptr_t value, struct picotm_shared_treemap* treemap)
{
    struct value* val = (struct value*)value;
    free(val);
}

void
ptrdata_init(struct ptrdata* self)
{
    assert(self);

    picotm_shared_treemap_init(&self->map, sizeof(void*) * CHAR_BIT, 8);
}

void
ptrdata_uninit(struct ptrdata* self)
{
    assert(self);

    picotm_shared_treemap_uninit(&self->map, destroy_value);
}

void
ptrdata_set_shared_data(struct ptrdata* self, const void* ptr,
                        const void* data, struct picotm_error* error)
{
    assert(self);

    uintptr_t value = picotm_shared_treemap_find_value(&self->map,
                                                       key_for_ptr(ptr),
                                                       create_value,
                                                       destroy_value, error);
    if (picotm_error_is_set(error)) {
        return;
    }
    struct value* val = (struct value*)value;

    atomic_store_explicit(&val->data, (uintptr_t)data, memory_order_release);
}

bool
ptrdata_test_and_set_shared_data(struct ptrdata* self, const void* ptr,
                                 const void* current, const void* data,
                                 struct picotm_error* error)
{
    assert(self);

    uintptr_t value = picotm_shared_treemap_find_value(&self->map,
                                                       key_for_ptr(ptr),
                                                       create_value,
                                                       destroy_value, error);
    if (picotm_error_is_set(error)) {
        return false;
    }
    struct value* val = (struct value*)value;

    uintptr_t expected = (uintptr_t)current;

    return atomic_compare_exchange_strong_explicit(&val->data, &expected,
                                                   (uintptr_t)data,
                                                   memory_order_acq_rel,
                                                   memory_order_relaxed);
}

void
ptrdata_clear_shared_data(struct ptrdata* self, const void* ptr,
                          struct picotm_error* error)
{
    assert(self);

    uintptr_t value = picotm_shared_treemap_find_value(&self->map,
                                                       key_for_ptr(ptr),
                                                       NULL, NULL, error);
    if (picotm_error_is_set(error)) {
        return;
    } else if (!value) {
        return;
    }
    struct value* val = (struct value*)value;

    atomic_store_explicit(&val->data, 0, memory_order_release);
}

void*
ptrdata_get_shared_data(struct ptrdata* self, const void* ptr,
                        struct picotm_error* error)
{
    assert(self);

    uintptr_t value = picotm_shared_treemap_find_value(&self->map,
                                                       key_for_ptr(ptr),
                                                       NULL, NULL, error);
    if (!value) {
        return NULL;
    }
    struct value* val = (struct value*)value;

    return (void*)atomic_load_explicit(&val->data, memory_order_acquire);
}
