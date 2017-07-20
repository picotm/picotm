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

#include "dirtab.h"
#include <errno.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-tab.h>
#include <pthread.h>
#include <stdlib.h>
#include "dir.h"
#include "range.h"

static struct dir       dirtab[MAXNUMFD];
static size_t           dirtab_len = 0;
static pthread_rwlock_t dirtab_rwlock = PTHREAD_RWLOCK_INITIALIZER;

/* Destructor */

static void dirtab_uninit(void) __attribute__ ((destructor));

static size_t
dirtab_dir_uninit_walk(void* dir, struct picotm_error* error)
{
    dir_uninit(dir);
    return 1;
}

static void
dirtab_uninit(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(dirtab, dirtab_len, sizeof(dirtab[0]),
                     dirtab_dir_uninit_walk, &error);
    if (picotm_error_is_set(&error)) {
        abort();
    }

    int err = pthread_rwlock_destroy(&dirtab_rwlock);
    if (err) {
        abort();
    }
}

/* End of destructor */

static void
rdlock_dirtab(void)
{
    int err = pthread_rwlock_rdlock(&dirtab_rwlock);
    if (err) {
        abort();
    }
}

static void
wrlock_dirtab(void)
{
    int err = pthread_rwlock_wrlock(&dirtab_rwlock);
    if (err) {
        abort();
    }
}

static void
unlock_dirtab(void)
{
    int err = pthread_rwlock_unlock(&dirtab_rwlock);
    if (err) {
        abort();
    }
}

/* requires a writer lock */
static struct dir*
append_empty_dir(struct picotm_error* error)
{
    if (dirtab_len == picotm_arraylen(dirtab)) {
        /* Return error if not enough ids available */
        picotm_error_set_conflicting(error, NULL);
        return NULL;
    }

    struct dir* dir = dirtab + dirtab_len;

    dir_init(dir, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    ++dirtab_len;

    return dir;
}

/* requires reader lock */
static struct dir*
find_by_id(const struct ofdid* id)
{
    struct dir *dir_beg = picotm_arraybeg(dirtab);
    const struct dir* dir_end = picotm_arrayat(dirtab, dirtab_len);

    while (dir_beg < dir_end) {

        const int cmp = dir_cmp_and_ref(dir_beg, id);
        if (!cmp) {
            return dir_beg;
        }

        ++dir_beg;
    }

    return NULL;
}

/* requires writer lock */
static struct dir*
search_by_id(const struct ofdid* id, int fildes, struct picotm_error* error)
{
    struct dir* dir_beg = picotm_arraybeg(dirtab);
    const struct dir* dir_end = picotm_arrayat(dirtab, dirtab_len);

    while (dir_beg < dir_end) {

        const int cmp = dir_cmp_and_ref_or_set_up(dir_beg, id, fildes,
                                                     error);
        if (!cmp) {
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return dir_beg; /* set-up dir structure; return */
        }

        ++dir_beg;
    }

    return NULL;
}

struct dir*
dirtab_ref_fildes(int fildes, bool want_new, struct picotm_error* error)
{
    struct ofdid id;
    ofdid_init_from_fildes(&id, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct dir* dir;

    /* Try to find an existing dir structure with the given id; iff
     * a new element was not explicitly requested.
     */

    if (!want_new) {
        rdlock_dirtab();

        dir = find_by_id(&id);
        if (dir) {
            goto unlock; /* found dir for id; return */
        }

        unlock_dirtab();
    }

    /* Not found or new entry is requested; acquire writer lock to
     * create a new entry in the dir table. */
    wrlock_dirtab();

    if (!want_new) {
        /* Re-try find operation; maybe element was added meanwhile. */
        dir = find_by_id(&id);
        if (dir) {
            goto unlock; /* found dir for id; return */
        }
    }

    /* No entry with the id exists; try to set up an existing, but
     * currently unused, dir structure.
     */

    struct ofdid empty_id;
    ofdid_clear(&empty_id);

    dir = search_by_id(&empty_id, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_search_by_id;
    } else if (dir) {
        goto unlock; /* found dir for id; return */
    }

    /* The dir table is full; create a new entry for the dir id at the
     * end of the table.
     */

    dir = append_empty_dir(error);
    if (picotm_error_is_set(error)) {
        goto err_append_empty_dir;
    }

    /* To perform the setup, we must have acquired a writer lock at
     * this point. No other transaction may interfere. */
    dir_ref_or_set_up(dir, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_dir_ref_or_set_up;
    }

unlock:
    unlock_dirtab();

    return dir;

err_dir_ref_or_set_up:
err_append_empty_dir:
err_search_by_id:
    unlock_dirtab();
    return NULL;
}

size_t
dirtab_index(struct dir* dir)
{
    return dir - dirtab;
}