/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "picotm/picotm-tm.h"
#include "module.h"

PICOTM_EXPORT
void
__picotm_tm_load(uintptr_t addr, void* buf, size_t siz)
{
    tm_module_load(addr, buf, siz);
}

PICOTM_EXPORT
void
__picotm_tm_store(uintptr_t addr, const void* buf, size_t siz)
{
    tm_module_store(addr, buf, siz);
}

PICOTM_EXPORT
void
__picotm_tm_loadstore(uintptr_t laddr, uintptr_t saddr, size_t siz)
{
    tm_module_loadstore(laddr, saddr, siz);
}

PICOTM_EXPORT
void
__picotm_tm_privatize(uintptr_t addr, size_t siz, unsigned long flags)
{
    tm_module_privatize(addr, siz, flags);
}

PICOTM_EXPORT
void
__picotm_tm_privatize_c(uintptr_t addr, int c, unsigned long flags)
{
    tm_module_privatize_c(addr, c, flags);
}
