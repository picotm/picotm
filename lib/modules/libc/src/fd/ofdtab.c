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
#include <picotm/picotm-lib-tab.h>
#include <stdlib.h>
#include "range.h"

struct ofd ofdtab[MAXNUMFD];
size_t     ofdtab_len = 0;

/* Initializer */

static void ofdtab_init(void) __attribute__ ((constructor));

static size_t
ofdtab_ofd_init_walk(void* ofd, struct picotm_error* error)
{
    ofd_init(ofd, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static void
ofdtab_init(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(ofdtab, sizeof(ofdtab)/sizeof(ofdtab[0]),
                     sizeof(ofdtab[0]), ofdtab_ofd_init_walk, &error);
    if (picotm_error_is_set(&error)) {
        abort();
    }
}

/* End of initializer */

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

    picotm_tabwalk_1(ofdtab, sizeof(ofdtab)/sizeof(ofdtab[0]),
                     sizeof(ofdtab[0]), ofdtab_ofd_uninit_walk, &error);
    if (picotm_error_is_set(&error)) {
        abort();
    }
}

/* End of destructor */

#include "ofdtab.h"

static pthread_mutex_t ofdtab_mutex = PTHREAD_MUTEX_INITIALIZER;

void
ofdtab_lock()
{
    int err = pthread_mutex_lock(&ofdtab_mutex);
    if (err) {
        abort();
    }
}

void
ofdtab_unlock()
{
    int err = pthread_mutex_unlock(&ofdtab_mutex);
    if (err) {
        abort();
    }
}

static long
ofdtab_find_by_id(const struct ofdid *id, size_t len)
{
    struct ofd *ofd = ofdtab;

    while (ofd < ofdtab+len) {

        ofd_rdlock(ofd);
        const int cmp = ofdidcmp(id, &ofd->id);
        ofd_unlock(ofd);

        if (!cmp) {
            break;
        }

        ++ofd;
    }

    return (long)(ofd-ofdtab);
}

static long
ofdtab_search_by_id(const struct ofdid *id, size_t len,
                    struct picotm_error* error)
{
    size_t max_len;
    long i;

    max_len = lmin(ofdtab_len, len);

    i = ofdtab_find_by_id(id, max_len);

    if (i == (ssize_t)max_len) {
        /* Get an empty entry */
        struct ofdid empty;
        ofdid_clear(&empty);

        i = ofdtab_find_by_id(&empty, len);

        if (i == (ssize_t)len) {
            /* Abort if not enough ids available */
            picotm_error_set_conflicting(error, NULL);
            return -1;
        }
    }

    return i;
}

static struct ofd*
ofdtab_search(int fildes, struct picotm_error* error)
{
    struct ofdid id;
    ofdid_init_from_fildes(&id, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    long i = ofdtab_search_by_id(&id, sizeof(ofdtab)/sizeof(ofdtab[0]), error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    /* Update maximum index */
    ofdtab_len = lmax(i+1, ofdtab_len);

    return ofdtab + i;
}

struct ofd*
ofdtab_ref_fildes(int fildes, bool want_new, bool unlink_file,
                  struct picotm_error* error)
{
    struct ofd* ofd = ofdtab_search(fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    ofd_ref_or_set_up(ofd, fildes, want_new, unlink_file, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return ofd;
}

size_t
ofdtab_index(struct ofd *ofd)
{
    return ofd-ofdtab;
}
