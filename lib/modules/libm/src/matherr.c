/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "err.h"
#include <errno.h>
#include <fenv.h>
#include <math.h>
#include <picotm/picotm-libc.h>
#include <picotm/picotm-libm.h>
#include <picotm/picotm-module.h>

/* Gcc does not support the FENV_ACCESS pragma. */
#ifndef __GNUC__
#pragma STDC FENV_ACCESS ON
#endif

void
matherr_save_and_clear(int except)
{
    if (math_errhandling & MATH_ERRNO) {
        picotm_libc_save_errno();
        errno = 0;
    }
    if (math_errhandling & MATH_ERREXCEPT) {
        picotm_libm_save_fexcept();
        feclearexcept(except);
    }
}

void
matherr_test_and_resolve(int except)
{
    if (math_errhandling & MATH_ERRNO) {
        if (errno) {
            picotm_resolve_error(errno);
        }
    }
    if (math_errhandling & MATH_ERREXCEPT) {
        int set_except = fetestexcept(except);
        if (set_except & FE_INVALID) {
            picotm_resolve_error(EDOM);
        } else if (set_except & FE_DIVBYZERO) {
            picotm_resolve_error(ERANGE);
        } else if (set_except & FE_OVERFLOW) {
            picotm_resolve_error(ERANGE);
        } else if (set_except & FE_UNDERFLOW) {
            picotm_resolve_error(ERANGE);
        }
    }
}
