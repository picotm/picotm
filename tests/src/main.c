/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <picotm/picotm-libc.h>
#include <unistd.h>
#include "fdio_test.h"
#include "malloc_test.h"
#include "test.h"
#include "tm_test.h"
#include "vfs_test.h"

static const struct test_func test[] = {
    /* Test 0 */
    {"malloc_test_1", malloc_test_1, NULL, NULL},
    {"malloc_test_2", malloc_test_2, NULL, NULL},
    {"malloc_test_3", malloc_test_3, NULL, NULL},
    {"malloc_test_4", malloc_test_4, NULL, NULL},
    {"malloc_test_5", malloc_test_5, NULL, NULL},
    {"malloc_test_6", malloc_test_6, NULL, NULL},
    {"malloc_test_7", malloc_test_7, malloc_test_7_pre, NULL},
    {"malloc_test_8", malloc_test_8, malloc_test_8_pre, NULL},
    /* Test 8 */
    {"fdio_test_1", fdio_test_1, fdio_test_1_pre, fdio_test_1_post},
    {"fdio_test_2", fdio_test_2, fdio_test_2_pre, fdio_test_2_post},
    {"fdio_test_3", fdio_test_3, fdio_test_3_pre, fdio_test_3_post},
    {"fdio_test_4", fdio_test_4, fdio_test_4_pre, fdio_test_4_post},
    {"fdio_test_5", fdio_test_5, fdio_test_5_pre, fdio_test_5_post},
    {"fdio_test_6", fdio_test_6, fdio_test_6_pre, fdio_test_6_post},
    {"fdio_test_7", fdio_test_7, fdio_test_7_pre, fdio_test_7_post},
    {"fdio_test_8", fdio_test_8, fdio_test_8_pre, fdio_test_8_post},
    {"fdio_test_9", fdio_test_9, fdio_test_9_pre, fdio_test_9_post},
    {"fdio_test_10", fdio_test_10, fdio_test_10_pre, fdio_test_10_post},
    {"fdio_test_11", fdio_test_11, fdio_test_11_pre, fdio_test_11_post},
    {"fdio_test_12", fdio_test_12, fdio_test_12_pre, fdio_test_12_post},
    {"fdio_test_13", fdio_test_13, NULL, NULL},
    {"fdio_test_14", fdio_test_14, fdio_test_14_pre, fdio_test_14_post},
    {"fdio_test_15", fdio_test_15, fdio_test_15_pre, fdio_test_15_post},
    {"fdio_test_16", fdio_test_16, fdio_test_16_pre, fdio_test_16_post},
    {"fdio_test_17", fdio_test_17, fdio_test_17_pre, fdio_test_17_post},
    {"fdio_test_18", fdio_test_18, fdio_test_18_pre, fdio_test_18_post},
    {"fdio_test_19", fdio_test_19, NULL, NULL},
    {"fdio_test_20", fdio_test_20, fdio_test_20_pre, fdio_test_20_post},
    {"fdio_test_21", fdio_test_21, fdio_test_21_pre, fdio_test_21_post},
    {"fdio_test_22", fdio_test_22, fdio_test_22_pre, fdio_test_22_post},
    {"fdio_test_23", fdio_test_23, fdio_test_23_pre, fdio_test_23_post},
    {"fdio_test_24", fdio_test_24, fdio_test_24_pre, fdio_test_24_post},
    {"fdio_test_25", fdio_test_25, fdio_test_25_pre, fdio_test_25_post},
    {"fdio_test_26", fdio_test_26, fdio_test_26_pre, fdio_test_26_post},
    {"fdio_test_27", fdio_test_27, fdio_test_27_pre, fdio_test_27_post},
    {"fdio_test_28", fdio_test_28, fdio_test_28_pre, fdio_test_28_post},
    {"fdio_test_29", fdio_test_29, fdio_test_29_pre, fdio_test_29_post},
    {"fdio_test_30", fdio_test_30, fdio_test_30_pre, fdio_test_30_post},
    {"fdio_test_31", fdio_test_31, fdio_test_31_pre, fdio_test_31_post},
    {"fdio_test_32", fdio_test_32, fdio_test_32_pre, fdio_test_32_post},
    {"fdio_test_33", fdio_test_33, fdio_test_33_pre, fdio_test_33_post},
    {"fdio_test_34", fdio_test_34, fdio_test_34_pre, fdio_test_34_post},
    {"fdio_test_35", fdio_test_35, fdio_test_35_pre, fdio_test_35_post},
    {"fdio_test_36", fdio_test_36, fdio_test_36_pre, fdio_test_36_post},
    {"fdio_test_37", fdio_test_37, fdio_test_37_pre, fdio_test_37_post},
    {"fdio_test_38", fdio_test_38, fdio_test_38_pre, fdio_test_38_post},
    {"fdio_test_39", fdio_test_39, fdio_test_39_pre, fdio_test_39_post},
    {"fdio_test_40", fdio_test_40, fdio_test_40_pre, fdio_test_40_post},
    {"fdio_test_41", fdio_test_41, fdio_test_41_pre, fdio_test_41_post},
    {"fdio_test_42", fdio_test_42, fdio_test_42_pre, fdio_test_42_post},
    {"fdio_test_43", fdio_test_43, fdio_test_43_pre, fdio_test_43_post},
    {"fdio_test_44", fdio_test_44, fdio_test_44_pre, fdio_test_44_post},
    {"fdio_test_45", fdio_test_45, fdio_test_45_pre, fdio_test_45_post},
    {"fdio_test_46", fdio_test_46, fdio_test_46_pre, fdio_test_46_post},
    {"fdio_test_47", fdio_test_47, fdio_test_47_pre, fdio_test_47_post},
    {"fdio_test_48", fdio_test_48, fdio_test_48_pre, fdio_test_48_post},
    {"fdio_test_49", fdio_test_49, fdio_test_49_pre, fdio_test_49_post},
    {"fdio_test_52", fdio_test_50, fdio_test_50_pre, fdio_test_50_post},
    {"fdio_test_51", fdio_test_51, fdio_test_51_pre, fdio_test_51_post},
    {"fdio_test_52", fdio_test_52, fdio_test_52_pre, fdio_test_52_post},
    /* Test 60 */
    {"vfs_test_1", vfs_test_1, vfs_test_1_pre, vfs_test_1_post},
    /* Test 61 */
    {"malloc_test_9", malloc_test_9, malloc_test_9_pre, NULL},
    /* Test 62 */
    {"tm_test_1", tm_test_1, tm_test_1_pre, tm_test_1_post}
};

static enum boundary_type g_btype = BOUND_CYCLES;
static enum loop_mode     g_loop = LOOP_INNER;
static unsigned int       g_off = 0;
static unsigned int       g_num = sizeof(test)/sizeof(test[0]);
static unsigned int       g_cycles = 10;
static unsigned int       g_nthreads = 1;
static unsigned int       g_verbose = 0;
static unsigned int       g_normalize = 0;

enum picotm_libc_cc_mode g_cc_mode = PICOTM_LIBC_CC_MODE_2PL;
size_t                  g_txcycles = 1;

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
        perror("strtoul");
        return -1;
    }

    return 0;
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
        perror("strtoul");
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
        perror("strtoul");
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
        perror("strtoul");
        return -1;
    }

    return 0;
}

static int
opt_regular_ccmode(const char *optarg)
{
    static const char * const optstr[] = { "noundo", "ts", "2pl", "2pl-ext" };
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
        perror("strtoul");
        return -1;
    }

    return 0;
}

static int
opt_verbose(const char *optarg)
{
    errno = 0;

    g_verbose = strtoul(optarg, NULL, 0);

    if (errno) {
        perror("strtoul");
        return -1;
    }

    return 0;
}

static int
opt_help(const char *optarg)
{
    printf("Usage: tlctest [options]\n"
           "Options:\n"
           "  -V                            About this program\n"
           "  -h                            This help\n"
           "  -o <number>                   Number of first test, zero upwards\n"
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
    printf("Taglibc test application\n");
    printf("This software is licensed under the Mozilla Public License, v. 2.0.\n");

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
        ['n'] = opt_num,
        ['o'] = opt_off,
        ['t'] = opt_nthreads,
        ['v'] = opt_verbose};

    if (argc < 2) {
        printf("enter `tlctest -h` for a list of command-line options\n");
        return 1;
    }

    int c;

    while ((c = getopt(argc, argv, "I:L:NR:Vb:c:hn:o:t:v:")) != -1) {
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

static int
logmsg(unsigned int verbose, const char* format, va_list ap)
{
    if (g_verbose < verbose) {
        return 0;
    }
    return vprintf(format, ap);
}

static int
logmsg_result(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    int res = logmsg(1, format, ap);
    va_end(ap);

    return res;
}

static int
logmsg_debug(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    int res = logmsg(2, format, ap);
    va_end(ap);

    return res;
}

/* TODO: export this value from picotm */
long long abort_count;

extern long long fdio_test_21_ticks;
extern long long fdio_test_21_count;

extern long long pwrite_ticks;
extern long long pwrite_count;
extern long long pread_ticks;
extern long long pread_count;

extern long long lock_ticks;
extern long long lock_count;
extern long long unlock_ticks;
extern long long unlock_count;
extern long long validate_ticks;
extern long long validate_count;
extern long long apply_ticks;
extern long long apply_count;
extern long long updatecc_ticks;
extern long long updatecc_count;
extern long long undo_ticks;
extern long long undo_count;
extern long long finish_ticks;
extern long long finish_count;

/*extern long long pgtreess_lookup_ticks;
extern long long pgtreess_lookup_count;

extern long long ofdtx_updatecc_ticks_0;
extern long long ofdtx_updatecc_ticks_1;
extern long long ofdtx_updatecc_count;

extern long long ofdtx_pre_commit_ticks_0;
extern long long ofdtx_pre_commit_ticks_1;
extern long long ofdtx_pre_commit_count;

extern long long ofdtx_post_commit_ticks_0;
extern long long ofdtx_post_commit_count;
*/
/*extern long long cmapss_unlock_region_ticks_0;
extern long long cmapss_unlock_region_ticks_1;
extern long long cmapss_unlock_region_count;*/
/*
extern long long com_fd_finish_ticks_0;
extern long long com_fd_finish_ticks_1;
extern long long com_fd_finish_count;*/

/*extern long long pgtree_lookup_ticks;
extern long long pgtree_lookup_count;*/

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

    long long ntx = 0;

    /* run tests  */

    const struct test_func *t;

    for (t = test+g_off; t < test+g_off+g_num; ++t) {
        ntx = run_test(t, g_nthreads, g_loop, g_btype, g_cycles, logmsg_debug);
        if (ntx < 0) {
            abort();
        }
    }

    /* print results */

    if (g_normalize) {
        logmsg_result("%d %lld %lld\n", g_nthreads, (long long)((ntx*1000)/g_cycles), abort_count);

            /*printf("fdio21_ticks=%lld fdio21_count=%lld, average=%lld\n", fdio_test_21_ticks,
                                                                          fdio_test_21_count,
                                                                          fdio_test_21_count ? (fdio_test_21_ticks/fdio_test_21_count) : 0);

            printf("pread_ticks=%lld pread_count=%lld, average=%lld\n", pread_ticks,
                                                                        pread_count,
                                                                        pread_count ? (pread_ticks/pread_count) : 0);

            printf("pwrite_ticks=%lld pwrite_count=%lld, average=%lld\n", pwrite_ticks,
                                                                          pwrite_count,
                                                                          pwrite_count ? (pwrite_ticks/pwrite_count) : 0);

            printf("lock_ticks=%lld lock_count=%lld, average=%lld\n", lock_ticks,
                                                                      lock_count,
                                                                      lock_count ? (lock_ticks/lock_count) : 0);

            printf("unlock_ticks=%lld unlock_count=%lld, average=%lld\n", unlock_ticks,
                                                                          unlock_count,
                                                                          unlock_count ? (unlock_ticks/validate_count) : 0);

            printf("validate_ticks=%lld validate_count=%lld, average=%lld\n", validate_ticks,
                                                                              validate_count,
                                                                              validate_count ? (validate_ticks/validate_count) : 0);

            printf("apply_ticks=%lld apply_count=%lld, average=%lld\n", apply_ticks,
                                                                        apply_count,
                                                                        apply_count ? (apply_ticks/apply_count) : 0);

            printf("updatecc_ticks=%lld updatecc_count=%lld, average=%lld\n", updatecc_ticks,
                                                                              updatecc_count,
                                                                              updatecc_count ? (updatecc_ticks/updatecc_count) : 0);

            printf("undo_ticks=%lld undo_count=%lld, average=%lld\n", undo_ticks,
                                                                      undo_count,
                                                                      undo_count ? (undo_ticks/undo_count): 0);

            printf("finish_ticks=%lld finish_count=%lld, average=%lld\n", finish_ticks,
                                                                          finish_count,
                                                                          finish_count ? (finish_ticks/finish_count): 0);*/

/*            printf("pgtreess_lookup_ticks=%lld pgtreess_lookup_count=%lld, average=%lld\n", pgtreess_lookup_ticks,
                                                                                            pgtreess_lookup_count,
                                                                                            pgtreess_lookup_count ? (pgtreess_lookup_ticks/pgtreess_lookup_count): 0);

            printf("ofdtx_pre_commit_ticks_0=%lld ofdtx_pre_commit_ticks_1=%lld ofdtx_pre_commit_count=%lld, average0=%lld, average1=%lld\n", ofdtx_pre_commit_ticks_0,
                                                                                                                                              ofdtx_pre_commit_ticks_1,
                                                                                                                                              ofdtx_pre_commit_count,
                                                                                                                                              ofdtx_pre_commit_count ? (ofdtx_pre_commit_ticks_0/ofdtx_pre_commit_count): 0,
                                                                                                                                              ofdtx_pre_commit_count ? (ofdtx_pre_commit_ticks_1/ofdtx_pre_commit_count): 0);

            printf("ofdtx_post_commit_ticks_0=%lld ofdtx_post_commit_count=%lld, average0=%lld\n", ofdtx_post_commit_ticks_0,
                                                                                                   ofdtx_post_commit_count,
                                                                                                   ofdtx_post_commit_count ? (ofdtx_post_commit_ticks_0/ofdtx_post_commit_count): 0);
*/
/*            printf("cmapss_unlock_region_ticks_0=%lld cmapss_unlock_region_ticks_1=%lld cmapss_unlock_region_count=%lld, average0=%lld, average1=%lld\n", cmapss_unlock_region_ticks_0,
                                                                                                                                                          cmapss_unlock_region_ticks_1,
                                                                                                                                                          cmapss_unlock_region_count,
                                                                                                                                                          cmapss_unlock_region_count ? (cmapss_unlock_region_ticks_0/cmapss_unlock_region_count): 0,
                                                                                                                                                          cmapss_unlock_region_count ? (cmapss_unlock_region_ticks_1/cmapss_unlock_region_count): 0);
*/
/*            printf("com_fd_finish_ticks_0=%lld com_fd_finish_ticks_1=%lld com_fd_finish_count=%lld, average0=%lld, average1=%lld\n", com_fd_finish_ticks_0,
                                                                                                                                     com_fd_finish_ticks_1,
                                                                                                                                     com_fd_finish_count,
                                                                                                                                     com_fd_finish_count ? (com_fd_finish_ticks_0/com_fd_finish_count): 0,
                                                                                                                                     com_fd_finish_count ? (com_fd_finish_ticks_1/com_fd_finish_count): 0);

            printf("ofdtx_updatecc_ticks_0=%lld ofdtx_updatecc_ticks_1=%lld ofdtx_updatecc_count=%lld, average0=%lld, average1=%lld\n", ofdtx_updatecc_ticks_0,
                                                                                                                                        ofdtx_updatecc_ticks_1,
                                                                                                                                        ofdtx_updatecc_count,
                                                                                                                                        ofdtx_updatecc_count ? (ofdtx_updatecc_ticks_0/ofdtx_updatecc_count): 0,
                                                                                                                                        ofdtx_updatecc_count ? (ofdtx_updatecc_ticks_1/ofdtx_updatecc_count): 0);
*/

/*            printf("pgtree_lookup_ticks=%lld pgtree_lookup_count=%lld, average=%lld\n", pgtree_lookup_ticks,
                                                                                        pgtree_lookup_count,
                                                                                        pgtree_lookup_count ? (pgtree_lookup_ticks/pgtree_lookup_count): 0);*/
    } else {
        logmsg_result("%d %lld %lld\n", g_nthreads, ntx, abort_count);
    }

    return EXIT_SUCCESS;
}

