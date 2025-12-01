/*
 * picotm - A system-level transaction manager
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "sigstate.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include <errno.h>

struct sigstate_entry {
    /* Previous state */
    struct sigaction old_action;

    /* Signal-handler function while in non-tx code. */
    void (*nontx_sigaction)(int, siginfo_t*, void*);
    /* Signal handler has been acquired by module. */
    volatile sig_atomic_t acquired;

    /* Protects against concurrent access. */
    struct picotm_spinlock lock;
};

static void
sigstate_entry_acquire(struct sigstate_entry* self, int signum,
                       void (*signal_handler)(int, siginfo_t*, void*),
                       void (*nontx_sigaction)(int, siginfo_t*, void*),
                       struct picotm_error* error)
{
    struct sigaction act = {
        .sa_sigaction = signal_handler,
        .sa_flags = SA_SIGINFO,
    };
    int res = sigemptyset(&act.sa_mask);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }

    picotm_spinlock_lock(&self->lock);

    if (self->acquired) {
        picotm_spinlock_unlock(&self->lock);
        return;
    }

    /* We set-up the fields *before* acquiring the signal. The
     * call to sigaction() acts as 'atomic' state change. */
    self->nontx_sigaction = nontx_sigaction;
    self->acquired = true;

    /* We unlock *between* setting the fields and setting the handler. The
     * handler can lock as soon as sigaction() is done. */
    picotm_spinlock_unlock(&self->lock);

    atomic_signal_fence(memory_order_release);

    res = sigaction(signum, &act, &self->old_action);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        goto err_sigaction;
    }

    return;

err_sigaction:
    /* No other thread could have modified the fields; simply
     * clear them on errors. */
    picotm_spinlock_lock(&self->lock);
    self->nontx_sigaction = NULL;
    self->acquired = false;
    picotm_spinlock_unlock(&self->lock);
}

static void
sigstate_entry_release(struct sigstate_entry* self, int signum,
                       struct picotm_error* error)
{
    if (!self->acquired) {
        return;
    }

    /* First, we restore old signal-handler state. After sigaction()
     * returned, no module-internal signal handler can run on this thread. */
    int res = sigaction(signum, &self->old_action, NULL);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }

    atomic_signal_fence(memory_order_release);

    /* If other threads still run the module-internal signal handler, we
     * protect against them by acquiring this lock. */
    picotm_spinlock_lock(&self->lock);

    if (!self->acquired) {
        goto out;
    }
    self->acquired = false;
    self->nontx_sigaction = NULL;

out:
    picotm_spinlock_unlock(&self->lock);

    return;
}

static void
(*sigstate_entry_get_nontx_sigaction(struct sigstate_entry* self))
    (int, siginfo_t*, void*)
{
    if (!self->acquired) {
        return NULL;
    }

    /* The construction of the acquire and release functions guarantees
     * that we never hold this lock on a thread whne it enters the
     * signal handler. It is safe to acquire this lock here. */
    picotm_spinlock_lock(&self->lock);

    void (*nontx_sigaction)(int, siginfo_t*, void*) = NULL;
    if (!self->acquired) {
        goto out;
    }
    nontx_sigaction = self->nontx_sigaction;

out:
    picotm_spinlock_unlock(&self->lock);

    return nontx_sigaction;
}

struct sigstate {
    struct sigstate_entry entries[NSIG];
    struct picotm_spinlock lock;
};

#define SIGSTATE_INITIALIZER                    \
    {                                           \
        .lock = PICOTM_SPINLOCK_INITIALIZER,    \
    }

static struct sigstate s_sigstate = SIGSTATE_INITIALIZER;

static void
(*get_nontx_sigaction(int signum))(int, siginfo_t*, void*)
{
    return sigstate_entry_get_nontx_sigaction(s_sigstate.entries + signum);
}

static void
signal_handler(int signum, siginfo_t* info, void* ucontext)
{
    tx_signal_handler(signum, info, ucontext);

    /* The transaction is not running or not ready to recover from
     * the signal. We run the non-transactional signal handler if we
     * ended up here. */

    void (*nontx_sigaction)(int, siginfo_t*, void*) =
        get_nontx_sigaction(signum);
    if (!nontx_sigaction) {
        return;
    }
    nontx_sigaction(signum, info, ucontext);
}

void
sigstate_acquire_proc_signal(int signum,
                             void (*nontx_sigaction)(int, siginfo_t*, void*),
                             struct picotm_error* error)
{
    if (signum < 0) {
        picotm_error_set_errno(error, EINVAL);
        return;
    } else if ((size_t)signum >= picotm_arraylen(s_sigstate.entries)) {
        picotm_error_set_errno(error, EINVAL);
        return;
    }

    /* This lock protect against concurrent threads that try to release the
     * signal. It does *not* protect against a concurrent signal handler. */
    picotm_spinlock_lock(&s_sigstate.lock);

    sigstate_entry_acquire(s_sigstate.entries + signum, signum,
                           signal_handler, nontx_sigaction,
                           error);

    picotm_spinlock_unlock(&s_sigstate.lock);
}

void
sigstate_release_proc_signal(int signum, struct picotm_error* error)
{
    if (signum < 0) {
        return;
    } else if ((size_t)signum >= picotm_arraylen(s_sigstate.entries)) {
        return;
    }

    /* This lock protect against concurrent threads that try to acquire the
     * signal. It does *not* protect against a concurrent signal handler. */
    picotm_spinlock_lock(&s_sigstate.lock);

    sigstate_entry_release(s_sigstate.entries + signum, signum, error);

    picotm_spinlock_unlock(&s_sigstate.lock);
}
