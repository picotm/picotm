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

#include "picotm_lock_manager.h"
#include <assert.h>
#include <search.h>
#include <string.h>
#include "picotm_lock_owner.h"
#include "picotm_os_timespec.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-module.h"
#include "table.h"

void
picotm_lock_manager_init(struct picotm_lock_manager* self,
                         struct picotm_error* error)
{
    assert(self);

    picotm_os_rwlock_init(&self->lo_rwlock, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    self->lo = NULL;
    self->nlos = 0;

    picotm_os_rwlock_init(&self->exclusive_lo_lock, error);
    if (picotm_error_is_set(error)) {
        goto err_picotm_os_rwlock_init;
    }

    self->exclusive_lo = NULL;

    return;

err_picotm_os_rwlock_init:
    picotm_os_rwlock_uninit(&self->lo_rwlock);
}

void
picotm_lock_manager_uninit(struct picotm_lock_manager* self)
{
    assert(self);

    tabfree(self->lo);

    picotm_os_rwlock_uninit(&self->exclusive_lo_lock);
    picotm_os_rwlock_uninit(&self->lo_rwlock);
}

/*
 * Requires writer lock on lock_manager::lo_rwlock
 */
static void
grow_lo_array(struct picotm_lock_manager* self, struct picotm_error* error)
{
    assert(self);

    /* Allocate an additional first element if array is still
     * empty. Index 0 is a magic value and never handed out to
     * lock owners. */
    size_t new_nlos = !(self->nlos) + self->nlos + 1;

    struct picotm_lock_owner** new_lo = tabresize(self->lo, self->nlos,
                                                  new_nlos, sizeof(self->lo),
                                                  error);
    if (picotm_error_is_set(error)) {
        return;
    }

    struct picotm_lock_owner** beg = picotm_arrayat(new_lo, self->nlos);
    struct picotm_lock_owner* const * end = picotm_arrayat(new_lo, new_nlos);

    while (beg < end) {
        *beg = NULL;
        ++beg;
    }

    self->lo = new_lo;
    self->nlos = new_nlos;
}

static int
ptrcmp(const void* key, const void* instance)
{
    const void* lhs = *((const void* const *)key);
    const void* rhs = *((const void* const *)instance);

    return (lhs > rhs) - (lhs < rhs);
}

/*
 * Requires writer lock on lock_manager::lo_rwlock
 *
 * The find_lo() function always skips the first element of
 * |self->lo|. The index of 0 is a magic value, so we don't
 * hand it out to lock owners.
 */
static struct picotm_lock_owner**
find_lo(struct picotm_lock_manager* self, struct picotm_lock_owner* lo,
        struct picotm_error* error)
{
    static const struct picotm_lock_owner* key = NULL;
    size_t nlos = self->nlos - !!(self->nlos);

    struct picotm_lock_owner** pos = lfind(&key, self->lo + (!!nlos), &nlos,
                                           sizeof(*self->lo), ptrcmp);
    if (pos) {
        return pos;
    }

    nlos = self->nlos;

    grow_lo_array(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    pos = self->lo + (!nlos) + nlos;
    assert(pos != self->lo);
    assert(!(*pos));

    return pos;
}

void
picotm_lock_manager_register_owner(struct picotm_lock_manager* self,
                                   struct picotm_lock_owner* lo,
                                   struct picotm_error* error)
{
    assert(self);
    assert(lo);

    picotm_os_rwlock_wrlock(&self->lo_rwlock, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    struct picotm_lock_owner** pos = find_lo(self, NULL, error);
    if (picotm_error_is_set(error)) {
        goto err_find_lo;
    }

    assert(!(*pos));
    *pos = lo;
    picotm_lock_owner_set_index(lo, pos - self->lo);

    picotm_os_rwlock_unlock(&self->lo_rwlock);

    return;

err_find_lo:
    picotm_os_rwlock_unlock(&self->lo_rwlock);
}

void
picotm_lock_manager_unregister_owner(struct picotm_lock_manager* self,
                                     struct picotm_lock_owner* lo)
{
    assert(self);

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        picotm_os_rwlock_wrlock(&self->lo_rwlock, &error);
        if (picotm_error_is_set(&error)) {
            picotm_error_mark_as_non_recoverable(&error);;
            picotm_recover_from_error(&error);
            continue;
        }
        break;
    } while (true);

    self->lo[picotm_lock_owner_get_index(lo)] = NULL;

    picotm_os_rwlock_unlock(&self->lo_rwlock);
}

/*
 * Irrevocability
 */

void
picotm_lock_manager_make_irrevocable(struct picotm_lock_manager* self,
                                     struct picotm_lock_owner* exclusive_lo,
                                     struct picotm_error* error)
{
    assert(self);
    assert(exclusive_lo);

    picotm_os_rwlock_wrlock(&self->exclusive_lo_lock, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    self->exclusive_lo = exclusive_lo;
}

void
picotm_lock_manager_wait_irrevocable(struct picotm_lock_manager* self,
                                     struct picotm_error* error)
{
    assert(self);

    picotm_os_rwlock_rdlock(&self->exclusive_lo_lock, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
picotm_lock_manager_release_irrevocability(struct picotm_lock_manager* self)
{
    assert(self);

    self->exclusive_lo = NULL;
    picotm_os_rwlock_unlock(&self->exclusive_lo_lock);
}

/*
 * Listing Handling
 */

static void
lock_and_prepend_waiter(struct picotm_lock_manager* self,
                        struct picotm_lock_owner* waiter,
                        const struct picotm_lock_slist_funcs* slist_funcs,
                        void* slist, struct picotm_error* error)
{
    picotm_os_rwlock_rdlock(&self->lo_rwlock, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    picotm_lock_owner_lock(waiter, error);
    if (picotm_error_is_set(error)) {
        goto err_picotm_lock_owner_lock;
    }

    unsigned long index = picotm_lock_owner_get_index(waiter);

    unsigned long first_index = slist_funcs->get_first_index(slist);
    struct picotm_lock_owner* first_waiter = NULL;

    do {

        /* We retrieve the instance of the current first entry in the
         * waiter list. */
        assert(first_index < self->nlos);
        first_waiter = self->lo[first_index];

        /* At this point we have the index and instance of the first waiter
         * in the list. The list and the first waiter itself is not locked,
         * so a new first waiter can appear any time. We handle this case
         * below in a separate test.
         */

        /* update waiter to refer to current first element. */
        picotm_lock_owner_set_next(waiter, first_index);
        waiter->next = first_waiter;

        unsigned long old_first_index =
            slist_funcs->cmpxchg_first_index(slist, first_index, index);

        /* The first list element has not changed if the list's
         * first index is still the index we retrieved before. A
         * new element might have been prepended meanwhile to the
         * list. In this case we drop our first element and start
         * anew.
         */

        if (old_first_index != first_index) {
            /* The first list element changed, so we do the look-up
             * again. */
            first_index = old_first_index;
            first_waiter = NULL;
            continue;
        }
        break;
    } while (true);

    picotm_os_rwlock_unlock(&self->lo_rwlock);

    return;

err_picotm_lock_owner_lock:
    picotm_os_rwlock_unlock(&self->lo_rwlock);
}

static struct picotm_lock_owner*
get_first_waiter(struct picotm_lock_manager* self,
                 const struct picotm_lock_slist_funcs* slist_funcs,
                 void* slist, struct picotm_error* error)
{
    struct picotm_lock_owner* first_waiter = NULL;

    unsigned long first_index = slist_funcs->get_first_index(slist);

    /* We retrieve the index and locked instance of the current
     * first entry in the waiter list. The waiter can disappear
     * between looking up the index and the locked instance. In
     * this case, we start again with an updated index.
     */

    while (first_index && !first_waiter) {

        assert(first_index < self->nlos);
        first_waiter = self->lo[first_index];

        picotm_lock_owner_lock(first_waiter, error);
        if (picotm_error_is_set(error)) {
            return NULL;
        }

        first_index = slist_funcs->get_first_index(slist);

        if (!first_waiter) {
            /* The first waiter disappeared after retrieving the
             * index. */
            continue;
        } else if (picotm_lock_owner_get_index(first_waiter) != first_index) {
            /* The first list element changed, so we do the look-up
             * again. */
            picotm_lock_owner_unlock(first_waiter);
            first_waiter = NULL;
            continue;
        }
    }

    return first_waiter;
}

static struct picotm_lock_owner*
find_prec_waiter(struct picotm_lock_manager* self,
                 struct picotm_lock_owner* waiter,
                 struct picotm_lock_owner* first_waiter,
                 const struct picotm_lock_slist_funcs* slist_funcs,
                 void* slist, struct picotm_error* error)
{
    assert(first_waiter || !waiter);

    if (first_waiter == waiter) {
        return NULL; /* no previous list element */
    }

    struct picotm_lock_owner* prec_waiter = NULL;

    while (first_waiter->next) {

        struct picotm_lock_owner* next_waiter = first_waiter->next;

        if (next_waiter == waiter) {
            prec_waiter = first_waiter;
            goto out;
        }
        picotm_lock_owner_lock(next_waiter, error);
        if (picotm_error_is_set(error)) {
            picotm_lock_owner_unlock(first_waiter);
            goto out;
        }
        picotm_lock_owner_unlock(first_waiter);
        first_waiter = next_waiter;
    }

    /* We must have stopped before reaching 'waiter'. */
    assert(false);

out:
    assert(!prec_waiter || (prec_waiter->next == waiter));

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        picotm_lock_owner_lock(prec_waiter->next, &error);
        if (picotm_error_is_set(&error)) {
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
            continue;
        }
        break;
    } while (true);

    return prec_waiter;
}

/* Removes a waiter from a lock's waiting list. The 'waiter' and
 * 'prec_waiter' have to be locked. The 'prec_waiter' is optional.
 *
 * The function might change the 'prec_waiter' and acquire a lock
 * on the new instance. The locked 'prec_waiter' is returned and
 * it's the callers responsibility to unlock.
 */
static struct picotm_lock_owner*
remove_waiter(struct picotm_lock_manager* self,
              struct picotm_lock_owner* waiter,
              struct picotm_lock_owner* prec_waiter,
              const struct picotm_lock_slist_funcs* slist_funcs, void* slist)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        picotm_os_rwlock_rdlock(&self->lo_rwlock, &error);
        if (picotm_error_is_set(&error)) {
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
            continue;
        }
        break;
    } while (true);

    do {
        if (!prec_waiter) {

            /* No (known) preceding waiter in the list. We try to remove
             * the waiter directly from the lock's list header.
             */

            unsigned long index = picotm_lock_owner_get_index(waiter);
            unsigned long next_index = picotm_lock_owner_get_next(waiter);

            unsigned long old_first_index =
                slist_funcs->cmpxchg_first_index(slist, index, next_index);

            if (old_first_index == index) {
                unsigned long current = slist_funcs->get_first_index(slist);
                goto out; /* removed waiter from lock; early out */
            }

        } else if (prec_waiter->next == waiter) {

            /* We have a preceding waiter that is still up-to-date.
             */

            unsigned long next_index = picotm_lock_owner_get_next(waiter);
            picotm_lock_owner_set_next(prec_waiter, next_index);
            prec_waiter->next = waiter->next;

            goto out; /* removed waiter from list; early out */

        } else {

            /* Unlisting fails if the preceding waiter changed meanwhile. In
             * this case, we unlock the current preceding waiter and search for
             * the current one.
             */

            picotm_lock_owner_unlock(prec_waiter);
        }

        /* To preserve the waiter list's locking order, we have to
         * unlock the waiter *before* calling get_first_waiter(). The
         * lock will be re-acquired by find_prec_waiter(). */
        picotm_lock_owner_unlock(waiter);

        struct picotm_lock_owner* first_waiter = NULL;

        do {
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            first_waiter = get_first_waiter(self, slist_funcs, slist,
                                            &error);
            if (picotm_error_is_set(&error)) {
                picotm_error_mark_as_non_recoverable(&error);
                picotm_recover_from_error(&error);
                continue;
            }
            break;
        } while (true);

        assert(first_waiter);

        do {
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            prec_waiter = find_prec_waiter(self, waiter, first_waiter,
                                           slist_funcs, slist, &error);
            if (picotm_error_is_set(&error)) {
                picotm_error_mark_as_non_recoverable(&error);
                picotm_recover_from_error(&error);
                continue;
            }
            break;
        } while (true);

    } while (true);

out:
    picotm_os_rwlock_unlock(&self->lo_rwlock);

    return prec_waiter;
}

/*
 * Waiting
 */

/* waits for another duration of the waiter transaction */
static void
compute_timeout(struct picotm_lock_owner* waiter, struct timespec* timeout,
                struct picotm_error* error)
{
    picotm_os_get_timespec(timeout, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    struct timespec timediff;
    timediff.tv_sec = 0;
    timediff.tv_nsec = 1000;
    picotm_os_add_timespec(timeout, &timediff);
}

bool
picotm_lock_manager_wait(struct picotm_lock_manager* self,
                         struct picotm_lock_owner* waiter, bool wr,
                         const struct picotm_lock_slist_funcs* slist_funcs,
                         void* slist, struct picotm_error* error)
{
    /* On success, we will have acquired a lock on 'waiter'. */
    lock_and_prepend_waiter(self, waiter, slist_funcs, slist, error);
    if (picotm_error_is_set(error)) {
        return false;
    }

    waiter->flags |= wr ? LOCK_OWNER_WR : LOCK_OWNER_RD;

    struct timespec timeout;
    compute_timeout(waiter, &timeout, error);
    if (picotm_error_is_set(error)) {
        goto err_compute_timeout;
    }

    bool woken_up = picotm_lock_owner_wait_until(waiter, &timeout, error);
    if (picotm_error_is_set(error)) {
        goto err_picotm_lock_owner_locked_wait;
    }

    struct picotm_lock_owner* prec_waiter = remove_waiter(self, waiter, NULL,
                                                          slist_funcs, slist);
    if (prec_waiter) {
        picotm_lock_owner_unlock(prec_waiter);
    }

    waiter->flags &= (wr ? ~LOCK_OWNER_WR : ~LOCK_OWNER_RD);

    picotm_lock_owner_unlock(waiter);

    return woken_up;

err_picotm_lock_owner_locked_wait:
    prec_waiter = remove_waiter(self, waiter, NULL, slist_funcs, slist);
    if (prec_waiter) {
        picotm_lock_owner_unlock(prec_waiter);
    }
err_compute_timeout:
    waiter->flags &= (wr ? ~LOCK_OWNER_WR : ~LOCK_OWNER_RD);
    picotm_lock_owner_unlock(waiter);
    return false;
}

/*
 * Wake up
 */

/**
 * This is the function that decides about which lock owner to schedule
 * next for a lock. Run with different strategies if necessary.
 *
 * When entering this function, 'first_waiter' has to be locked
 * already. Except for 'first_waiter' and the returned lock owner,
 * all lock owners are expected to be *unlocked* after this
 * function returned.
 */
static struct picotm_lock_owner*
pick_waiter(struct picotm_lock_owner* first_waiter,
            int (*compare_waiters)(const struct picotm_lock_owner*,
                                   const struct picotm_lock_owner*),
            struct picotm_error* error)
{
    /* pick longest waiting */

    struct picotm_lock_owner* picked_waiter = NULL;

    struct picotm_lock_owner* waiter = first_waiter;

    while (waiter) {

        if (waiter->flags & LOCK_OWNER_WT) {
            if (!picked_waiter) {
                picked_waiter = waiter;

            } else {
                int cmp = compare_waiters(picked_waiter, waiter);
                if (cmp > 0) {
                    if (picked_waiter != first_waiter) {
                        picotm_lock_owner_unlock(picked_waiter);
                    }
                    picked_waiter = waiter;
                }
            }
        }

        struct picotm_lock_owner* next_waiter = waiter->next;

        if (next_waiter) {

            picotm_lock_owner_lock(next_waiter, error);
            if (picotm_error_is_set(error)) {
                if (waiter != first_waiter) {
                    picotm_lock_owner_unlock(waiter);
                }
                if (picked_waiter
                        && (picked_waiter != first_waiter)
                        && (picked_waiter != waiter)) {
                    picotm_lock_owner_unlock(picked_waiter);
                }
                return NULL;
            }
        }

        if ((waiter != picked_waiter) && (waiter != first_waiter)) {
            picotm_lock_owner_unlock(waiter);
        }

        waiter = next_waiter;
    }

    return picked_waiter;
}

static int
compare_longest_waiting(const struct picotm_lock_owner* old_waiter,
                        const struct picotm_lock_owner* new_waiter)
{
    assert(old_waiter);
    assert(new_waiter);

    return 1;
}

static int
compare_longest_running(const struct picotm_lock_owner* old_waiter,
                        const struct picotm_lock_owner* new_waiter)
{
    assert(old_waiter);
    assert(new_waiter);

    return picotm_os_timespec_compare(picotm_lock_owner_get_timestamp(old_waiter),
                                      picotm_lock_owner_get_timestamp(new_waiter));
}

void
picotm_lock_manager_wake_up(struct picotm_lock_manager* self,
                            bool concurrent_readers_supported,
                            const struct picotm_lock_slist_funcs* slist_funcs,
                            void* slist, struct picotm_error* error)
{
    static int (* const compare_waiters[])(const struct picotm_lock_owner*,
                                           const struct picotm_lock_owner*) = {
        compare_longest_waiting,
        compare_longest_running
    };

    assert(self);

    picotm_os_rwlock_rdlock(&self->lo_rwlock, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    struct picotm_lock_owner* first_waiter =
        get_first_waiter(self, slist_funcs, slist, error);
    if (picotm_error_is_set(error)) {
        goto err_get_first_waiter;
    }

    if (!first_waiter) {
        /* That's not really an error; maybe the lock's list
         * has been emptied by other threads. */
        goto out;
    }

    struct picotm_lock_owner* picked_waiter = pick_waiter(first_waiter,
                                                          compare_waiters[0],
                                                          error);
    if (picotm_error_is_set(error)) {
        goto err_pick_next;
    }

    if (!picked_waiter) {
        /* Again, that's not really an error. */
        picotm_lock_owner_unlock(first_waiter);
        goto out;
    }

    assert(picked_waiter->flags & (LOCK_OWNER_RD | LOCK_OWNER_WR));

    /* Wake up selected waiter */

    picotm_lock_owner_wake_up(picked_waiter, error);
    if (picotm_error_is_set(error)) {
        goto err_picotm_lock_owner_wake_up;
    }

    bool wake_up_all_readers =
        concurrent_readers_supported
            && (picked_waiter->flags & LOCK_OWNER_RD);

    if (picked_waiter != first_waiter) {
        picotm_lock_owner_unlock(picked_waiter);
    }

    if (wake_up_all_readers) {

        /* Wake up all readers if a reader was selected. */

        struct picotm_lock_owner* waiter = first_waiter;
        struct picotm_lock_owner* prec_waiter = NULL;

        while (waiter) {

            /* During this loop we always hold a lock on the current and
             * the previous value of 'waiter'. The initial value of 'waiter'
             * has already been locked when we enter the loop. Further
             * locking is ordered by the item's list position. Deadlocks
             * with concurrent transaction are therefore impossible.
             */

            if ((waiter->flags & LOCK_OWNER_RD)
                    && (waiter->flags & LOCK_OWNER_WT)) {
                picotm_lock_owner_wake_up(waiter, error);
                if (picotm_error_is_set(error)) {
                    if (prec_waiter) {
                        picotm_lock_owner_unlock(prec_waiter);
                    }
                    picotm_lock_owner_unlock(waiter);
                    picotm_os_rwlock_unlock(&self->lo_rwlock);
                    return;
                }
            }

            prec_waiter = waiter;
            waiter = waiter->next;

            if (waiter) {
                picotm_lock_owner_lock(waiter, error);
                if (picotm_error_is_set(error)) {
                    picotm_lock_owner_unlock(prec_waiter);
                    picotm_os_rwlock_unlock(&self->lo_rwlock);
                    return;
                }
            }
            picotm_lock_owner_unlock(prec_waiter);
        }
    } else {
        picotm_lock_owner_unlock(first_waiter);
    }

out:
    picotm_os_rwlock_unlock(&self->lo_rwlock);

    return;

err_picotm_lock_owner_wake_up:
    picotm_lock_owner_unlock(picked_waiter);
err_pick_next:
    picotm_lock_owner_unlock(first_waiter);
err_get_first_waiter:
    picotm_os_rwlock_unlock(&self->lo_rwlock);
}
