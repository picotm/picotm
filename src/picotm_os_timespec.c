/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
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
 */

#include "picotm_os_timespec.h"
#include <assert.h>
#include <errno.h>
#if defined(__MACH__)
#include <mach/clock.h>
#include <mach/mach.h>
#endif
#include <string.h>
#include "picotm/picotm-error.h"
#include "picotm/picotm-module.h"

static const long MAX_NSEC = 999999999;

void
picotm_os_get_timespec(struct timespec* self,
                       struct picotm_error* error)
{
#if defined(__MACH__)
    clock_serv_t csrv;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &csrv);

    mach_timespec_t ts;
    kern_return_t retval = clock_get_time(csrv, &ts);
    if (retval != KERN_SUCCESS) {
        picotm_error_set_kern_return_t(error, retval);
        return;
    }

    retval = mach_port_deallocate(mach_task_self(), csrv);
    if (retval != KERN_SUCCESS) {
        picotm_error_set_kern_return_t(error, retval);
        picotm_error_mark_as_non_recoverable(error);
        return;
    }

    self->tv_sec = ts.tv_sec;
    self->tv_nsec = ts.tv_nsec;
#else
    int res = clock_gettime(PICOTM_OS_TIMESPEC_CLOCKID, self);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
#endif
}

void
picotm_os_add_timespec(struct timespec* lhs,
                 const struct timespec* rhs)
{
    assert(lhs);
    assert(rhs);

    time_t carry = (MAX_NSEC - rhs->tv_nsec) < lhs->tv_nsec;

    lhs->tv_sec += rhs->tv_sec + carry;
    lhs->tv_nsec += rhs->tv_nsec;
    lhs->tv_nsec -= (MAX_NSEC + 1) * !!carry;

    assert(lhs->tv_nsec >= 0);
    assert(lhs->tv_nsec <= MAX_NSEC);
}

void
picotm_os_sub_timespec(struct timespec* lhs,
                 const struct timespec* rhs)
{
    assert(lhs);
    assert(rhs);

    time_t carry = rhs->tv_nsec > lhs->tv_nsec;

    lhs->tv_sec -= rhs->tv_sec - carry;
    lhs->tv_nsec -= rhs->tv_nsec;
    lhs->tv_nsec += (MAX_NSEC + 1) * !!carry;

    assert(lhs->tv_nsec >= 0);
    assert(lhs->tv_nsec <= MAX_NSEC);
}

static int
ulonglong_cmp(unsigned long long lhs, unsigned long long rhs)
{
    return (lhs > rhs) - (lhs < rhs);
}

int
picotm_os_timespec_compare(const struct timespec* restrict lhs,
                           const struct timespec* restrict rhs)
{
    assert(lhs);
    assert(rhs);

    int cmp = ulonglong_cmp(lhs->tv_sec, rhs->tv_sec);
    if (cmp) {
        return cmp;
    }

    return ulonglong_cmp(lhs->tv_nsec, rhs->tv_nsec);
}

void
picotm_os_nanosleep(const struct timespec* sleep_time,
                    struct picotm_error* error)
{
    struct timespec remaining_time, saved_remaining_time;

    do {
        int res = nanosleep(sleep_time, &remaining_time);
        if (res < 0) {
            if (errno != EINTR) {
                picotm_error_set_errno(error, errno);
                return;
            }
            sleep_time = memcpy(&saved_remaining_time, &remaining_time,
                                sizeof(saved_remaining_time));
            continue;
        }
        break;
    } while (true);
}
