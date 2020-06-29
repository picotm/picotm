/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2019-2020  Thomas Zimmermann <contact@tzimmermann.org>
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

#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-rwlock.h"
#include "picotm/picotm-lib-rwstate.h"
#include "filebuf.h"

struct picotm_error;
struct picotm_rwstate;

enum pipebuf_field {
    PIPEBUF_FIELD_READ_END,
    PIPEBUF_FIELD_STATE,
    PIPEBUF_FIELD_WRITE_END,
    NUMBER_OF_PIPEBUF_FIELDS
};

struct pipebuf {
    struct filebuf base;
    struct picotm_rwlock rwlock[NUMBER_OF_PIPEBUF_FIELDS];
};

static inline struct pipebuf*
pipebuf_of_base(struct filebuf base[static 1])
{
    return picotm_containerof(base, struct pipebuf, base);
}

void
pipebuf_init(struct pipebuf self[static 1],
             struct picotm_error error[static 1]);

void
pipebuf_uninit(struct pipebuf self[static 1]);

void
pipebuf_try_rdlock_field(struct pipebuf self[static 1],
                         enum pipebuf_field field,
                         struct picotm_rwstate rwstate[static 1],
                         struct picotm_error error[static 1]);

void
pipebuf_try_wrlock_field(struct pipebuf self[static 1],
                         enum pipebuf_field field,
                         struct picotm_rwstate rwstate[static 1],
                         struct picotm_error error[static 1]);

void
pipebuf_unlock_field(struct pipebuf self[static 1], enum pipebuf_field field,
                     struct picotm_rwstate rwstate[static 1]);
