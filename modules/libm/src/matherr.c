/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
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
 *
 * SPDX-License-Identifier: MIT
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
