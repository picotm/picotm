/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "fpu_tx.h"
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

int
fpu_tx_init(struct fpu_tx* self, unsigned long module)
{
    self->module = module;
    self->flags = 0;

    int res = fegetenv(&self->saved_fenv);
    if (res < 0) {
        return -1;
    }

    res = fegetexceptflag(&self->saved_fexcept, FE_ALL_EXCEPT);
    if (res < 0) {
        return -1;
    }

    return 0;
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

int
fpu_tx_save_fenv(struct fpu_tx* self)
{
    if (self->flags & FPU_TX_FLAG_FENV_SAVED) {
        return 0;
    }

    int res = fegetenv(&self->saved_fenv);
    if (res < 0) {
        return -1;
    }

    self->flags |= FPU_TX_FLAG_FENV_SAVED;

    return 0;
}

int
fpu_tx_save_fexcept(struct fpu_tx* self)
{
    if (self->flags & FPU_TX_FLAG_FEXCEPT_SAVED) {
        return 0;
    }

    int res = fegetexceptflag(&self->saved_fexcept, FE_ALL_EXCEPT);
    if (res < 0) {
        return -1;
    }

    self->flags |= FPU_TX_FLAG_FEXCEPT_SAVED;

    return 0;
}

int
fpu_tx_undo(struct fpu_tx* self)
{
    if (self->flags & FPU_TX_FLAG_FENV_SAVED) {
        int res = fesetenv(&self->saved_fenv);
        if (res < 0) {
            return -1;
        }
    }

    if (self->flags & FPU_TX_FLAG_FEXCEPT_SAVED) {
        int res = fesetexceptflag(&self->saved_fexcept, FE_ALL_EXCEPT);
        if (res < 0) {
            return -1;
        }
    }

    return 0;
}

int
fpu_tx_finish(struct fpu_tx* self)
{
    self->flags = 0; /* marks errno as 'not saved' */

    return 0;
}
