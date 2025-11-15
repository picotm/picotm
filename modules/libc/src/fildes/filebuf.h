/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2020   Thomas Zimmermann
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

#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-shared-ref-obj.h"
#include "filebuf_id.h"

struct filebuf {

    /* Reference-counting base object. */
    struct picotm_shared_ref16_obj ref_obj;

    /* The file buffer's unique id. */
    struct filebuf_id id;
};

void
filebuf_init(struct filebuf self[static 1],
             struct picotm_error error[static 1]);

void
filebuf_uninit(struct filebuf self[static 1]);

void
filebuf_ref_or_set_up(struct filebuf self[static 1], int fildes,
                      struct picotm_error error[static 1]);

int
filebuf_ref_or_set_up_if_id(struct filebuf self[static 1], int fildes,
                            const struct filebuf_id id[static 1],
                            struct picotm_error error[static 1]);

void
filebuf_ref(struct filebuf self[static 1],
            struct picotm_error error[static 1]);

int
filebuf_ref_if_id(struct filebuf self[static 1],
                  const struct filebuf_id id[static 1]);

void
filebuf_unref(struct filebuf self[static 1]);
