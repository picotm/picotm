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

#include <stdio.h>
#include <stdlib.h>
#include "module.h"
#include "opts.h"
#include "safeblk.h"
#include "tap.h"

int
main(int argc, char **argv)
{
    switch (parse_opts(argc, argv)) {
        case PARSE_OPTS_EXIT:
            return EXIT_SUCCESS;
        case PARSE_OPTS_ERROR:
            return EXIT_FAILURE;
        default:
            break;
    }

    tap_version(12);

    const struct module* mod_beg;
    const struct module* mod_end;

    if (g_module) {
        mod_beg = g_module;
        mod_end = g_module + 1;
    } else {
        mod_beg = module_list;
        mod_end = module_list + number_of_modules();
    }

    while (mod_beg < mod_end) {

        if (mod_beg->number_of_tests() <= g_off) {
            fprintf(stderr, "Test index out of range\n");
            return -1;
        }

        size_t off = g_off;
        size_t num;

        if (!g_num) {
            num = mod_beg->number_of_tests() - off;
        } else if (mod_beg->number_of_tests() < (off + g_num)) {
            fprintf(stderr, "Test index out of range\n");
            abort();
        } else {
            num = g_num;
        }

        /* run tests  */

        const struct test_func* test_beg = mod_beg->test + off;
        const struct test_func* test_end = mod_beg->test + off + num;

        tap_plan((test_end - test_beg));

        unsigned long testnum = 1;

        while (test_beg < test_end) {

            int test_aborted = 0;

            begin_safe_block(test_aborted)

                run_test(test_beg, g_nthreads, g_loop, g_btype, g_cycles);

            end_safe_block

            if (test_aborted) {
                tap_not_ok(testnum, test_beg->name);
            } else {
                tap_ok(testnum, test_beg->name);
            }

            ++testnum;

            ++test_beg;
        }

        ++mod_beg;
    }

    return EXIT_SUCCESS;
}
