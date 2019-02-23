/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018-2019  Thomas Zimmermann <contact@tzimmermann.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "picotm/picotm.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-libc.h"
#include <pthread.h>
#include "ptr.h"
#include "safeblk.h"
#include "sysenv.h"
#include "taputils.h"
#include "test.h"
#include "testhlp.h"

/* Windows reports program errors, such as segmentation faults, as
 * exception. Cygwin translates each exception to its corresponding
 * Unix signal, which it then reports to the program. This is broken
 * on 64-bit systems. We therefore skip the tests until the bug has
 * been resolved.
 *
 * Asynchronous signals use a different mechanism, which works on
 * 64-bit implementations as well.
 */

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
signal_test_func(int signum, bool is_recoverable, bool cond,
                 void (*gen_signal)(int))
{
    if (!cond) {
        tap_info("skipping test for signal %d;"
                 " condition failed", signum);
        return;
    }

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

    for (picotm_safe unsigned long i = 0ul; i < 5ul; ++i) {

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
    signal_test_func(SIGFPE, true, !is_cygwin64(), gen_SIGFPE);
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
    signal_test_func(SIGUSR1, true, true, gen_signal_by_signum);
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
    signal_test_func(SIGSEGV, true, !is_cygwin64(), gen_SIGSEGV);
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
