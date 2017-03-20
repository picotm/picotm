/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <tanger-stm.h>
#include <tanger-stm-ext-actions.h>
#include "malloc_test.h"
#include "fdio_test.h"
#include "fs_test.h"
#include "socket_test.h"
#include "error_test.h"

/* Note:
 *
 * This program needs a directory ./testdir with the files
 *
 *   testfile.bin      (1 MiB, random)
 *   testfile-100.bin  (100 MiB, random)
 *   testfile2.bin     (1 MiB, all zeros)
 *   testfile2-100.bin (100MiB, all zeros)
 *
 * Possible segfault on sedell17:
 *
 *   ./tlctest -o6 -c60000 -btime -n1 -v1 -I100 -N -t15
 *
 * Bug is not reproducible on sedell05.
 */

typedef void (* const pre_func)(void);
typedef void (* const post_func)(void);
typedef void (* const call_func)(unsigned int);

struct test_func
{
    const char *name;
    call_func   call;
    pre_func    pre;
    post_func   post;
};

enum boundary_type
{
    BOUND_CYCLES = 0,
    BOUND_TIME
};

enum loop_mode {
    LOOP_INNER = 0,
    LOOP_OUTER
};

struct test_func_args
{
    void             (*call)(unsigned int);
    void             (*pre)(void);
    void             (*post)(void);
    unsigned int       tid; /* Logical thread ID*/
    enum boundary_type btype; /* Boundary type */
    unsigned long      bound; /* Time (ms) or Cycles to run */
    unsigned long long ntx; /* Number of succesful transactions, return value */
};

static const struct test_func test[] = {
    /* Test 0 */
    {"malloc_test_1", tanger_stm_malloc_test_1, NULL, NULL},
    {"malloc_test_2", tanger_stm_malloc_test_2, NULL, NULL},
    {"malloc_test_3", tanger_stm_malloc_test_3, NULL, NULL},
    {"malloc_test_4", tanger_stm_malloc_test_4, NULL, NULL},
    {"malloc_test_5", tanger_stm_malloc_test_5, NULL, NULL},
    {"malloc_test_6", tanger_stm_malloc_test_6, NULL, NULL},
    {"malloc_test_7", tanger_stm_malloc_test_7, NULL, NULL},
    {"malloc_test_8", tanger_stm_malloc_test_8, NULL, NULL},
    /* Test 8 */
    {"fdio_test_1", tanger_stm_fdio_test_1, NULL, NULL},
    {"fdio_test_2", tanger_stm_fdio_test_2, NULL, NULL},
    {"fdio_test_3", tanger_stm_fdio_test_3, NULL, NULL},
    {"fdio_test_4", tanger_stm_fdio_test_4, NULL, NULL},
    {"fdio_test_5", tanger_stm_fdio_test_5, NULL, NULL},
    {"fdio_test_6", tanger_stm_fdio_test_6, NULL, NULL},
    {"fdio_test_7", tanger_stm_fdio_test_7, NULL, NULL},
    {"fdio_test_8", tanger_stm_fdio_test_8, NULL, NULL},
    {"fdio_test_9", tanger_stm_fdio_test_9, NULL, NULL},
    {"fdio_test_10", tanger_stm_fdio_test_10, NULL, NULL},
    {"fdio_test_11", tanger_stm_fdio_test_11, NULL, NULL},
    {"fdio_test_12", tanger_stm_fdio_test_12, NULL, NULL},
    {"fdio_test_13", tanger_stm_fdio_test_13, NULL, NULL},
    {"fdio_test_14", tanger_stm_fdio_test_14, NULL, NULL},
    {"fdio_test_15", tanger_stm_fdio_test_15, NULL, NULL},
    {"fdio_test_16", tanger_stm_fdio_test_16, NULL, NULL},
    {"fdio_test_17", tanger_stm_fdio_test_17, NULL, NULL},
    {"fdio_test_18", tanger_stm_fdio_test_18, NULL, NULL},
    {"fdio_test_19", tanger_stm_fdio_test_19, NULL, NULL},
    {"fdio_test_20", tanger_stm_fdio_test_20, tanger_stm_fdio_test_20_pre, tanger_stm_fdio_test_20_post},
    {"fdio_test_21", tanger_stm_fdio_test_21, tanger_stm_fdio_test_21_pre, tanger_stm_fdio_test_21_post},
    {"fdio_test_22", tanger_stm_fdio_test_22, tanger_stm_fdio_test_22_pre, tanger_stm_fdio_test_22_post},
    {"fdio_test_23", tanger_stm_fdio_test_23, tanger_stm_fdio_test_23_pre, tanger_stm_fdio_test_23_post},
    {"fdio_test_24", tanger_stm_fdio_test_24, tanger_stm_fdio_test_24_pre, tanger_stm_fdio_test_24_post},
    {"fdio_test_25", tanger_stm_fdio_test_25, tanger_stm_fdio_test_25_pre, tanger_stm_fdio_test_25_post},
    {"fdio_test_26", tanger_stm_fdio_test_26, tanger_stm_fdio_test_26_pre, tanger_stm_fdio_test_26_post},
    {"fdio_test_27", tanger_stm_fdio_test_27, tanger_stm_fdio_test_27_pre, tanger_stm_fdio_test_27_post},
    {"fdio_test_28", tanger_stm_fdio_test_28, tanger_stm_fdio_test_28_pre, tanger_stm_fdio_test_28_post},
    {"fdio_test_29", tanger_stm_fdio_test_29, tanger_stm_fdio_test_29_pre, tanger_stm_fdio_test_29_post},
    {"fdio_test_30", tanger_stm_fdio_test_30, tanger_stm_fdio_test_30_pre, tanger_stm_fdio_test_30_post},
    {"fdio_test_31", tanger_stm_fdio_test_31, tanger_stm_fdio_test_31_pre, tanger_stm_fdio_test_31_post},
    {"fdio_test_32", tanger_stm_fdio_test_32, tanger_stm_fdio_test_32_pre, tanger_stm_fdio_test_32_post},
    {"fdio_test_33", tanger_stm_fdio_test_33, tanger_stm_fdio_test_33_pre, tanger_stm_fdio_test_33_post},
    {"fdio_test_34", tanger_stm_fdio_test_34, tanger_stm_fdio_test_34_pre, tanger_stm_fdio_test_34_post},
    {"fdio_test_35", tanger_stm_fdio_test_35, tanger_stm_fdio_test_35_pre, tanger_stm_fdio_test_35_post},
    {"fdio_test_36", tanger_stm_fdio_test_36, tanger_stm_fdio_test_36_pre, tanger_stm_fdio_test_36_post},
    {"fdio_test_37", tanger_stm_fdio_test_37, tanger_stm_fdio_test_37_pre, tanger_stm_fdio_test_37_post},
    {"fdio_test_38", tanger_stm_fdio_test_38, tanger_stm_fdio_test_38_pre, tanger_stm_fdio_test_38_post},
    {"fdio_test_39", tanger_stm_fdio_test_39, tanger_stm_fdio_test_39_pre, tanger_stm_fdio_test_39_post},
    {"fdio_test_40", tanger_stm_fdio_test_40, tanger_stm_fdio_test_40_pre, tanger_stm_fdio_test_40_post},
    {"fdio_test_41", tanger_stm_fdio_test_41, tanger_stm_fdio_test_41_pre, tanger_stm_fdio_test_41_post},
    {"fdio_test_42", tanger_stm_fdio_test_42, tanger_stm_fdio_test_42_pre, tanger_stm_fdio_test_42_post},
    {"fdio_test_43", tanger_stm_fdio_test_43, tanger_stm_fdio_test_43_pre, tanger_stm_fdio_test_43_post},
    {"fdio_test_44", tanger_stm_fdio_test_44, tanger_stm_fdio_test_44_pre, tanger_stm_fdio_test_44_post},
    {"fdio_test_45", tanger_stm_fdio_test_45, tanger_stm_fdio_test_45_pre, tanger_stm_fdio_test_45_post},
    {"fdio_test_46", tanger_stm_fdio_test_46, tanger_stm_fdio_test_46_pre, tanger_stm_fdio_test_46_post},
    {"fdio_test_47", tanger_stm_fdio_test_47, tanger_stm_fdio_test_47_pre, tanger_stm_fdio_test_47_post},
    {"fdio_test_48", tanger_stm_fdio_test_48, tanger_stm_fdio_test_48_pre, tanger_stm_fdio_test_48_post},
    {"fdio_test_49", tanger_stm_fdio_test_49, tanger_stm_fdio_test_49_pre, tanger_stm_fdio_test_49_post},
    {"fdio_test_52", tanger_stm_fdio_test_50, tanger_stm_fdio_test_50_pre, tanger_stm_fdio_test_50_post},
    {"fdio_test_51", tanger_stm_fdio_test_51, tanger_stm_fdio_test_51_pre, tanger_stm_fdio_test_51_post},
    {"fdio_test_52", tanger_stm_fdio_test_52, tanger_stm_fdio_test_52_pre, tanger_stm_fdio_test_52_post},
    /* Test 60 */
    {"fs_test_1", tanger_stm_fs_test_1, NULL, NULL},
    /* Test 61 */
    {"socket_test_1", tanger_stm_socket_test_1, NULL, NULL},
    {"socket_test_2", tanger_stm_socket_test_2, NULL, NULL},
    {"socket_test_3", tanger_stm_socket_test_3, NULL, NULL},
    {"socket_test_4", tanger_stm_socket_test_4, NULL, NULL},
    /* Test 65 */
    {"error_test_1", tanger_stm_error_test_1, NULL, NULL},
    {"error_test_2", tanger_stm_error_test_2, NULL, NULL},
    {"error_test_3", tanger_stm_error_test_3, NULL, NULL},
    /* Test 68 */
    {"malloc_test_9", tanger_stm_malloc_test_9, NULL, NULL}};

static enum boundary_type g_btype = BOUND_CYCLES;
static enum loop_mode     g_loop = LOOP_INNER;
static unsigned int       g_off = 0;
static unsigned int       g_num = sizeof(test)/sizeof(test[0]);
static unsigned int       g_cycles = 10;
static unsigned int       g_nthreads = 1;
static unsigned int       g_verbose = 0;
static unsigned int       g_normalize = 0;

/* Helgrind 3.3 does not support barrieres, so you might get a warning here */
static pthread_barrier_t g_waitbeg;

enum ccmode g_ccmode   = CC_MODE_2PL;
size_t      g_txcycles = 1;

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
    static const char * const optstr[] = { "noundo",  "ts", "2pl", "2pl-ext" };
    size_t i;

    for (i = 0; i < sizeof(optstr)/sizeof(optstr[0]); ++i) {
        if (!strcmp(optstr[i], optarg)) {
            g_ccmode = i;
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

    exit(EXIT_SUCCESS);

    return 0;
}

static int
opt_version(const char *optarg)
{
    printf("Taglibc test application\n");
    printf("This software is licensed under the Mozilla Public License, v. 2.0.\n");

    exit(EXIT_SUCCESS);

    return 0;
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

    int c;

    if (argc < 2) {
        printf("enter `tlctest -h` for a list of command-line options\n");
        exit(EXIT_SUCCESS);
    }

    while ((c = getopt(argc, argv, "I:L:NR:Vb:c:hn:o:t:v:")) != -1) {
        if ((c ==  '?') || (c == ':')) {
            return -1;
        }
        if ((c < sizeof(opt)/sizeof(opt[0])) && opt[c] && (opt[c](optarg) < 0)) {
            return -1;
        }
    }

    return 0;
}


static int
is_barrier_wait_error(int code)
{
    return code && (code != PTHREAD_BARRIER_SERIAL_THREAD);
}

/* Returns the number of milliseconds since the epoch */
static unsigned long long
getmsofday(void *tzp)
{
    struct timeval t;
    gettimeofday(&t, tzp);

    return t.tv_sec*1000 + t.tv_usec/1000;
}

/* Inner loops
 */

static void *
inner_loop_func_cycles(void *arg)
{
    struct test_func_args *args = arg;

    assert(arg);

    tanger_thread_init();

    if (is_barrier_wait_error(pthread_barrier_wait(&g_waitbeg))) {
        perror("pthread_barrier_wait");
        exit(EXIT_FAILURE);
    }

    args->ntx = 0;

    int i;

    for (i = 0; i < args->bound; ++i) {
        args->call(args->tid);
        ++args->ntx;
    }

    tanger_thread_shutdown();

    return NULL;
}

static void *
inner_loop_func_time(void *arg)
{
    struct test_func_args *args = arg;

    assert(arg);

    tanger_thread_init();

    if (is_barrier_wait_error(pthread_barrier_wait(&g_waitbeg))) {
        perror("pthread_barrier_wait");
        exit(EXIT_FAILURE);
    }

    args->ntx = 0;

    const unsigned long long begms = getmsofday(NULL);

    unsigned long long ms = 0;

    while (ms < args->bound) {
        args->call(args->tid);
        ++args->ntx;

        ms = getmsofday(NULL)-begms;
    }

    tanger_thread_shutdown();

    return NULL;
}

static long long
run_inner_loop(const struct test_func *test, enum boundary_type btype, unsigned long long bound)
{
    pthread_t *thread = malloc(g_nthreads*sizeof(thread[0]));

    if (!thread) {
        perror("malloc");
        return -1;
    }

    struct test_func_args *fargs = malloc(g_nthreads*sizeof(fargs[0]));

    if (!fargs) {
        perror("malloc");
        return -1;
    }

    if (g_verbose > 1) {
        printf("Running test %s...\n", test->name);
    }

    if (test->pre) {
        test->pre();
    }

    pthread_t *th;

    for (th = thread; th < thread+g_nthreads; ++th) {

        static void* (* const inner_loop_func[])(void*) = {
            inner_loop_func_cycles,
            inner_loop_func_time};

        const unsigned int i = th-thread;
        int err;

        fargs[i].call = test->call;
        fargs[i].tid = i;
        fargs[i].btype = btype;
        fargs[i].bound = bound;
        fargs[i].ntx = 0;

        if ( !!(err = pthread_create(th, NULL, inner_loop_func[btype], fargs+i)) ) {
            errno = err;
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    long long ntx = 0;

    for (th = thread; th < thread+g_nthreads; ++th) {
        int err;
        if ( (err = pthread_join(*th, NULL)) != 0 ) {
            errno = err;
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }

        const unsigned int i = th-thread;

        ntx += fargs[i].ntx;
    }

    if (test->post) {
        test->post();
    }

    free(fargs);
    free(thread);

    return ntx;
}

/* Outer loops
 */

static void *
outer_loop_func(void *arg)
{
    struct test_func_args *args = arg;

    assert(arg);

    tanger_thread_init();

    if (is_barrier_wait_error(pthread_barrier_wait(&g_waitbeg))) {
        perror("pthread_barrier_wait");
        exit(EXIT_FAILURE);
    }

    args->call(args->tid);

    tanger_thread_shutdown();

    args->ntx = 1;

    return NULL;
}

static long
run_outer_loop_cycles(const struct test_func *test, int cycles)
{
    pthread_t *thread = malloc(g_nthreads*sizeof(thread[0]));

    if (!thread) {
        perror("malloc");
        return -1;
    }

    struct test_func_args *fargs = malloc(g_nthreads*sizeof(fargs[0]));

    if (!fargs) {
        perror("malloc");
        return -1;
    }

    if (test->pre) {
        test->pre();
    }

    long long ntx = 0;
    int i;

    for (i = 1; i <= cycles; ++i) {

        if (g_verbose > 1) {
            printf("Running test %s [%d of %d]...\n", test->name, i, cycles);
        }

        pthread_t *th;

        for (th = thread; th < thread+g_nthreads; ++th) {

            int err;
            unsigned int i = th-thread;

            fargs[i].call = test->call;
            fargs[i].tid = i;
            fargs[i].btype = BOUND_CYCLES;
            fargs[i].bound = cycles;
            fargs[i].ntx = 0;

            if ( (err = pthread_create(th, NULL, outer_loop_func, fargs+i)) ) {
                errno = err;
                perror("pthread_create");
                exit(EXIT_FAILURE);
            }
        }

        for (th = thread; th < thread+g_nthreads; ++th) {
            int err;
            if ( (err = pthread_join(*th, NULL)) ) {
                errno = err;
                perror("pthread_join");
                exit(EXIT_FAILURE);
            }

            unsigned int i = th-thread;

            ntx += fargs[i].ntx;
        }
    }

    if (test->post) {
        test->post();
    }

    free(fargs);
    free(thread);

    return ntx;
}

static long
run_outer_loop_time(const struct test_func *test, int ival_ms)
{
    pthread_t *thread = malloc(g_nthreads*sizeof(thread[0]));

    if (!thread) {
        perror("malloc");
        return -1;
    }

    struct test_func_args *fargs = malloc(g_nthreads*sizeof(fargs[0]));

    if (!fargs) {
        perror("malloc");
        return -1;
    }

    if (g_verbose > 1) {
        printf("Running test %s [%d ms]...\n", test->name, ival_ms);
    }

    if (test->pre) {
        test->pre();
    }

    long long ntx = 0;

    const unsigned long long begms = getmsofday(NULL);
    unsigned long long ms = 0;

    while (ms < ival_ms) {

        pthread_t *th;

        for (th = thread; th < thread+g_nthreads; ++th) {

            int err;
            const unsigned int i = th-thread;

            fargs[i].call = test->call;
            fargs[i].tid = i;
            fargs[i].btype = BOUND_TIME;
            fargs[i].bound = ival_ms;
            fargs[i].ntx = 0;

            if ( (err = pthread_create(th, NULL, outer_loop_func, fargs+i)) ) {
                errno = err;
                perror("pthread_create");
                exit(EXIT_FAILURE);
            }
        }

        for (th = thread; th < thread+g_nthreads; ++th) {
            int err;
            if ( (err = pthread_join(*th, NULL)) ) {
                errno = err;
                perror("pthread_join");
                exit(EXIT_FAILURE);
            }

            const unsigned int i = th-thread;
            ntx += fargs[i].ntx;
        }

        ms = getmsofday(NULL)-begms;
    }

    if (test->post) {
        test->post();
    }

    free(fargs);
    free(thread);

    return ntx;
}

static long long
run_outer_loop(const struct test_func* test, enum boundary_type btype,
               unsigned long long bound)
{
    static long (* const loop_func[])(const struct test_func*, int) = {
            run_outer_loop_cycles,
            run_outer_loop_time
    };
    return loop_func[btype](test, bound);
}

static long long
run_test(const struct test_func* test, enum loop_mode loop,
         enum boundary_type btype, unsigned long long bound)
{
    static long (* const loop_func[])(const struct test_func*,
                                      enum loop_mode,
                                      enum boundary_type,
                                      unsigned long long) = {
            run_inner_loop,
            run_outer_loop
    };

    return loop_func[loop](test, btype, bound);
}

/* comes from TinySTM */
extern long long abort_count;

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
    int err;

    /* initialize */

    if (parse_opts(argc, argv) < 0) {
        exit(EXIT_FAILURE);
    }

    long long ntx = 0;

    /* run tests */

    if ( (err = pthread_barrier_init(&g_waitbeg, NULL, g_nthreads)) ) {
        errno = err;
        perror("pthread_barriere_init");
        exit(EXIT_FAILURE);
    }

    tanger_init();

    const struct test_func *t;

    for (t = test+g_off; t < test+g_off+g_num; ++t) {
        ntx = run_test(t, g_loop, g_btype, g_cycles);
        if (ntx < 0) {
            exit(EXIT_FAILURE);
        }
    }

    tanger_shutdown();

    if ( (err = pthread_barrier_destroy(&g_waitbeg)) ) {
        errno = err;
        perror("pthread_barriere_destroy");
        exit(EXIT_FAILURE);
    }

    /* print results */

    if (g_verbose) {
        if (g_normalize) {
            printf("%d %lld %lld\n", g_nthreads, (long long)((ntx*1000)/g_cycles), abort_count);

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
            printf("%d %lld %lld\n", g_nthreads, ntx, abort_count);
        }
    }

    exit(EXIT_SUCCESS);
}

