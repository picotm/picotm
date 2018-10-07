/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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

#include "picotm/picotm-lib-rwstate.h"
#include <stdbool.h>

struct fildes;
struct picotm_error;

struct fdtab_tx {
    struct picotm_rwstate rwstate;

    struct fildes* fildes;
};

void
fdtab_tx_init(struct fdtab_tx* self, struct fildes* fildes);

void
fdtab_tx_uninit(struct fdtab_tx* self);

void
fdtab_tx_try_rdlock(struct fdtab_tx* self, struct picotm_error* error);

void
fdtab_tx_try_wrlock(struct fdtab_tx* self, struct picotm_error* error);

struct fd*
fdtab_tx_ref_fildes(struct fdtab_tx* self, int fildes,
                    struct picotm_error* error);

/*
 * Module Interface
 */

void
fdtab_tx_finish(struct fdtab_tx* self);
