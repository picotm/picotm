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
#include <string.h>
#include "fildes_test.h"
#include "module.h"
#include "opts.h"
#include "tap.h"
#include "test.h"

static const struct module* g_module = NULL;

static enum parse_opts_result
opt_module(const char* optarg)
{
    const struct module* beg = module_list;
    const struct module* end = module_list + number_of_modules();

    while (beg < end) {
        if (!strcmp(beg->name, optarg)) {
            g_module = beg;
            return PARSE_OPTS_OK;
        }
        ++beg;
    }

    fprintf(stderr, "Unknown module %s\n", optarg);

    return PARSE_OPTS_ERROR;
}

static enum parse_opts_result
opt_regular_ccmode(const char* optarg)
{
    static const char * const optstr[] = { "noundo", "2pl"};
    size_t i;

    for (i = 0; i < sizeof(optstr)/sizeof(optstr[0]); ++i) {
        if (!strcmp(optstr[i], optarg)) {
            g_cc_mode = i;
            return PARSE_OPTS_OK;
        }
    }

    fprintf(stderr, "unknown CC mode %s\n", optarg);

    return PARSE_OPTS_ERROR;
}

static enum parse_opts_result
opt_help(const char* optarg)
{
    printf("Usage: picotm-test [options]\n"
           "Options:\n"
           "  -V                            About this program\n"
           "  -h                            This help\n"
           "  -o <number>                   Number of first test, zero upwards\n"
           "  -m <module>                   Module name\n"
           "  -n <number>                   Number of tests, one upwards\n"
           "  -t <number>                   Number of concurrent threads\n"
           "  -I <number>                   Number of iterations in transaction\n"
           "  -R {noundo|ts|2pl|2pl-ext}    Set CC mode for file I/O\n"
           "                                  noundo: irrevocability\n"
           "                                  ts: timestamps\n"
           "                                  2pl: two-phase locking\n"
           "                                  2pl-ext: (inofficial) commit protocol for sockets\n"
           "  -L {inner|outer}              Loop mode\n"
           "  -b {time|cycles}              Bound for cycles\n"
           "                                  time: bound is maximum run time in milliseconds\n"
           "                                  cycles: cycles is maximum number of transaction runs\n"
           "  -c <number>                   Number of cycles, aka when to stop the test\n"
           "  -v                            Output benchmark results: '<threads> <commits> <aborts>'\n"
           "  -N                            Normalize >commits> to transactions/second\n"
           );

    return PARSE_OPTS_EXIT;
}

static enum parse_opts_result
opt_version(const char* optarg)
{
    printf("picotm test application\n");
    printf("This software is licensed under the MIT License.\n");

    return PARSE_OPTS_EXIT;
}

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
main(int argc, char** argv)
{
    PARSE_OPT('R', opt_regular_ccmode);
    PARSE_OPT('V', opt_version);
    PARSE_OPT('m', opt_module);
    PARSE_OPT('h', opt_help);

    switch (parse_opts(argc, argv, PARSE_OPTS_STRING("R:Vm:h"))) {
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
