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

enum sockbuf_field {
    SOCKBUF_FIELD_RECV_END,
    SOCKBUF_FIELD_SEND_END,
    SOCKBUF_FIELD_STATE,
    NUMBER_OF_SOCKBUF_FIELDS
};

struct sockbuf {
    struct filebuf base;
    struct picotm_rwlock rwlock[NUMBER_OF_SOCKBUF_FIELDS];
};

static inline struct sockbuf*
sockbuf_of_base(struct filebuf base[static 1])
{
    return picotm_containerof(base, struct sockbuf, base);
}

void
sockbuf_init(struct sockbuf self[static 1],
             struct picotm_error error[static 1]);

void
sockbuf_uninit(struct sockbuf self[static 1]);

void
sockbuf_try_rdlock_field(struct sockbuf self[static 1],
                         enum sockbuf_field field,
                         struct picotm_rwstate rwstate[static 1],
                         struct picotm_error error[static 1]);

void
sockbuf_try_wrlock_field(struct sockbuf self[static 1],
                         enum sockbuf_field field,
                         struct picotm_rwstate rwstate[static 1],
                         struct picotm_error error[static 1]);

void
sockbuf_unlock_field(struct sockbuf self[static 1], enum sockbuf_field field,
                     struct picotm_rwstate rwstate[static 1]);
