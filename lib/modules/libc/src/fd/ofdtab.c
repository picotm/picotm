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

#include "ofdtab.h"
#include <errno.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-tab.h>
#include <pthread.h>
#include <stdlib.h>
#include "ofd.h"
#include "range.h"

static struct ofd       ofdtab[MAXNUMFD];
static size_t           ofdtab_len = 0;
static pthread_rwlock_t ofdtab_rwlock = PTHREAD_RWLOCK_INITIALIZER;

/* Destructor */

static void ofdtab_uninit(void) __attribute__ ((destructor));

static size_t
ofdtab_ofd_uninit_walk(void* ofd, struct picotm_error* error)
{
    ofd_uninit(ofd);
    return 1;
}

static void
ofdtab_uninit(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(ofdtab, ofdtab_len, sizeof(ofdtab[0]),
                     ofdtab_ofd_uninit_walk, &error);
    if (picotm_error_is_set(&error)) {
        abort();
    }

    int err = pthread_rwlock_destroy(&ofdtab_rwlock);
    if (err) {
        abort();
    }
}

/* End of destructor */

static void
rdlock_ofdtab(void)
{
    int err = pthread_rwlock_rdlock(&ofdtab_rwlock);
    if (err) {
        abort();
    }
}

static void
wrlock_ofdtab(void)
{
    int err = pthread_rwlock_wrlock(&ofdtab_rwlock);
    if (err) {
        abort();
    }
}

static void
unlock_ofdtab(void)
{
    int err = pthread_rwlock_unlock(&ofdtab_rwlock);
    if (err) {
        abort();
    }
}

/* requires a writer lock */
static struct ofd*
append_empty_ofd(struct picotm_error* error)
{
    if (ofdtab_len == picotm_arraylen(ofdtab)) {
        /* Return error if not enough ids available */
        picotm_error_set_conflicting(error, NULL);
        return NULL;
    }

    struct ofd* ofd = ofdtab + ofdtab_len;

    ofd_init(ofd, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    ++ofdtab_len;

    return ofd;
}

/* requires reader lock */
static struct ofd*
find_by_id(const struct ofdid* id)
{
    struct ofd *ofd_beg = picotm_arraybeg(ofdtab);
    const struct ofd* ofd_end = picotm_arrayat(ofdtab, ofdtab_len);

    while (ofd_beg < ofd_end) {

        const int cmp = ofd_cmp_and_ref(ofd_beg, id);
        if (!cmp) {
            return ofd_beg;
        }

        ++ofd_beg;
    }

    return NULL;
}

/* requires writer lock */
static struct ofd*
search_by_id(const struct ofdid* id, int fildes, bool want_new,
             bool unlink_file, struct picotm_error* error)
{
    struct ofd* ofd_beg = picotm_arraybeg(ofdtab);
    const struct ofd* ofd_end = picotm_arrayat(ofdtab, ofdtab_len);

    while (ofd_beg < ofd_end) {

        const int cmp = ofd_cmp_and_ref_or_set_up(ofd_beg, id, fildes,
                                                  want_new, unlink_file,
                                                  error);
        if (!cmp) {
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return ofd_beg; /* set-up ofd structure; return */
        }

        ++ofd_beg;
    }

    return NULL;
}

struct ofd*
ofdtab_ref_fildes(int fildes, bool want_new, bool unlink_file,
                  struct picotm_error* error)
{
    struct ofdid id;
    ofdid_init_from_fildes(&id, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct ofd* ofd;

    /* Try to find an existing ofd structure with the given id; iff
     * a new element was not explicitly requested.
     */

    if (!want_new) {
        rdlock_ofdtab();

        ofd = find_by_id(&id);
        if (ofd) {
            goto unlock; /* found ofd for id; return */
        }

        unlock_ofdtab();
    }

    /* Not found or new entry is requested; acquire writer lock to
     * create a new entry in the ofd table. */
    wrlock_ofdtab();

    if (!want_new) {
        /* Re-try find operation; maybe element was added meanwhile. */
        ofd = find_by_id(&id);
        if (ofd) {
            goto unlock; /* found ofd for id; return */
        }
    }

    /* No entry with the id exists; try to set up an existing, but
     * currently unused, ofd structure.
     */

    struct ofdid empty_id;
    ofdid_clear(&empty_id);

    ofd = search_by_id(&empty_id, fildes, want_new, unlink_file, error);
    if (picotm_error_is_set(error)) {
        goto err_search_by_id;
    }

    /* The ofd table is full; create a new entry for the ofd id at the
     * end of the table.
     */

    ofd = append_empty_ofd(error);
    if (picotm_error_is_set(error)) {
        goto err_append_empty_ofd;
    }

    /* To perform the setup, we must have acquired a writer lock at
     * this point. No other transaction may interfere. */
    ofd_ref_or_set_up(ofd, fildes, want_new, unlink_file, error);
    if (picotm_error_is_set(error)) {
        goto err_ofd_ref_or_set_up;
    }

unlock:
    unlock_ofdtab();

    return ofd;

err_ofd_ref_or_set_up:
err_append_empty_ofd:
err_search_by_id:
    unlock_ofdtab();
    return NULL;
}

size_t
ofdtab_index(struct ofd* ofd)
{
    return ofd - ofdtab;
}
