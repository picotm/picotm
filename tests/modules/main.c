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
#include "tap.h"
#include "test.h"

static int
run_module_tests(const struct module* mod, size_t off, size_t num,
                 unsigned long nthreads,
                 enum loop_mode loop, enum boundary_type btype,
                 unsigned long long limit)
{
    if (mod->number_of_tests() <= off) {
        fprintf(stderr, "Test index out of range\n");
        return -1;
    }

    if (!num) {
        num = mod->number_of_tests() - off;
    }

    if (mod->number_of_tests() < (off + num)) {
        fprintf(stderr, "Test index out of range\n");
        return -1;
    }

    /* run tests  */

    const struct test_func* test_beg = mod->test + off;
    const struct test_func* test_end = mod->test + off + num;

    run_tests(test_beg, test_end, nthreads, loop, btype, limit);

    return 0;
}

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

        int res = run_module_tests(mod_beg, g_off, g_num, g_nthreads, g_loop, g_btype,
                                   g_cycles);
        if (res < 0) {
            tap_bail_out("");
            return EXIT_FAILURE;
        }

        ++mod_beg;
    }

    return EXIT_SUCCESS;
}
