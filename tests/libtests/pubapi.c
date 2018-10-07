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

#include "pubapi.h"
#include <stdio.h>
#include <stdlib.h>
#include "opts.h"
#include "tap.h"
#include "test.h"

int
pubapi_main(int argc, char** argv, const char* optstring,
            const struct test_func* test, size_t ntests)
{
    switch (parse_opts(argc, argv, optstring)) {
        case PARSE_OPTS_EXIT:
            return EXIT_SUCCESS;
        case PARSE_OPTS_ERROR:
            return EXIT_FAILURE;
        default:
            break;
    }

    if (ntests <= g_off) {
        fprintf(stderr, "Test index out of range\n");
        return EXIT_FAILURE;
    }

    size_t off = g_off;
    size_t num;

    if (!g_num) {
        num = ntests - off;
    } else {
        num = g_num;
    }

    if (ntests < (off + g_num)) {
        fprintf(stderr, "Test index out of range\n");
        return EXIT_FAILURE;
    }

    /* run tests  */

    tap_version(12);

    const struct test_func* test_beg = test + off;
    const struct test_func* test_end = test + off + num;

    run_tests(test_beg, test_end, g_nthreads, g_loop, g_btype, g_cycles);

    return EXIT_SUCCESS;
}
