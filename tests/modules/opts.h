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

#pragma once

#include <picotm/picotm-libc.h>
#include <stddef.h>
#include "test.h"

extern enum boundary_type       g_btype;
extern enum loop_mode           g_loop;
extern unsigned int             g_off;
extern const struct module*     g_module;
extern unsigned int             g_num;
extern unsigned int             g_cycles;
extern unsigned int             g_nthreads;
extern unsigned int             g_normalize;
extern enum picotm_libc_cc_mode g_cc_mode;
extern size_t                   g_txcycles;

enum parse_opts_result {
    PARSE_OPTS_OK,
    PARSE_OPTS_EXIT,
    PARSE_OPTS_ERROR
};

enum parse_opts_result
parse_opts(int argc, char* argv[]);
