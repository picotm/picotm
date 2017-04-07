/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <systx/systx-module.h>
#include <tanger-stm-internal.h>
#include <tanger-stm-internal-errcode.h>
#include <tanger-stm-internal-extact.h>
#include <tanger-stm-ext-actions.h>
#include <unistd.h>
#include "types.h"
#include "range.h"
#include "mutex.h"
#include "rwlock.h"
#include "counter.h"
#include "pgtree.h"
#include "pgtreess.h"
#include "cmap.h"
#include "cmapss.h"
#include "rwlockmap.h"
#include "rwstatemap.h"
#include "connection.h"
#include "region.h"
#include "regiontab.h"
#include "ioop.h"
#include "iooptab.h"
#include "seekop.h"
#include "seekoptab.h"
#include "fcntlop.h"
#include "fcntloptab.h"
#include "ofdid.h"
#include "ofd.h"
#include "ofdtab.h"
#include "ofdtx.h"

static int
ofdtx_2pl_release_locks(struct ofdtx *ofdtx)
{
    struct ofd *ofd;
    struct region *region;
    const struct region *regionend;

    assert(ofdtx);

    ofd = ofdtab+ofdtx->ofd;

    /* release all locks */

    region = ofdtx->modedata.tpl.locktab;
    regionend = region+ofdtx->modedata.tpl.locktablen;

    while (region < regionend) {
        ofd_2pl_unlock_region(ofd,
                              region->offset,
                              region->nbyte,
                              &ofdtx->modedata.tpl.rwstatemap);
        ++region;
    }

    return 0;
}

int
ofdtx_init(struct ofdtx *ofdtx)
{
    int err;

    assert(ofdtx);

    ofdtx->ofd = -1;

    ofdtx->flags = 0;

    ofdtx->wrbuf = NULL;
    ofdtx->wrbuflen = 0;
    ofdtx->wrbufsiz = 0;

    ofdtx->wrtab = NULL;
    ofdtx->wrtablen = 0;
    ofdtx->wrtabsiz = 0;

    ofdtx->rdtab = NULL;
    ofdtx->rdtablen = 0;
    ofdtx->rdtabsiz = 0;

    ofdtx->seektab = NULL;
    ofdtx->seektablen = 0;

    ofdtx->fcntltab = NULL;
    ofdtx->fcntltablen = 0;

    ofdtx->conn = NULL;
    ofdtx->offset = 0;
    ofdtx->size = 0;
    ofdtx->type = TYPE_ANY;
    ofdtx->ccmode = CC_MODE_NOUNDO;

    /* TL */

    ofdtx->modedata.ts.ver = (count_type)-1;

    if ((err = cmapss_init(&ofdtx->modedata.ts.cmapss)) < 0) {
        return err;
    }
    ofdtx->modedata.ts.locktab = NULL;
    ofdtx->modedata.ts.locktablen = 0;
    ofdtx->modedata.ts.locktabsiz = 0;

    /* PLP */

    ofdtx->modedata.tpl.rwstate = RW_NOLOCK;

    if ((err = rwstatemap_init(&ofdtx->modedata.tpl.rwstatemap)) < 0) {
        return err;
    }

    ofdtx->modedata.tpl.locktab = NULL;
    ofdtx->modedata.tpl.locktablen = 0;
    ofdtx->modedata.tpl.locktabsiz = 0;

    return 0;
}

void
ofdtx_uninit(struct ofdtx *ofdtx)
{
    assert(ofdtx);

    free(ofdtx->modedata.tpl.locktab);
    rwstatemap_uninit(&ofdtx->modedata.tpl.rwstatemap);

    free(ofdtx->modedata.ts.locktab);
    cmapss_uninit(&ofdtx->modedata.ts.cmapss);

    fcntloptab_clear(&ofdtx->fcntltab, &ofdtx->fcntltablen);
    seekoptab_clear(&ofdtx->seektab, &ofdtx->seektablen);
    iooptab_clear(&ofdtx->wrtab, &ofdtx->wrtablen);
    iooptab_clear(&ofdtx->rdtab, &ofdtx->rdtablen);
    free(ofdtx->wrbuf);
}

/*
 * Validation
 */

static int
ofdtx_validate_noundo(struct ofdtx *ofdtx)
{
    return 0;
}

static int
ofdtx_validate_ts(struct ofdtx *ofdtx)
{
    assert(ofdtx);

    /* fastpath: no local changes */
    if ( !(ofdtx->flags&(OFDTX_FL_LOCALSTATE|OFDTX_FL_LOCALBUF)) ) {
        return 0;
    }

    /* validate version of file descriptor */
    if (ofdtx->modedata.ts.ver != (count_type)-1) {
	    int err =
	        ofd_ts_validate_state(ofdtab+ofdtx->ofd, ofdtx->modedata.ts.ver);

        if (err) {
            abort();
        }

        if (err < 0) {
            return err;
        }
    }

    /* validate versions of read records
     */

    if (ofdtx->type == TYPE_REGULAR) {

        struct ioop *ioop;
        int err = 0;

        for (ioop = ofdtx->rdtab;
            (ioop < ofdtx->rdtab+ofdtx->rdtablen) && !err; ++ioop) {
            err = ofd_ts_validate_region(ofdtab+ofdtx->ofd,
                                         ioop->nbyte,
                                         ioop->off,
                                        &ofdtx->modedata.ts.cmapss);
        }

        if (err < 0) {
            return ERR_CONFLICT;
        }
    }

    return 0;
}

static int
ofdtx_validate_2pl(struct ofdtx *ofdtx)
{
    assert(ofdtx);

    /* Locked regions are ours, so we do not need to validate here. All
     * conflicting transactions will have aborted on encountering our locks.
     *
     * The state of the OFD itself is guarded by ofd::rwlock.
     */
    return 0;
}

static int
ofdtx_validate_2pl_ext(struct ofdtx *ofdtx)
{
    assert(ofdtx);
    assert(ofdtx->ccmode == CC_MODE_2PL_EXT);

    /* Locked regions are ours, so why do we need to validate here. All
     * conflicting transactions will have aborted on encountering our locks.
     *
     * The state of the OFD itself is guarded by struct ofd::rwlock.
     */
    return 0;
}

int
ofdtx_validate(struct ofdtx *ofdtx)
{
    static int (* const validate[])(struct ofdtx*) = {
        ofdtx_validate_noundo,
        ofdtx_validate_ts,
        ofdtx_validate_2pl,
        ofdtx_validate_2pl_ext
    };

    if (!ofdtx_holds_ref(ofdtx)) {
        return 0;
    }

    return validate[ofdtx->ccmode](ofdtx);
}

/*
 * Update CC
 */

static int
ofdtx_updatecc_noundo(struct ofdtx *ofdtx)
{
    return 0;
}

static int
ofdtx_updatecc_ts(struct ofdtx *ofdtx)
{
    assert(ofdtx);
    assert(ofdtx->ccmode == CC_MODE_TS);

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* update version of open file descriptor */

    if (ofdtx->flags & OFDTX_FL_TL_INCVER) {
        ofd_ts_inc_state_version(ofd);
    }

    /* update version numbers of file buffer records */

    int err = 0;

    if (ofdtx->type == TYPE_REGULAR) {

        const struct ioop *ioop;

        for (ioop = ofdtx->wrtab;
            (ioop < ofdtx->wrtab+ofdtx->wrtablen) && !err; ++ioop) {
            err = ofd_ts_inc_region_versions(ofd,
                                             ioop->nbyte,
                                             ioop->off,
                                            &ofdtx->modedata.ts.cmapss);
        }
    }

    return err;
}

static int
ofdtx_updatecc_2pl(struct ofdtx *ofdtx)
{
    assert(ofdtx);
    assert(ofdtx->ccmode == CC_MODE_2PL);

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* release record locks */

    if (ofdtx->type == TYPE_REGULAR) {
        ofdtx_2pl_release_locks(ofdtx);
    }

    /* release ofd lock */

    ofd_rwunlock_state(ofd, &ofdtx->modedata.tpl.rwstate);

    return 0;
}

static int
ofdtx_updatecc_2pl_ext(struct ofdtx *ofdtx)
{
    assert(ofdtx);
    assert(ofdtx->ccmode == CC_MODE_2PL_EXT);

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* release record locks */

    if (ofdtx->type == TYPE_REGULAR) {
        ofdtx_2pl_release_locks(ofdtx);
    }

    /* release ofd lock */

    ofd_rwunlock_state(ofd, &ofdtx->modedata.tpl.rwstate);

    return 0;
}

int
ofdtx_updatecc(struct ofdtx *ofdtx)
{
    static int (* const updatecc[])(struct ofdtx*) = {
        ofdtx_updatecc_noundo,
        ofdtx_updatecc_ts,
        ofdtx_updatecc_2pl,
        ofdtx_updatecc_2pl_ext
    };

    assert(ofdtx_holds_ref(ofdtx));

    return updatecc[ofdtx->ccmode](ofdtx);
}

/*
 * Clear CC
 */

static int
ofdtx_clearcc_noundo(struct ofdtx *ofdtx)
{
    assert(ofdtx);
    assert(ofdtx->ccmode == CC_MODE_NOUNDO);

    return 0;
}

static int
ofdtx_clearcc_ts(struct ofdtx *ofdtx)
{
    assert(ofdtx);
    assert(ofdtx->ccmode == CC_MODE_TS);

    return 0;
}

static int
ofdtx_clearcc_2pl(struct ofdtx *ofdtx)
{
    assert(ofdtx);
    assert(ofdtx->ccmode == CC_MODE_2PL);

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* release record locks */

    if (ofdtx->type == TYPE_REGULAR) {
        ofdtx_2pl_release_locks(ofdtx);
    }

    /* release ofd lock */

    ofd_rwunlock_state(ofd, &ofdtx->modedata.tpl.rwstate);

    return 0;
}

static int
ofdtx_clearcc_2pl_ext(struct ofdtx *ofdtx)
{
    assert(ofdtx);
    assert(ofdtx->ccmode == CC_MODE_2PL_EXT);

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* release record locks */

    if (ofdtx->type == TYPE_REGULAR) {
        ofdtx_2pl_release_locks(ofdtx);
    }

    /* release ofd lock */

    ofd_rwunlock_state(ofd, &ofdtx->modedata.tpl.rwstate);

    return 0;
}

int
ofdtx_clearcc(struct ofdtx *ofdtx)
{
    static int (* const clearcc[])(struct ofdtx*) = {
        ofdtx_clearcc_noundo,
        ofdtx_clearcc_ts,
        ofdtx_clearcc_2pl,
        ofdtx_clearcc_2pl_ext
    };

    assert(ofdtx_holds_ref(ofdtx));

    return clearcc[ofdtx->ccmode](ofdtx);
}

/*
 * Referencing
 */

enum error_code
ofdtx_ref(struct ofdtx *ofdtx, int ofdindex, int fildes, unsigned long flags, int *optcc)
{
    assert(ofdtx);
    assert(ofdindex >= 0);
    assert(ofdindex < sizeof(ofdtab)/sizeof(ofdtab[0]));

    if (!ofdtx_holds_ref(ofdtx)) {

        /* get reference and status */

        off_t offset;
        enum ofd_type type;
        enum ccmode ccmode;

        struct ofd *ofd = ofdtab+ofdindex;

        int err = ofd_ref_state(ofd, fildes, flags, &type, &ccmode, &offset);

        if (err) {
            return err;
        }

        /* setup fields */

        ofdtx->ofd = ofdindex;
        ofdtx->type = type;
        ofdtx->ccmode = ccmode;
        ofdtx->offset = offset;
        ofdtx->size = 0;
        ofdtx->flags = 0;

        ofdtx->conn = NULL;

        ofdtx->fcntltablen = 0;
        ofdtx->seektablen = 0;
        ofdtx->rdtablen = 0;
        ofdtx->wrtablen = 0;
        ofdtx->wrbuflen = 0;

        ofdtx->modedata.ts.ver = (count_type)-1;
        cmapss_clear(&ofdtx->modedata.ts.cmapss);
        ofdtx->modedata.ts.locktablen = 0;

        ofdtx->modedata.tpl.rwstate = RW_NOLOCK;
        ofdtx->modedata.tpl.locktablen = 0;
    }

    if (optcc) {
        /* Signal optimistic CC */
        *optcc = (ofdtx->ccmode == CC_MODE_TS);
    }

    return 0;
}

void
ofdtx_unref(struct ofdtx *ofdtx)
{
    assert(ofdtx);

    if (!ofdtx_holds_ref(ofdtx)) {
        return;
    }

    if (ofdtx->conn) {
        connection_destroy(ofdtx->conn);
    }

    struct ofd *ofd = ofdtab+ofdtx->ofd;
    ofd_unref(ofd);

    ofdtx->ofd = -1;
}

int
ofdtx_holds_ref(struct ofdtx *ofdtx)
{
    assert(ofdtx);

    return (ofdtx->ofd >= 0) &&
           (ofdtx->ofd < sizeof(ofdtab)/sizeof(ofdtab[0]));
}

static off_t
ofdtx_append_to_iobuffer(struct ofdtx *ofdtx, size_t nbyte, const void *buf)
{
    off_t bufoffset;

    assert(ofdtx);

    bufoffset = ofdtx->wrbuflen;

    if (nbyte && buf) {

        /* resize */
        void *tmp = systx_tabresize(ofdtx->wrbuf,
                                    ofdtx->wrbuflen,
                                    ofdtx->wrbuflen+nbyte,
                                    sizeof(ofdtx->wrbuf[0]));
        if (!tmp) {
            return (off_t)ERR_SYSTEM;
        }
        ofdtx->wrbuf = tmp;

        /* append */
        memcpy(ofdtx->wrbuf+ofdtx->wrbuflen, buf, nbyte);
        ofdtx->wrbuflen += nbyte;
    }

    return bufoffset;
}

int
ofdtx_append_to_writeset(struct ofdtx *ofdtx, size_t nbyte,
                                              off_t offset,
                                              const void *buf)
{
    off_t bufoffset;

    assert(ofdtx);

    bufoffset = ofdtx_append_to_iobuffer(ofdtx, nbyte, buf);

    if (!(bufoffset >= 0)) {
        return (int)bufoffset;
    }

    return iooptab_append(&ofdtx->wrtab,
                          &ofdtx->wrtablen,
                          &ofdtx->wrtabsiz, nbyte, offset, bufoffset);
}

int
ofdtx_append_to_readset(struct ofdtx *ofdtx, size_t nbyte,
                                             off_t offset,
                                             const void *buf)
{
    off_t bufoffset;

    assert(ofdtx);

    bufoffset = ofdtx_append_to_iobuffer(ofdtx, nbyte, buf);

    if (!(bufoffset >= 0)) {
        return (int)bufoffset;
    }

    return iooptab_append(&ofdtx->rdtab,
                          &ofdtx->rdtablen,
                          &ofdtx->rdtabsiz, nbyte, offset, bufoffset);
}

/*
 * prepare commit
 */

int
ofdtx_pre_commit(struct ofdtx *ofdtx)
{
    assert(ofdtx);

    if (ofdtx->ccmode == CC_MODE_TS) {

        const struct ioop *ioop, *ioopend;
        const struct region *reg, *regend;

        /* collect write set regions */

        ioop = ofdtx->wrtab;
        ioopend = ofdtx->wrtab+ofdtx->wrtablen;

        while (ioop < ioopend) {

            int err;

            if ((err = regiontab_append(&ofdtx->modedata.ts.locktab,
                                        &ofdtx->modedata.ts.locktablen,
                                        &ofdtx->modedata.ts.locktabsiz,
                                         ioop->nbyte,
                                         ioop->off)) < 0) {
                return err;
            }

            ++ioop;
        }

        /* collect read set regions */

        ioop = ofdtx->rdtab;
        ioopend = ofdtx->rdtab+ofdtx->rdtablen;

        while (ioop < ioopend) {

            int err;

            if ((err = regiontab_append(&ofdtx->modedata.ts.locktab,
                                        &ofdtx->modedata.ts.locktablen,
                                        &ofdtx->modedata.ts.locktabsiz,
                                         ioop->nbyte,
                                         ioop->off)) < 0) {
                return err;
            }

            ++ioop;
        }

        /* sort lockset by offset order, to prevent deadlocks
           with other transactions */

        if (regiontab_sort(ofdtx->modedata.ts.locktab,
                           ofdtx->modedata.ts.locktablen) < 0) {
            abort();
        }

        /* lock regions */

        reg = ofdtx->modedata.ts.locktab;
        regend = ofdtx->modedata.ts.locktab+ofdtx->modedata.ts.locktablen;

        while (reg < regend) {

            int err = ofd_ts_lock_region(ofdtab+ofdtx->ofd,
                                         reg->nbyte,
                                         reg->offset,
                                        &ofdtx->modedata.ts.cmapss);

            if (err) {
                abort();
            }

            ++reg;
        }

        if (ofdtx->flags&OFDTX_FL_LOCALSTATE) {
            ofd_wrlock(ofdtab+ofdtx->ofd);
        }
    }

    return 0;
}

/*
 * cleanup commit
 */

int
ofdtx_post_commit(struct ofdtx *ofdtx)
{
    assert(ofdtx);

    if (ofdtx->ccmode == CC_MODE_TS) {

        const struct region *reg, *regbeg;

        if (ofdtx->flags&OFDTX_FL_LOCALSTATE) {
            ofd_unlock(ofdtab+ofdtx->ofd);
        }

        /* unlock regions by reversed offset order */

        reg = ofdtx->modedata.ts.locktab+ofdtx->modedata.ts.locktablen;
        regbeg = ofdtx->modedata.ts.locktab;

        while (reg > regbeg) {

            --reg;

            int err = ofd_ts_unlock_region(ofdtab+ofdtx->ofd,
                                            reg->nbyte,
                                            reg->offset,
                                          &ofdtx->modedata.ts.cmapss);

            if (err) {
                abort();
            }
        }
    }

    return 0;
}

int
ofdtx_is_optimistic(const struct ofdtx *ofdtx)
{
    assert(ofdtx);

    return ofdtx->ccmode == CC_MODE_TS;
}

/*
 * Optimistic CC
 */

int
ofdtx_ts_get_state_version(struct ofdtx *ofdtx)
{
    assert(ofdtx);

    if (ofdtx->modedata.ts.ver == (count_type)-1) {

        struct ofd *ofd = ofdtab+ofdtx->ofd;

        ofdtx->modedata.ts.ver = ofd_ts_get_state_version(ofd);
    }

    return 0;
}

int
ofdtx_ts_get_region_versions(struct ofdtx *ofdtx, size_t nbyte, off_t offset)
{
    assert(ofdtx);

    return ofd_ts_get_region_versions(ofdtab+ofdtx->ofd,
                                      nbyte,
                                      offset,
                                     &ofdtx->modedata.ts.cmapss);
}

int
ofdtx_ts_validate_state(struct ofdtx *ofdtx)
{
    assert(ofdtx);

    if (ofdtx->modedata.ts.ver == (count_type)-1) {
        return 0;
    }

    return ofd_ts_validate_state(ofdtab+ofdtx->ofd, ofdtx->modedata.ts.ver);
}

int
ofdtx_ts_validate_region(struct ofdtx *ofdtx, size_t nbyte, off_t offset)
{
    assert(ofdtx);

    if (!(ofdtx->flags&OFDTX_FL_LOCALBUF)) {
        return 0;
    }

    return ofd_ts_validate_region(ofdtab+ofdtx->ofd,
                                  nbyte,
                                  offset, &ofdtx->modedata.ts.cmapss);
}

/*
 * Pessimistic CC
 */

int
ofdtx_2pl_lock_region(struct ofdtx *ofdtx, size_t nbyte,
                                           off_t offset,
                                           int write)
{
    int err;

    assert(ofdtx);

    err = ofd_2pl_lock_region(ofdtab+ofdtx->ofd,
                              offset,
                              nbyte,
                              write,
                             &ofdtx->modedata.tpl.rwstatemap);

    if (!err) {
        err = regiontab_append(&ofdtx->modedata.tpl.locktab,
                               &ofdtx->modedata.tpl.locktablen,
                               &ofdtx->modedata.tpl.locktabsiz,
                                nbyte, offset);
    }

    return err;
}

#include <stdio.h>

void
ofdtx_dump(const struct ofdtx *ofdtx)
{
    fprintf(stderr, "%p: %d %p %zu %p %zu %p %zu %ld\n", (void*)ofdtx,
                                                                ofdtx->ofd,
                                                         (void*)ofdtx->seektab,
                                                                ofdtx->seektablen,
                                                         (void*)ofdtx->wrtab,
                                                                ofdtx->wrtablen,
                                                         (void*)ofdtx->rdtab,
                                                                ofdtx->rdtablen,
                                                                ofdtx->offset);
}

