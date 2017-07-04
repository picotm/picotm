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

static struct ofd ofdtab[MAXNUMFD];
static size_t     ofdtab_len = 0;

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
}

/* End of destructor */

static pthread_mutex_t ofdtab_mutex = PTHREAD_MUTEX_INITIALIZER;

static void
ofdtab_lock(void)
{
    int err = pthread_mutex_lock(&ofdtab_mutex);
    if (err) {
        abort();
    }
}

static void
ofdtab_unlock(void)
{
    int err = pthread_mutex_unlock(&ofdtab_mutex);
    if (err) {
        abort();
    }
}

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

static struct ofd*
find_by_id(const struct ofdid* id)
{
    struct ofd *ofd_beg = picotm_arraybeg(ofdtab);
    const struct ofd* ofd_end = picotm_arrayat(ofdtab, ofdtab_len);

    while (ofd_beg < ofd_end) {

        ofd_rdlock(ofd_beg);
        const int cmp = ofdidcmp(id, &ofd_beg->id);
        ofd_unlock(ofd_beg);

        if (!cmp) {
            return ofd_beg;
        }

        ++ofd_beg;
    }

    return NULL;
}

static struct ofd*
search_by_id(const struct ofdid* id, struct picotm_error* error)
{
    struct ofd* ofd = find_by_id(id);
    if (ofd) {
        return ofd; /* found ofd for id; return */
    }

    /* Get an empty entry */
    struct ofdid empty;
    ofdid_clear(&empty);

    ofd = find_by_id(&empty);
    if (ofd) {
        return ofd; /* found empty ofd; return */
    }

    ofd = append_empty_ofd(error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return ofd;
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

    ofdtab_lock();

    struct ofd* ofd = search_by_id(&id, error);
    if (picotm_error_is_set(error)) {
        goto err_search_by_id;
    }

    ofd_ref_or_set_up(ofd, fildes, want_new, unlink_file, error);
    if (picotm_error_is_set(error)) {
        goto err_ofd_ref_or_set_up;
    }

    ofdtab_unlock();

    return ofd;

err_ofd_ref_or_set_up:
err_search_by_id:
    ofdtab_unlock();
    return NULL;
}

size_t
ofdtab_index(struct ofd* ofd)
{
    return ofd - ofdtab;
}
