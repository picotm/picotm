/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
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
 */

#include "picotm/picotm-libc.h"
#include <stdatomic.h>
#include "error/module.h"

/*
 * Error handling
 */

PICOTM_EXPORT
void
picotm_libc_save_errno()
{
    error_module_save_errno();
}

PICOTM_EXPORT
void
picotm_libc_set_error_recovery(enum picotm_libc_error_recovery recovery)
{
    return error_module_set_error_recovery(recovery);
}

PICOTM_EXPORT
enum picotm_libc_error_recovery
picotm_libc_get_error_recovery()
{
    return error_module_get_error_recovery();
}

/*
 * File I/O
 */

static _Atomic enum picotm_libc_cc_mode g_file_type_cc_mode[] = {
    PICOTM_LIBC_CC_MODE_2PL,
    PICOTM_LIBC_CC_MODE_2PL,
    PICOTM_LIBC_CC_MODE_2PL,
    PICOTM_LIBC_CC_MODE_2PL,
    PICOTM_LIBC_CC_MODE_2PL,
    PICOTM_LIBC_CC_MODE_2PL,
    PICOTM_LIBC_CC_MODE_2PL
};

PICOTM_EXPORT
void
picotm_libc_set_file_type_cc_mode(enum picotm_libc_file_type file_type,
                                  enum picotm_libc_cc_mode cc_mode)
{
    atomic_store_explicit(g_file_type_cc_mode + file_type, cc_mode,
                          memory_order_release);
}

PICOTM_EXPORT
enum picotm_libc_cc_mode
picotm_libc_get_file_type_cc_mode(enum picotm_libc_file_type file_type)
{
    return atomic_load_explicit(g_file_type_cc_mode + file_type,
                                memory_order_acquire);
}
