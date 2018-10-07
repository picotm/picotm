/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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
