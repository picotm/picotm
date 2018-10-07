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
