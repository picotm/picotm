/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
