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

#include "fifotab.h"
#include <errno.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-tab.h>
#include <pthread.h>
#include <stdlib.h>
#include "fifo.h"
#include "range.h"

static struct fifo    fifotab[MAXNUMFD];
static size_t           fifotab_len = 0;
static pthread_rwlock_t fifotab_rwlock = PTHREAD_RWLOCK_INITIALIZER;

/* Destructor */

static void fifotab_uninit(void) __attribute__ ((destructor));

static size_t
fifotab_fifo_uninit_walk(void* fifo, struct picotm_error* error)
{
    fifo_uninit(fifo);
    return 1;
}

static void
fifotab_uninit(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(fifotab, fifotab_len, sizeof(fifotab[0]),
                     fifotab_fifo_uninit_walk, &error);
    if (picotm_error_is_set(&error)) {
        abort();
    }

    int err = pthread_rwlock_destroy(&fifotab_rwlock);
    if (err) {
        abort();
    }
}

/* End of destructor */

static void
rdlock_fifotab(void)
{
    int err = pthread_rwlock_rdlock(&fifotab_rwlock);
    if (err) {
        abort();
    }
}

static void
wrlock_fifotab(void)
{
    int err = pthread_rwlock_wrlock(&fifotab_rwlock);
    if (err) {
        abort();
    }
}

static void
unlock_fifotab(void)
{
    int err = pthread_rwlock_unlock(&fifotab_rwlock);
    if (err) {
        abort();
    }
}

/* requires a writer lock */
static struct fifo*
append_empty_fifo(struct picotm_error* error)
{
    if (fifotab_len == picotm_arraylen(fifotab)) {
        /* Return error if not enough ids available */
        picotm_error_set_conflicting(error, NULL);
        return NULL;
    }

    struct fifo* fifo = fifotab + fifotab_len;

    fifo_init(fifo, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    ++fifotab_len;

    return fifo;
}

/* requires reader lock */
static struct fifo*
find_by_id(const struct ofdid* id)
{
    struct fifo *fifo_beg = picotm_arraybeg(fifotab);
    const struct fifo* fifo_end = picotm_arrayat(fifotab, fifotab_len);

    while (fifo_beg < fifo_end) {

        const int cmp = fifo_cmp_and_ref(fifo_beg, id);
        if (!cmp) {
            return fifo_beg;
        }

        ++fifo_beg;
    }

    return NULL;
}

/* requires writer lock */
static struct fifo*
search_by_id(const struct ofdid* id, int fildes, struct picotm_error* error)
{
    struct fifo* fifo_beg = picotm_arraybeg(fifotab);
    const struct fifo* fifo_end = picotm_arrayat(fifotab, fifotab_len);

    while (fifo_beg < fifo_end) {

        const int cmp = fifo_cmp_and_ref_or_set_up(fifo_beg, id, fildes,
                                                     error);
        if (!cmp) {
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return fifo_beg; /* set-up fifo structure; return */
        }

        ++fifo_beg;
    }

    return NULL;
}

struct fifo*
fifotab_ref_fildes(int fildes, bool want_new, struct picotm_error* error)
{
    struct ofdid id;
    ofdid_init_from_fildes(&id, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct fifo* fifo;

    /* Try to find an existing fifo structure with the given id; iff
     * a new element was not explicitly requested.
     */

    if (!want_new) {
        rdlock_fifotab();

        fifo = find_by_id(&id);
        if (fifo) {
            goto unlock; /* found fifo for id; return */
        }

        unlock_fifotab();
    }

    /* Not found or new entry is requested; acquire writer lock to
     * create a new entry in the fifo table. */
    wrlock_fifotab();

    if (!want_new) {
        /* Re-try find operation; maybe element was added meanwhile. */
        fifo = find_by_id(&id);
        if (fifo) {
            goto unlock; /* found fifo for id; return */
        }
    }

    /* No entry with the id exists; try to set up an existing, but
     * currently unused, fifo structure.
     */

    struct ofdid empty_id;
    ofdid_clear(&empty_id);

    fifo = search_by_id(&empty_id, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_search_by_id;
    }

    /* The fifo table is full; create a new entry for the fifo id at the
     * end of the table.
     */

    fifo = append_empty_fifo(error);
    if (picotm_error_is_set(error)) {
        goto err_append_empty_fifo;
    }

    /* To perform the setup, we must have acquired a writer lock at
     * this point. No other transaction may interfere. */
    fifo_ref_or_set_up(fifo, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_fifo_ref_or_set_up;
    }

unlock:
    unlock_fifotab();

    return fifo;

err_fifo_ref_or_set_up:
err_append_empty_fifo:
err_search_by_id:
    unlock_fifotab();
    return NULL;
}

size_t
fifotab_index(struct fifo* fifo)
{
    return fifo - fifotab;
}
