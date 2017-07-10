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

#include "regfiletab.h"
#include <errno.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-tab.h>
#include <pthread.h>
#include <stdlib.h>
#include "range.h"
#include "regfile.h"

static struct regfile   regfiletab[MAXNUMFD];
static size_t           regfiletab_len = 0;
static pthread_rwlock_t regfiletab_rwlock = PTHREAD_RWLOCK_INITIALIZER;

/* Destructor */

static void regfiletab_uninit(void) __attribute__ ((destructor));

static size_t
regfiletab_regfile_uninit_walk(void* regfile, struct picotm_error* error)
{
    regfile_uninit(regfile);
    return 1;
}

static void
regfiletab_uninit(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(regfiletab, regfiletab_len, sizeof(regfiletab[0]),
                     regfiletab_regfile_uninit_walk, &error);
    if (picotm_error_is_set(&error)) {
        abort();
    }

    int err = pthread_rwlock_destroy(&regfiletab_rwlock);
    if (err) {
        abort();
    }
}

/* End of destructor */

static void
rdlock_regfiletab(void)
{
    int err = pthread_rwlock_rdlock(&regfiletab_rwlock);
    if (err) {
        abort();
    }
}

static void
wrlock_regfiletab(void)
{
    int err = pthread_rwlock_wrlock(&regfiletab_rwlock);
    if (err) {
        abort();
    }
}

static void
unlock_regfiletab(void)
{
    int err = pthread_rwlock_unlock(&regfiletab_rwlock);
    if (err) {
        abort();
    }
}

/* requires a writer lock */
static struct regfile*
append_empty_regfile(struct picotm_error* error)
{
    if (regfiletab_len == picotm_arraylen(regfiletab)) {
        /* Return error if not enough ids available */
        picotm_error_set_conflicting(error, NULL);
        return NULL;
    }

    struct regfile* regfile = regfiletab + regfiletab_len;

    regfile_init(regfile, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    ++regfiletab_len;

    return regfile;
}

/* requires reader lock */
static struct regfile*
find_by_id(const struct ofdid* id)
{
    struct regfile *regfile_beg = picotm_arraybeg(regfiletab);
    const struct regfile* regfile_end = picotm_arrayat(regfiletab,
                                                       regfiletab_len);

    while (regfile_beg < regfile_end) {

        const int cmp = regfile_cmp_and_ref(regfile_beg, id);
        if (!cmp) {
            return regfile_beg;
        }

        ++regfile_beg;
    }

    return NULL;
}

/* requires writer lock */
static struct regfile*
search_by_id(const struct ofdid* id, int fildes, struct picotm_error* error)
{
    struct regfile* regfile_beg = picotm_arraybeg(regfiletab);
    const struct regfile* regfile_end = picotm_arrayat(regfiletab,
                                                       regfiletab_len);

    while (regfile_beg < regfile_end) {

        const int cmp = regfile_cmp_and_ref_or_set_up(regfile_beg, id, fildes,
                                                      error);
        if (!cmp) {
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return regfile_beg; /* set-up regfile structure; return */
        }

        ++regfile_beg;
    }

    return NULL;
}

struct regfile*
regfiletab_ref_fildes(int fildes, bool want_new, struct picotm_error* error)
{
    struct ofdid id;
    ofdid_init_from_fildes(&id, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct regfile* regfile;

    /* Try to find an existing regfile structure with the given id; iff
     * a new element was not explicitly requested.
     */

    if (!want_new) {
        rdlock_regfiletab();

        regfile = find_by_id(&id);
        if (regfile) {
            goto unlock; /* found regfile for id; return */
        }

        unlock_regfiletab();
    }

    /* Not found or new entry is requested; acquire writer lock to
     * create a new entry in the regfile table. */
    wrlock_regfiletab();

    if (!want_new) {
        /* Re-try find operation; maybe element was added meanwhile. */
        regfile = find_by_id(&id);
        if (regfile) {
            goto unlock; /* found regfile for id; return */
        }
    }

    /* No entry with the id exists; try to set up an existing, but
     * currently unused, regfile structure.
     */

    struct ofdid empty_id;
    ofdid_clear(&empty_id);

    regfile = search_by_id(&empty_id, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_search_by_id;
    } else if (regfile) {
        goto unlock; /* found regfile for id; return */
    }

    /* The regfile table is full; create a new entry for the regfile id at the
     * end of the table.
     */

    regfile = append_empty_regfile(error);
    if (picotm_error_is_set(error)) {
        goto err_append_empty_regfile;
    }

    /* To perform the setup, we must have acquired a writer lock at
     * this point. No other transaction may interfere. */
    regfile_ref_or_set_up(regfile, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_regfile_ref_or_set_up;
    }

unlock:
    unlock_regfiletab();

    return regfile;

err_regfile_ref_or_set_up:
err_append_empty_regfile:
err_search_by_id:
    unlock_regfiletab();
    return NULL;
}

size_t
regfiletab_index(struct regfile* regfile)
{
    return regfile - regfiletab;
}
