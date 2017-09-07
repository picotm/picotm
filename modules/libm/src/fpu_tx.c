/* Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "fpu_tx.h"
#include <errno.h>
#include <picotm/picotm-error.h>

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
