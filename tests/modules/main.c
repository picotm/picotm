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

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <picotm/picotm-libc.h>
#include <unistd.h>
#include "module.h"
#include "ptr.h"
#include "tap.h"
#include "taputils.h"
#include "test.h"
#include "test_state.h"

static enum boundary_type   g_btype = BOUND_CYCLES;
static enum loop_mode       g_loop = LOOP_INNER;
static unsigned int         g_off = 0;
static const struct module* g_module = NULL;
static unsigned int         g_num = 0;
static unsigned int         g_cycles = 10;
static unsigned int         g_nthreads = 1;
static unsigned int         g_normalize = 0;

enum picotm_libc_cc_mode g_cc_mode = PICOTM_LIBC_CC_MODE_2PL;
size_t                   g_txcycles = 1;

static int
opt_btype(const char *optarg)
{
    if (!strcmp("cycles", optarg)) {
        g_btype = BOUND_CYCLES;
    } else if (!strcmp("time", optarg)) {
        g_btype = BOUND_TIME;
    } else {
        return -1;
    }

    return 0;
}

static int
opt_cycles(const char *optarg)
{
    errno = 0;

    g_cycles = strtoul(optarg, NULL, 0);

    if (errno) {
        perror("strtoul()");
        return -1;
    }

    return 0;
}

static int
opt_module(const char* optarg)
{
    const struct module* beg = module_list;
    const struct module* end = module_list + number_of_modules();

    while (beg < end) {
        if (!strcmp(beg->name, optarg)) {
            g_module = beg;
            return 0;
        }
        ++beg;
    }

    fprintf(stderr, "Unknown module %s\n", optarg);

    return -1;
}

static int
opt_normalize(const char *optarg)
{
    g_normalize = 1;

    return 0;
}

static int
opt_nthreads(const char *optarg)
{
    errno = 0;

    g_nthreads = strtoul(optarg, NULL, 0);

    if (errno) {
        perror("strtoul()");
        return -1;
    }

    return 0;
}

static int
opt_num(const char *optarg)
{
    errno = 0;

    g_num = strtoul(optarg, NULL, 0);

    if (errno) {
        perror("strtoul()");
        return -1;
    }

    return 0;
}

static int
opt_off(const char *optarg)
{
    errno = 0;

    g_off = strtoul(optarg, NULL, 0);

    if (errno) {
        perror("strtoul()");
        return -1;
    }

    return 0;
}

static int
opt_regular_ccmode(const char *optarg)
{
    static const char * const optstr[] = { "noundo", "2pl"};
    size_t i;

    for (i = 0; i < sizeof(optstr)/sizeof(optstr[0]); ++i) {
        if (!strcmp(optstr[i], optarg)) {
            g_cc_mode = i;
        }
    }

    return 0;
}

static int
opt_loop(const char *optarg)
{
    if (!strcmp("inner", optarg)) {
        g_loop = LOOP_INNER;
    } else if (!strcmp("time", optarg)) {
        g_loop = LOOP_OUTER;
    } else {
        return -1;
    }

    return 0;
}

static int
opt_tx_cycles(const char *optarg)
{
    errno = 0;

    g_txcycles = strtoul(optarg, NULL, 0);

    if (errno) {
        perror("strtoul()");
        return -1;
    }

    return 0;
}

static int
opt_verbose(const char *optarg)
{
    errno = 0;

    g_tap_verbosity = strtoul(optarg, NULL, 0);

    if (errno) {
        perror("strtoul()");
        return -1;
    }

    return 0;
}

static int
opt_help(const char *optarg)
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

    return 1;
}

static int
opt_version(const char *optarg)
{
    printf("picotm test application\n");
    printf("This software is licensed under the MIT License.\n");

    return 1;
}


static int
parse_opts(int argc, char *argv[])
{
    /* Parse options */

    extern char *optarg;

    static int (* const opt[])(const char*) = {
        ['I'] = opt_tx_cycles,
        ['L'] = opt_loop,
        ['N'] = opt_normalize,
        ['R'] = opt_regular_ccmode,
        ['V'] = opt_version,
        ['b'] = opt_btype,
        ['c'] = opt_cycles,
        ['h'] = opt_help,
        ['m'] = opt_module,
        ['n'] = opt_num,
        ['o'] = opt_off,
        ['t'] = opt_nthreads,
        ['v'] = opt_verbose};

    if (argc < 2) {
        printf("enter `picotm-test -h` for a list of command-line options\n");
        return 1;
    }

    int c;

    while ((c = getopt(argc, argv, "I:L:NR:Vb:c:hm:n:o:t:v:")) != -1) {
        if ((c ==  '?') || (c == ':')) {
            return -1;
        }
        if (c >= (ssize_t)(sizeof(opt)/sizeof(opt[0])) || !opt[c]) {
            return -1;
        }
        int res = opt[c](optarg);
        if (res) {
            return res;
        }
    }

    return 0;
}

int
main(int argc, char **argv)
{
    /* initialize */

    switch (parse_opts(argc, argv)) {
        case 0:
            break;
        case 1:
            return EXIT_SUCCESS;
        default:
            return EXIT_FAILURE;
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

            test_begin_on_thread(test_aborted)

                run_test(test_beg, g_nthreads, g_loop, g_btype, g_cycles);

            test_end_on_thread;

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
