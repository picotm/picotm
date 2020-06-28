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

#include "picotm/picotm-lib-rwlock.h"
#include "picotm/picotm-lib-rwstate.h"
#include "filebuf.h"
#include "rwcountermap.h"
#include "rwlockmap.h"

enum seekbuf_field {
    SEEKBUF_FIELD_FILE_SIZE,
    SEEKBUF_FIELD_STATE,
    NUMBER_OF_SEEKBUF_FIELDS
};

struct seekbuf {
    struct filebuf base;
    struct picotm_rwlock  rwlock[NUMBER_OF_SEEKBUF_FIELDS];
    struct rwlockmap rwlockmap;
};

void
seekbuf_init(struct seekbuf self[static 1],
             struct picotm_error error[static 1]);

void
seekbuf_uninit(struct seekbuf self[static 1]);

void
seekbuf_try_rdlock_field(struct seekbuf self[static 1],
                         enum seekbuf_field field,
                         struct picotm_rwstate rwstate[static 1],
                         struct picotm_error error[static 1]);

void
seekbuf_try_wrlock_field(struct seekbuf self[static 1],
                         enum seekbuf_field field,
                         struct picotm_rwstate rwstate[static 1],
                         struct picotm_error error[static 1]);

void
seekbuf_unlock_field(struct seekbuf self[static 1], enum seekbuf_field field,
                     struct picotm_rwstate rwstate[static 1]);

void
seekbuf_try_rdlock_region(struct seekbuf self[static 1],
                          off_t off, size_t nbyte,
                          struct rwcountermap rwcountermap[static 1],
                          struct picotm_error error[static 1]);

void
seekbuf_try_wrlock_region(struct seekbuf self[static 1],
                          off_t off, size_t nbyte,
                          struct rwcountermap rwcountermap[static 1],
                          struct picotm_error error[static 1]);

void
seekbuf_unlock_region(struct seekbuf self[static 1], off_t off, size_t nbyte,
                      struct rwcountermap rwcountermap[static 1]);
