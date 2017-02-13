/* Copyright (C) 2009  Thomas Zimmermann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <tanger-stm-internal-errcode.h>
#include "tanger-stm-ext-actions.h"
#include "types.h"
#include "range.h"
#include "rwlock.h"
#include "counter.h"
#include "pgtree.h"
#include "pgtreess.h"
#include "cmap.h"
#include "cmapss.h"
#include "rwlockmap.h"
#include "rwstatemap.h"
#include "mutex.h"
#include "table.h"
#include "fcntlop.h"
#include "ofdid.h"
#include "ofd.h"

struct ofd ofdtab[MAXNUMFD];
size_t     ofdtab_len = 0;

/* Initializer */

static void ofdtab_init(void) __attribute__ ((constructor));

static int
ofdtab_ofd_init_walk(void *ofd)
{
    return ofd_init(ofd) < 0 ? -1 : 1;
}

static void
ofdtab_init(void)
{
    int res = tabwalk_1(ofdtab,
                        sizeof(ofdtab)/sizeof(ofdtab[0]), sizeof(ofdtab[0]),
                        ofdtab_ofd_init_walk);

    if (res < 0) {
        abort();
    }
}

/* End of initializer */

/* Destructor */

static void ofdtab_uninit(void) __attribute__ ((destructor));

static int
ofdtab_ofd_uninit_walk(void *ofd)
{
    ofd_uninit(ofd);
    return 0;
}

static void
ofdtab_uninit(void)
{
    int res = tabwalk_1(ofdtab,
                        sizeof(ofdtab)/sizeof(ofdtab[0]), sizeof(ofdtab[0]),
                        ofdtab_ofd_uninit_walk);

    if (res < 0) {
        abort();
    }
}

/* End of destructor */

#include "ofdtab.h"

static pthread_mutex_t ofdtab_mutex = PTHREAD_MUTEX_INITIALIZER;

void
ofdtab_lock()
{
    mutex_lock(&ofdtab_mutex);
}

void
ofdtab_unlock()
{
    mutex_unlock(&ofdtab_mutex);
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
ofdtab_search_by_id(const struct ofdid *id, size_t len)
{
    size_t max_len;
    long i;

    max_len = lmin(ofdtab_len, len);

    i = ofdtab_find_by_id(id, max_len);

    if (i == max_len) {
        /* Get an empty entry */
        struct ofdid empty;
        ofdid_clear(&empty);

        i = ofdtab_find_by_id(&empty, len);

        if (i == len) {
            i = ERR_CONFLICT; /* Abort if not enough ids available*/
        }
    }

    return i;
}

static long
ofdtab_search(int fildes)
{
    struct ofdid id;

    int err = ofdid_init_from_fildes(&id, fildes);

    if (err) {
        return err;
    }

    long i = ofdtab_search_by_id(&id, sizeof(ofdtab)/sizeof(ofdtab[0]));

    /* Update maximum index */
    ofdtab_len = lmax(i+1, ofdtab_len);

    return i;
}

long
ofdtab_ref_ofd(int fildes, int flags)
{
    int ofd = ofdtab_search(fildes);

    if (ofd < 0) {
        return ofd;
    }

    int err = ofd_ref(ofdtab+ofd, fildes, flags);

    if (err < 0) {
        return err;
    }

    return ofd;
}

size_t
ofdtab_index(struct ofd *ofd)
{
    return ofd-ofdtab;
}

