/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "error_tx.h"
#include <errno.h>

enum {
    ERROR_TX_FLAG_ERRNO_SAVED = 1 << 0
};

void
error_tx_init(struct error_tx* self, unsigned long module)
{
    self->module = module;
    self->recovery = PICOTM_LIBC_ERROR_RECOVERY_AUTO;
    self->flags = 0;
    self->saved_errno = 0;
}

void
error_tx_uninit(struct error_tx* self)
{ }

void
error_tx_set_error_recovery(struct error_tx* self,
                            enum picotm_libc_error_recovery recovery)
{
    self->recovery = recovery;
}

enum picotm_libc_error_recovery
error_tx_get_error_recovery(const struct error_tx* self)
{
    return self->recovery;
}

bool
error_tx_errno_saved(const struct error_tx* self)
{
    return !!(self->flags & ERROR_TX_FLAG_ERRNO_SAVED);
}

void
error_tx_save_errno(struct error_tx* self)
{
    if (self->flags & ERROR_TX_FLAG_ERRNO_SAVED) {
        return;
    }

    self->saved_errno = errno;
    self->flags |= ERROR_TX_FLAG_ERRNO_SAVED;
}

void
error_tx_undo(struct error_tx* self, struct picotm_error* error)
{
    if (self->flags & ERROR_TX_FLAG_ERRNO_SAVED) {
        errno = self->saved_errno;
    }
}

void
error_tx_finish(struct error_tx* self, struct picotm_error* error)
{
    self->flags = 0; /* marks errno as 'not saved' */
}
