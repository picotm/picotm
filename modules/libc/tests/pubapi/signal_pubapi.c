/*
 * MIT License
 * Copyright (c) 2018   Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "picotm/picotm.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-libc.h"
#include <pthread.h>
#include "ptr.h"
#include "safeblk.h"
#include "taputils.h"
#include "test.h"
#include "testhlp.h"

/* Optimization breaks some of the signal generator
 * functions. We disable it in these functions. */
#if defined(__clang__)
#define NO_OPTIMIZE __attribute__((optnone))
#elif defined(__GNUC__)
#define NO_OPTIMIZE __attribute__((optimize(0)))
#endif

static void
signal_test_pre(int signum)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    picotm_libc_acquire_proc_signal(signum, NULL, &error);
    if (picotm_error_is_set(&error)) {
        tap_error("Failed to acquire signal %d.", signum);
        abort_safe_block();
    }
}

static void
signal_test_post(int signum)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    picotm_libc_release_proc_signal(signum, &error);
    if (picotm_error_is_set(&error)) {
        tap_error("Failed to release signal %d.", signum);
        abort_safe_block();
    }
}

static void
signal_test_func(int signum, bool is_recoverable, void (*gen_signal)(int))
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    picotm_libc_add_signal(signum, is_recoverable, &error);
    if (picotm_error_is_set(&error)) {
        abort_safe_block();
    }

    picotm_begin
        // Do nothing; tests plain begin() and finish() of signal module.
    picotm_commit
        abort_transaction_on_error(__func__);
    picotm_end

    /* Trigger the signal multiple times to test repeating calls to
     * signal-handler module. */

    for (unsigned long i = 0ul; i < 5ul; ++i) {

        bool error_detected = false;

        picotm_begin

            gen_signal(signum);

        picotm_commit

            switch (picotm_error_status()) {
            case PICOTM_SIGINFO_T: {
                const siginfo_t* info = picotm_error_as_siginfo_t();
                if (info->si_signo == signum) {
                    error_detected = true;
                }
                break;
            }
            default:
                abort_transaction_on_error(__func__);
                break;
            }

        picotm_end

        if (!error_detected) {
            tap_error("%s, No error detected.", __func__);
            abort_safe_block();
        }
    }
}

static void
gen_signal_by_signum(int signum)
{
    pthread_kill(pthread_self(), signum);
}

/*
 * Test SIGFPE
 */

NO_OPTIMIZE
static void
gen_SIGFPE(int signum)
{
    signum /= (signum < 0); /* triggers SIGFPE, sans compiler warnings */
}

static void
signal_test_SIGFPE_pre(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound)
{
    signal_test_pre(SIGFPE);
}

static void
signal_test_SIGFPE_post(unsigned long nthreads, enum loop_mode loop,
                        enum boundary_type btype, unsigned long long bound)
{
    signal_test_post(SIGFPE);
}

static void
signal_test_SIGFPE(unsigned int tid)
{
    /* UNSAFE: signals should usually *not* be recoverable. */
    signal_test_func(SIGFPE, true, gen_SIGFPE);
}

/*
 * Test SIGUSR1
 */

static void
signal_test_SIGUSR1_pre(unsigned long nthreads, enum loop_mode loop,
                        enum boundary_type btype, unsigned long long bound)
{
    signal_test_pre(SIGUSR1);
}

static void
signal_test_SIGUSR1_post(unsigned long nthreads, enum loop_mode loop,
                         enum boundary_type btype, unsigned long long bound)
{
    signal_test_post(SIGUSR1);
}

static void
signal_test_SIGUSR1(unsigned int tid)
{
    /* UNSAFE: signals should usually *not* be recoverable. */
    signal_test_func(SIGUSR1, true, gen_signal_by_signum);
}

/*
 * Test SIGSEGV
 */

static void
signal_test_SIGSEGV_pre(unsigned long nthreads, enum loop_mode loop,
                        enum boundary_type btype, unsigned long long bound)
{
    signal_test_pre(SIGSEGV);
}

static void
signal_test_SIGSEGV_post(unsigned long nthreads, enum loop_mode loop,
                         enum boundary_type btype, unsigned long long bound)
{
    signal_test_post(SIGSEGV);
}

NO_OPTIMIZE
static void
gen_SIGSEGV(int signum)
{
    long* null_ptr = NULL;
    *null_ptr = 1; /* triggers SIGSEGV */
}

static void
signal_test_SIGSEGV(unsigned int tid)
{
    /* UNSAFE: signals should usually *not* be recoverable. */
    signal_test_func(SIGSEGV, true, gen_SIGSEGV);
}

static const struct test_func signal_test[] = {
    {"Detect/handle SIGFPE",  signal_test_SIGFPE,  signal_test_SIGFPE_pre,  signal_test_SIGFPE_post},
    {"Detect/handle SIGUSR1", signal_test_SIGUSR1, signal_test_SIGUSR1_pre, signal_test_SIGUSR1_post},
    {"Detect/handle SIGSEGV", signal_test_SIGSEGV, signal_test_SIGSEGV_pre, signal_test_SIGSEGV_post},
};

/*
 * Entry point
 */

#include "opts.h"
#include "pubapi.h"

int
main(int argc, char* argv[])
{
    return pubapi_main(argc, argv, PARSE_OPTS_STRING(),
                       signal_test, arraylen(signal_test));
}
