/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "ptrdata_tx.h"
#include "picotm/picotm-error.h"
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

static unsigned long long
key_for_ptr(const void* ptr)
{
    return (unsigned long long)ptr;
}

struct value {
    uintptr_t data;
};

static uintptr_t
create_value(unsigned long long key,
             struct picotm_treemap* treemap,
             struct picotm_error* error)
{
    struct value* val = malloc(sizeof(*val));
    if (!val) {
        picotm_error_set_error_code(error, PICOTM_OUT_OF_MEMORY);
        return 0;
    }
    val->data = 0;

    return (uintptr_t)val;
}

static void
destroy_value(uintptr_t value, struct picotm_treemap* treemap)
{
    struct value* val = (struct value*)value;
    free(val);
}

void
ptrdata_tx_init(struct ptrdata_tx* self)
{
    assert(self);

    picotm_treemap_init(&self->map, 8);
}

void
ptrdata_tx_uninit(struct ptrdata_tx* self)
{
    assert(self);

    picotm_treemap_uninit(&self->map, destroy_value);
}

void
ptrdata_tx_set_data(struct ptrdata_tx* self, const void* ptr,
                    const void* data, struct picotm_error* error)
{
    assert(self);

    uintptr_t value = picotm_treemap_find_value(&self->map, key_for_ptr(ptr),
                                                create_value, error);
    if (picotm_error_is_set(error)) {
        return;
    }
    struct value* val = (struct value*)value;

    val->data = (uintptr_t)data;
}

void
ptrdata_tx_clear_data(struct ptrdata_tx* self, const void* ptr,
                      struct picotm_error* error)
{
    assert(self);

    uintptr_t value = picotm_treemap_find_value(&self->map, key_for_ptr(ptr),
                                                NULL, error);
    if (picotm_error_is_set(error)) {
        return;
    } else if (!value) {
        return;
    }
    struct value* val = (struct value*)value;

    val->data = (uintptr_t)0;
}

void*
ptrdata_tx_get_data(struct ptrdata_tx* self, const void* ptr,
                    struct picotm_error* error)
{
    assert(self);

    uintptr_t value = picotm_treemap_find_value(&self->map, key_for_ptr(ptr),
                                                NULL, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    } else if (!value) {
        return NULL;
    }
    struct value* val = (struct value*)value;

    return (void*)val->data;
}
