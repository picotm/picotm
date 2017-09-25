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

#include "opts.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "module.h"
#include "ptr.h"
#include "taputils.h"

enum boundary_type       g_btype = CYCLE_BOUND;
enum loop_mode           g_loop = INNER_LOOP;
unsigned int             g_off = 0;
const struct module*     g_module = NULL;
unsigned int             g_num = 0;
unsigned int             g_cycles = 10;
unsigned int             g_nthreads = 1;
unsigned int             g_normalize = 0;
enum picotm_libc_cc_mode g_cc_mode = PICOTM_LIBC_CC_MODE_2PL;
size_t                   g_txcycles = 1;

static enum parse_opts_result
opt_btype(const char* optarg)
{
    if (!strcmp("cycles", optarg)) {
        g_btype = CYCLE_BOUND;
    } else if (!strcmp("time", optarg)) {
        g_btype = TIME_BOUND;
    } else {
        return PARSE_OPTS_ERROR;
    }

    return PARSE_OPTS_OK;
}

static enum parse_opts_result
opt_cycles(const char *optarg)
{
    errno = 0;

    g_cycles = strtoul(optarg, NULL, 0);

    if (errno) {
        perror("strtoul()");
        return PARSE_OPTS_ERROR;
    }

    return PARSE_OPTS_OK;
}

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
opt_normalize(const char* optarg)
{
    g_normalize = 1;

    return PARSE_OPTS_OK;
}

static enum parse_opts_result
opt_nthreads(const char* optarg)
{
    errno = 0;

    g_nthreads = strtoul(optarg, NULL, 0);

    if (errno) {
        perror("strtoul()");
        return PARSE_OPTS_ERROR;
    }

    return PARSE_OPTS_OK;
}

static enum parse_opts_result
opt_num(const char* optarg)
{
    errno = 0;

    g_num = strtoul(optarg, NULL, 0);

    if (errno) {
        perror("strtoul()");
        return PARSE_OPTS_ERROR;
    }

    return PARSE_OPTS_OK;
}

static enum parse_opts_result
opt_off(const char* optarg)
{
    errno = 0;

    g_off = strtoul(optarg, NULL, 0);

    if (errno) {
        perror("strtoul()");
        return PARSE_OPTS_ERROR;
    }

    return PARSE_OPTS_OK;
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
opt_loop(const char* optarg)
{
    if (!strcmp("inner", optarg)) {
        g_loop = INNER_LOOP;
    } else if (!strcmp("time", optarg)) {
        g_loop = OUTER_LOOP;
    } else {
        return PARSE_OPTS_ERROR;
    }

    return PARSE_OPTS_OK;
}

static enum parse_opts_result
opt_tx_cycles(const char* optarg)
{
    errno = 0;

    g_txcycles = strtoul(optarg, NULL, 0);

    if (errno) {
        perror("strtoul()");
        return PARSE_OPTS_ERROR;
    }

    return PARSE_OPTS_OK;
}

static enum parse_opts_result
opt_verbose(const char* optarg)
{
    errno = 0;

    g_tap_verbosity = strtoul(optarg, NULL, 0);

    if (errno) {
        perror("strtoul()");
        return PARSE_OPTS_ERROR;
    }

    return PARSE_OPTS_OK;
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

enum parse_opts_result
parse_opts(int argc, char* argv[])
{
    /* Parse options */

    static enum parse_opts_result (* const opt[])(const char*) = {
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
        ['v'] = opt_verbose
    };

    if (argc < 2) {
        printf("enter `picotm-test -h` for a list of command-line options\n");
        return PARSE_OPTS_ERROR;
    }

    int c;

    while ((c = getopt(argc, argv, "I:L:NR:Vb:c:hm:n:o:t:v:")) != -1) {
        if ((c ==  '?') || (c == ':')) {
            return PARSE_OPTS_ERROR;
        }
        if (c >= (ssize_t)arraylen(opt) || !opt[c]) {
            return PARSE_OPTS_ERROR;
        }
        enum parse_opts_result res = opt[c](optarg);
        if (res) {
            return res;
        }
    }

    return PARSE_OPTS_OK;
}
