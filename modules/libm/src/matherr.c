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

#include "matherr.h"
#include "picotm/picotm-libc.h"
#include "picotm/picotm-module.h"
#include <errno.h>
#include <fenv.h>
#include <math.h>
#include "module.h"

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
        fpu_module_save_fexcept();
        feclearexcept(except);
    }
}

void
matherr_test_and_resolve(int except)
{
    if (math_errhandling & MATH_ERRNO) {
        if (errno) {
            picotm_recover_from_errno(errno);
        }
    }
    if (math_errhandling & MATH_ERREXCEPT) {
        int set_except = fetestexcept(except);
        if (set_except & FE_INVALID) {
            picotm_recover_from_errno(EDOM);
        } else if (set_except & FE_DIVBYZERO) {
            picotm_recover_from_errno(ERANGE);
        } else if (set_except & FE_OVERFLOW) {
            picotm_recover_from_errno(ERANGE);
        } else if (set_except & FE_UNDERFLOW) {
            picotm_recover_from_errno(ERANGE);
        }
    }
}
