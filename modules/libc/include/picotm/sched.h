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

#pragma once

#include "picotm/config/picotm-libc-config.h"
#include "picotm/compiler.h"
#include <sched.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <sched.h>.
 */

#if defined(PICOTM_LIBC_HAVE_SCHED_YIELD) && PICOTM_LIBC_HAVE_SCHED_YIELD || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [sched_yield()][posix::sched_yield].
 *
 * [posix::sched_yield]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/sched_yield.html
 */
int
sched_yield_tx(void);
#endif

PICOTM_END_DECLS
