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

#include "rwstatemap.h"
#include <assert.h>
#include <errno.h>
#include <picotm/picotm-error.h>
#include <stdlib.h>
#include <string.h>
#include "range.h"
#include "rwlockmap.h"

/*
 * rwstate_page
 */

enum {
    RWSTATE_WRITTEN = 0x8000000,
    RWSTATE_COUNTER = 0x7ffffff
};

struct rwstatemap_page
{
    struct rwlockmap_page *lockpg;
    unsigned long          state[RWLOCKMAP_PAGE_NENTRIES];
};

static void
rwstatemap_page_init(struct rwstatemap_page *statepg)
{
    assert(statepg);

    statepg->lockpg = NULL;
    memset(statepg->state, 0, sizeof(statepg->state));
}

static void
rwstatemap_page_uninit(struct rwstatemap_page *statepg)
{
    assert(statepg);

    return;
}

static struct rwlockmap_page*
rwstatemap_page_get_global_page(struct rwstatemap_page *statepg,
                                unsigned long long offset,
                                struct rwlockmap *rwlockmap,
                                struct picotm_error* error)
{
    assert(statepg);

    if (!statepg->lockpg) {
        statepg->lockpg = rwlockmap_lookup_page(rwlockmap, offset, error);
        if (picotm_error_is_set(error)) {
            return NULL;
        }
    }

    return statepg->lockpg;
}

static void
rwstatemap_page_unlock_regions(struct rwstatemap_page* statepg,
                               unsigned long long offset,
                               struct rwlockmap* rwlockmap,
                               struct picotm_error* error)
{
    unsigned long long pgoffset;

    assert(statepg);

    struct rwlockmap_page* lockpg = rwstatemap_page_get_global_page(statepg,
                                                                    offset,
                                                                    rwlockmap,
                                                                    error);
    if (picotm_error_is_set(error)) {
        return;
    }

    for (pgoffset = 0; pgoffset < RWLOCKMAP_PAGE_NENTRIES; ++pgoffset) {

        if (!(statepg->state[pgoffset]&RWSTATE_COUNTER)) {
            continue;
        }

        picotm_rwlock_unlock(lockpg->lock + pgoffset);
        statepg->state[pgoffset] = 0;
    }
}

/*
 * rwstate map
 */

void
rwstatemap_init(struct rwstatemap* rwstatemap)
{
    assert(rwstatemap);

    pgtreess_init(&rwstatemap->super);
}

static void
rwstatemap_destroy_page_fn(void *statepg)
{
    rwstatemap_page_uninit(statepg);
    free(statepg);
}

void
rwstatemap_uninit(struct rwstatemap *rwstatemap)
{
    assert(rwstatemap);

    pgtreess_uninit(&rwstatemap->super, rwstatemap_destroy_page_fn);
}

static void*
rwstatemap_create_page_fn(struct picotm_error* error)
{
    struct rwstatemap_page* statepg = malloc(sizeof(*statepg));
    if (!statepg) {
        picotm_error_set_errno(error, errno);
        return NULL;
    }

    rwstatemap_page_init(statepg);

    return statepg;
}

bool
rwstatemap_rdlock(struct rwstatemap* rwstatemap,
                  unsigned long long length, unsigned long long offset,
                  struct rwlockmap* rwlockmap, struct picotm_error* error)
{
    unsigned long long lockedlength, lockedoffset;

    lockedlength = 0;
    lockedoffset = offset;

    assert(rwstatemap);

    bool succ = false;

    while (length) {

        struct rwstatemap_page *statepg = pgtreess_lookup_page(&rwstatemap->super,
                                                               offset,
                                                               rwstatemap_create_page_fn,
                                                               error);
        if (picotm_error_is_set(error)) {
            goto doreturn;
        }

        struct rwlockmap_page* lockpg =
            rwstatemap_page_get_global_page(statepg, offset, rwlockmap, error);

        if (picotm_error_is_set(error)) {
            goto doreturn;
        }

        /* compute next slice */

        unsigned long long pgoffset = offset % RWLOCKMAP_PAGE_NENTRIES;
        unsigned long long diff = llmin(length,
                                        RWLOCKMAP_PAGE_NENTRIES - pgoffset);

        length -= diff;
        offset += diff;

        /* read-lock records */

        while (diff) {

            if ( !(statepg->state[pgoffset]&RWSTATE_COUNTER) ) {

                /* not yet read-locked */

                picotm_rwlock_try_rdlock(lockpg->lock + pgoffset, error);
                if (picotm_error_is_set(error)) {
                    goto doreturn;
                }
            }

            statepg->state[pgoffset] =
                (statepg->state[pgoffset]&RWSTATE_WRITTEN) |
               ((statepg->state[pgoffset]&RWSTATE_COUNTER) + 1);

            ++lockedlength;
            ++pgoffset;
            --diff;
        }
    }

    succ = true;

doreturn:

    if (!succ) {
        /* unlock all locked records if error occured */
        rwstatemap_unlock(rwstatemap, lockedlength, lockedoffset,
                          rwlockmap, error);
    }

    return succ;
}

bool
rwstatemap_wrlock(struct rwstatemap* rwstatemap,
                  unsigned long long length, unsigned long long offset,
                  struct rwlockmap* rwlockmap, struct picotm_error* error)
{
    unsigned long long lockedlength, lockedoffset;

    assert(rwstatemap);

    lockedlength = 0;
    lockedoffset = offset;

    bool succ = false;

    while (length) {

        struct rwstatemap_page* statepg =
            pgtreess_lookup_page(&rwstatemap->super, offset,
                                 rwstatemap_create_page_fn, error);

        if (picotm_error_is_set(error)) {
            goto doreturn;
        }

        struct rwlockmap_page* lockpg =
            rwstatemap_page_get_global_page(statepg, offset, rwlockmap, error);

        if (picotm_error_is_set(error)) {
            goto doreturn;
        }

        /* compute next slice */

        unsigned long long pgoffset = offset % RWLOCKMAP_PAGE_NENTRIES;
        unsigned long long diff = llmin(length,
                                        RWLOCKMAP_PAGE_NENTRIES - pgoffset);

        length -= diff;
        offset += diff;

        /* write-lock records */

        while (diff) {

            if ( !(statepg->state[pgoffset] & RWSTATE_WRITTEN) ) {

                /* not yet write-locked */

                bool upgrade = !!(statepg->state[pgoffset] & RWSTATE_COUNTER);

                picotm_rwlock_try_wrlock(lockpg->lock + pgoffset, upgrade, error);
                if (picotm_error_is_set(error)) {
                    goto doreturn;
                }
            }

            statepg->state[pgoffset] = RWSTATE_WRITTEN
                | ((statepg->state[pgoffset]&RWSTATE_COUNTER) + 1);

            ++lockedlength;
            ++pgoffset;
            --diff;
        }
    }

    succ = true;

doreturn:

    if (!succ) {
        /* unlock all locked records if error occured */
        rwstatemap_unlock(rwstatemap, lockedlength, lockedoffset,
                          rwlockmap, error);
    }

    return succ;
}

void
rwstatemap_unlock(struct rwstatemap *rwstatemap, unsigned long long length,
                                                 unsigned long long offset,
                                                 struct rwlockmap* rwlockmap,
                                                 struct picotm_error* error)
{
    assert(rwstatemap);

    while (length) {

        struct rwstatemap_page* statepg
            = pgtreess_lookup_page(&rwstatemap->super, offset,
                                   rwstatemap_create_page_fn,
                                   error);
        if (picotm_error_is_set(error)) {
            return;
        }

        struct rwlockmap_page* lockpg =
            rwstatemap_page_get_global_page(statepg, offset, rwlockmap, error);
        if (picotm_error_is_set(error)) {
            return;
        }

        /* compute next slice */

        unsigned long long pgoffset = offset % RWLOCKMAP_PAGE_NENTRIES;
        unsigned long long diff = llmin(length,
                                        RWLOCKMAP_PAGE_NENTRIES - pgoffset);

        length -= diff;
        offset += diff;

        /* unlock records */

        while (diff) {

            assert(statepg->state[pgoffset]&RWSTATE_COUNTER);

            statepg->state[pgoffset] =
                (statepg->state[pgoffset]&RWSTATE_WRITTEN) |
               ((statepg->state[pgoffset]&RWSTATE_COUNTER) - 1);

            if ( !(statepg->state[pgoffset]&RWSTATE_COUNTER) ) {
                /* last lock of this transaction */

                picotm_rwlock_unlock(lockpg->lock + pgoffset);
                statepg->state[pgoffset] = 0;
            }

            ++pgoffset;
            --diff;
        }
    }
}

static void
rwstatemap_for_each_page_unlock_regions(void *statepg,
                                        unsigned long long offset,
                                        void *rwlockmap,
                                        struct picotm_error* error)
{
    rwstatemap_page_unlock_regions(statepg, offset, rwlockmap, error);
}

void
rwstatemap_unlock_all(struct rwstatemap* rwstatemap,
                      struct rwlockmap* rwlockmap,
                      struct picotm_error* error)
{
    pgtreess_for_each_page(&rwstatemap->super,
                           rwstatemap_for_each_page_unlock_regions,
                           rwlockmap,
                           error);
}
