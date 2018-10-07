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

#include "fpu_tx.h"
#include "picotm/picotm-error.h"
#include <errno.h>

/**
 * \todo Access to floating-point environment and status flags is
 *       dependent on compiler support, build flags and program state.
 *       The whole implementation requires a review before the support
 *       in transaction can be stabilized.
 */

/* Gcc does not support the FENV_ACCESS pragma. */
#ifndef __GNUC__
#pragma STDC FENV_ACCESS ON
#endif

enum {
    FPU_TX_FLAG_FENV_SAVED    = 1 << 0,
    FPU_TX_FLAG_FEXCEPT_SAVED = 1 << 1
};

void
fpu_tx_init(struct fpu_tx* self, unsigned long module, struct picotm_error* error)
{
    self->module = module;
    self->flags = 0;

    int res = fegetenv(&self->saved_fenv);
    if (res < 0) {
        picotm_error_set_error_code(error, PICOTM_INVALID_FENV);
        return;
    }

    res = fegetexceptflag(&self->saved_fexcept, FE_ALL_EXCEPT);
    if (res < 0) {
        picotm_error_set_error_code(error, PICOTM_INVALID_FENV);
        return;
    }
}

void
fpu_tx_uninit(struct fpu_tx* self)
{
    return;
}

bool
fpu_tx_fenv_saved(const struct fpu_tx* self)
{
    return !!(self->flags & FPU_TX_FLAG_FENV_SAVED);
}

bool
fpu_tx_fexcept_saved(const struct fpu_tx* self)
{
    return !!(self->flags & FPU_TX_FLAG_FEXCEPT_SAVED);
}

void
fpu_tx_save_fenv(struct fpu_tx* self, struct picotm_error* error)
{
    if (self->flags & FPU_TX_FLAG_FENV_SAVED) {
        return;
    }

    int res = fegetenv(&self->saved_fenv);
    if (res < 0) {
        picotm_error_set_error_code(error, PICOTM_INVALID_FENV);
        return;
    }

    self->flags |= FPU_TX_FLAG_FENV_SAVED;
}

void
fpu_tx_save_fexcept(struct fpu_tx* self, struct picotm_error* error)
{
    if (self->flags & FPU_TX_FLAG_FEXCEPT_SAVED) {
        return;
    }

    int res = fegetexceptflag(&self->saved_fexcept, FE_ALL_EXCEPT);
    if (res < 0) {
        picotm_error_set_error_code(error, PICOTM_INVALID_FENV);
        return;
    }

    self->flags |= FPU_TX_FLAG_FEXCEPT_SAVED;
}

void
fpu_tx_undo(struct fpu_tx* self, struct picotm_error* error)
{
    if (self->flags & FPU_TX_FLAG_FENV_SAVED) {
        int res = fesetenv(&self->saved_fenv);
        if (res < 0) {
            picotm_error_set_error_code(error, PICOTM_INVALID_FENV);
            return;
        }
    }

    if (self->flags & FPU_TX_FLAG_FEXCEPT_SAVED) {
        int res = fesetexceptflag(&self->saved_fexcept, FE_ALL_EXCEPT);
        if (res < 0) {
            picotm_error_set_error_code(error, PICOTM_INVALID_FENV);
            return;
        }
    }
}

void
fpu_tx_finish(struct fpu_tx* self)
{
    self->flags = 0; /* marks errno as 'not saved' */
}
