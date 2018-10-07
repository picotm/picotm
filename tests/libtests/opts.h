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

#pragma once

#include <assert.h>
#include <stddef.h>
#include "test.h"

extern enum boundary_type   g_btype;
extern enum loop_mode       g_loop;
extern unsigned int         g_off;
extern unsigned int         g_num;
extern unsigned int         g_cycles;
extern unsigned int         g_nthreads;
extern unsigned int         g_normalize;
extern size_t               g_txcycles;

enum parse_opts_result {
    PARSE_OPTS_OK,
    PARSE_OPTS_EXIT,
    PARSE_OPTS_ERROR
};

extern enum parse_opts_result (*parse_opt[256])(const char*);

#define PARSE_OPT(_opt, _parse_opt_func)        \
    {                                           \
        assert(!(parse_opt[(_opt)]));           \
        parse_opt[(_opt)] = (_parse_opt_func);  \
    }

#define PARSE_OPTS_STRING(_optstring)   \
    ("I:L:Nb:c:n:o:t:v:" _optstring)

enum parse_opts_result
parse_opts(int argc, char* argv[], const char* optstring);
