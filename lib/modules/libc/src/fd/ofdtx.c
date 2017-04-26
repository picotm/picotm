/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ofdtx.h"
#include <assert.h>
#include <errno.h>
#include <picotm/picotm-module.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "errcode.h"
#include "event.h"
#include "fcntloptab.h"
#include "ioop.h"
#include "iooptab.h"
#include "ofdtab.h"
#include "range.h"
#include "region.h"
#include "regiontab.h"
#include "seekop.h"
#include "seekoptab.h"

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

    ofdtx->offset = 0;
    ofdtx->size = 0;
    ofdtx->type = PICOTM_LIBC_FILE_TYPE_OTHER;
    ofdtx->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;

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

    if (ofdtx->type == PICOTM_LIBC_FILE_TYPE_REGULAR) {

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
    assert(ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_2PL_EXT);

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

    return validate[ofdtx->cc_mode](ofdtx);
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
    assert(ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_TS);

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* update version of open file descriptor */

    if (ofdtx->flags & OFDTX_FL_TL_INCVER) {
        ofd_ts_inc_state_version(ofd);
    }

    /* update version numbers of file buffer records */

    int err = 0;

    if (ofdtx->type == PICOTM_LIBC_FILE_TYPE_REGULAR) {

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
    assert(ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_2PL);

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* release record locks */

    if (ofdtx->type == PICOTM_LIBC_FILE_TYPE_REGULAR) {
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
    assert(ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_2PL_EXT);

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* release record locks */

    if (ofdtx->type == PICOTM_LIBC_FILE_TYPE_REGULAR) {
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

    return updatecc[ofdtx->cc_mode](ofdtx);
}

/*
 * Clear CC
 */

static int
ofdtx_clearcc_noundo(struct ofdtx *ofdtx)
{
    assert(ofdtx);
    assert(ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO);

    return 0;
}

static int
ofdtx_clearcc_ts(struct ofdtx *ofdtx)
{
    assert(ofdtx);
    assert(ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_TS);

    return 0;
}

static int
ofdtx_clearcc_2pl(struct ofdtx *ofdtx)
{
    assert(ofdtx);
    assert(ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_2PL);

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* release record locks */

    if (ofdtx->type == PICOTM_LIBC_FILE_TYPE_REGULAR) {
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
    assert(ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_2PL_EXT);

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* release record locks */

    if (ofdtx->type == PICOTM_LIBC_FILE_TYPE_REGULAR) {
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

    return clearcc[ofdtx->cc_mode](ofdtx);
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
        enum picotm_libc_file_type type;
        enum picotm_libc_cc_mode cc_mode;

        struct ofd *ofd = ofdtab+ofdindex;

        int err = ofd_ref_state(ofd, fildes, flags, &type, &cc_mode, &offset);

        if (err) {
            return err;
        }

        /* setup fields */

        ofdtx->ofd = ofdindex;
        ofdtx->type = type;
        ofdtx->cc_mode = cc_mode;
        ofdtx->offset = offset;
        ofdtx->size = 0;
        ofdtx->flags = 0;

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
        *optcc = (ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_TS);
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
        void *tmp = picotm_tabresize(ofdtx->wrbuf,
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

    if (ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_TS) {

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

    if (ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_TS) {

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

    return ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_TS;
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

/*
 * bind()
 */

static int
ofdtx_bind_exec_noundo(struct ofdtx *ofdtx, int sockfd, const struct sockaddr *addr, socklen_t addrlen, int *cookie)
{
    return bind(sockfd, addr, addrlen);
}

int
ofdtx_bind_exec(struct ofdtx *ofdtx, int sockfd,
                               const struct sockaddr *addr,
                                     socklen_t addrlen,
                                     int *cookie,
                                     int noundo)
{
    static int (* const bind_exec[][4])(struct ofdtx*,
                                        int,
                                  const struct sockaddr*,
                                        socklen_t,
                                        int*) = {
        {ofdtx_bind_exec_noundo, NULL, NULL, NULL},
        {ofdtx_bind_exec_noundo, NULL, NULL, NULL},
        {ofdtx_bind_exec_noundo, NULL, NULL, NULL},
        {ofdtx_bind_exec_noundo, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(bind_exec)/sizeof(bind_exec[0]));
    assert(bind_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !bind_exec[ofdtx->type][ofdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return bind_exec[ofdtx->type][ofdtx->cc_mode](ofdtx,
                                                  sockfd,
                                                  addr,
                                                  addrlen, cookie);
}

static int
ofdtx_bind_apply_noundo(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    return 0;
}

int
ofdtx_bind_apply(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    static int (* const bind_apply[][4])(struct ofdtx*, int, const struct com_fd_event*, size_t) = {
        {ofdtx_bind_apply_noundo, NULL, NULL, NULL},
        {ofdtx_bind_apply_noundo, NULL, NULL, NULL},
        {ofdtx_bind_apply_noundo, NULL, NULL, NULL},
        {ofdtx_bind_apply_noundo, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(bind_apply)/sizeof(bind_apply[0]));
    assert(bind_apply[ofdtx->type]);

    return bind_apply[ofdtx->type][ofdtx->cc_mode](ofdtx, sockfd, event, n);
}

int
ofdtx_bind_undo(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    static int (* const bind_undo[][4])(int) = {
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(bind_undo)/sizeof(bind_undo[0]));
    assert(bind_undo[ofdtx->type]);

    return bind_undo[ofdtx->type][ofdtx->cc_mode](cookie);
}

/*
 * connect()
 */

static int
ofdtx_connect_exec_noundo(struct ofdtx *ofdtx, int sockfd,
                                               const struct sockaddr *serv_addr,
                                               socklen_t addrlen,
                                               int *cookie)
{
    return TEMP_FAILURE_RETRY(connect(sockfd, serv_addr, addrlen));
}

int
ofdtx_connect_exec(struct ofdtx *ofdtx, int sockfd,
                                        const struct sockaddr *serv_addr,
                                        socklen_t addrlen,
                                        int *cookie,
                                        int noundo)
{
    static int (* const connect_exec[][4])(struct ofdtx*,
                                           int,
                                           const struct sockaddr*,
                                           socklen_t,
                                           int*) = {
        {ofdtx_connect_exec_noundo, NULL, NULL, NULL},
        {ofdtx_connect_exec_noundo, NULL, NULL, NULL},
        {ofdtx_connect_exec_noundo, NULL, NULL, NULL},
        {ofdtx_connect_exec_noundo, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(connect_exec)/sizeof(connect_exec[0]));
    assert(connect_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !connect_exec[ofdtx->type][ofdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return connect_exec[ofdtx->type][ofdtx->cc_mode](ofdtx,
                                                     sockfd,
                                                     serv_addr,
                                                     addrlen, cookie);
}

static int
ofdtx_connect_apply_noundo(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    return 0;
}

int
ofdtx_connect_apply(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    static int (* const connect_apply[][4])(struct ofdtx*, int, const struct com_fd_event*, size_t) = {
        {ofdtx_connect_apply_noundo, NULL, NULL, NULL},
        {ofdtx_connect_apply_noundo, NULL, NULL, NULL},
        {ofdtx_connect_apply_noundo, NULL, NULL, NULL},
        {ofdtx_connect_apply_noundo, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(connect_apply)/sizeof(connect_apply[0]));
    assert(connect_apply[ofdtx->type]);

    return connect_apply[ofdtx->type][ofdtx->cc_mode](ofdtx, sockfd, event, n);
}

int
ofdtx_connect_undo(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    static int (* const connect_undo[][4])(struct ofdtx*, int, int) = {
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(connect_undo)/sizeof(connect_undo[0]));
    assert(connect_undo[ofdtx->type]);

    return connect_undo[ofdtx->type][ofdtx->cc_mode](ofdtx, sockfd, cookie);
}

/*
 * fcntl()
 */

int
ofdtx_fcntl_exec_noundo(struct ofdtx *ofdtx, int fildes,
                                             int cmd, union com_fd_fcntl_arg *arg,
                                             int *cookie)
{
    int res = 0;

    assert(arg);

    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN:

            arg->arg0 = TEMP_FAILURE_RETRY(fcntl(fildes, cmd));

            if (arg->arg0 < 0) {
                res = -1;
            }
            break;
        case F_GETLK:
            res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg1));
            break;
        case F_SETFL:
        case F_SETFD:
        case F_SETOWN:
            res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg0));
            break;
        case F_SETLK:
        case F_SETLKW:
            res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg1));
            break;
        default:
            errno = EINVAL;
            res = ERR_SYSTEM;
            break;
    }

    return res;
}

int
ofdtx_fcntl_exec_ts(struct ofdtx *ofdtx, int fildes,
                                         int cmd, union com_fd_fcntl_arg *arg,
                                         int *cookie)
{
    int res = 0;
    int err, valid;

    assert(arg);

    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN:

            /* fetch version number */
            if ((err = ofdtx_ts_get_state_version(ofdtx)) < 0) {
                return err;
            }

            arg->arg0 = TEMP_FAILURE_RETRY(fcntl(fildes, cmd));
            if (arg->arg0 < 0) {
                res = ERR_SYSTEM;
                break;
            }

            /* validate, as fcntl might have overlapped with commit */
            valid = !ofdtx_ts_validate_state(ofdtx);

            if (!valid) {
                return ERR_CONFLICT;
            }
            break;
        case F_GETLK:

            /* fetch version number */
            if ((err = ofdtx_ts_get_state_version(ofdtx)) < 0) {
                return err;
            }

            res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg1));

            if (!(res < 0)) {
                break;
            }

            /* validate, as fcntl might have overlapped with commit */
            valid = !ofdtx_ts_validate_state(ofdtx);

            if (!valid) {
                return ERR_CONFLICT;
            }
            break;
        case F_SETFL:
        case F_SETFD:
        case F_SETOWN:
        case F_SETLK:
        case F_SETLKW:
            res = ERR_NOUNDO;
            break;
        default:
            errno = EINVAL;
            res = ERR_SYSTEM;
            break;
    }

    if (res < 0) {
        return res;
    }

	ofdtx->flags |= OFDTX_FL_LOCALSTATE;

    return res;
}

int
ofdtx_fcntl_exec_2pl(struct ofdtx *ofdtx, int fildes,
                                          int cmd, union com_fd_fcntl_arg *arg,
                                          int *cookie)
{
    int res = 0;

    assert(arg);

    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN:

            /* Read-lock open file description */
            if ((res = ofd_rdlock_state(ofdtab+ofdtx->ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
                return res;
            }

            arg->arg0 = TEMP_FAILURE_RETRY(fcntl(fildes, cmd));
            if (arg->arg0 < 0) {
                res = -1;
            }
            break;
        case F_GETLK:

            /* Read-lock open file description */
            if ((res = ofd_rdlock_state(ofdtab+ofdtx->ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
                return res;
            }

            res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg1));
            break;
        case F_SETFL:
        case F_SETFD:
        case F_SETOWN:
        case F_SETLK:
        case F_SETLKW:
            res = ERR_NOUNDO;
            break;
        default:
            errno = EINVAL;
            res = ERR_SYSTEM;
            break;
    }

    if (res < 0) {
        return res;
    }

    return res;
}

int
ofdtx_fcntl_exec(struct ofdtx *ofdtx, int fildes,
                                      int cmd, union com_fd_fcntl_arg *arg,
                                      int *cookie, int noundo)
{
    static int (* const fcntl_exec[][4])(struct ofdtx*,
                                         int,
                                         int,
                                         union com_fd_fcntl_arg*, int*) = {
        {ofdtx_fcntl_exec_noundo, NULL,                NULL,                 NULL},
        {ofdtx_fcntl_exec_noundo, ofdtx_fcntl_exec_ts, ofdtx_fcntl_exec_2pl, NULL},
        {ofdtx_fcntl_exec_noundo, ofdtx_fcntl_exec_ts, ofdtx_fcntl_exec_2pl, NULL},
        {ofdtx_fcntl_exec_noundo, NULL,                NULL,                 NULL}};

    assert(ofdtx->type < sizeof(fcntl_exec)/sizeof(fcntl_exec[0]));
    assert(fcntl_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !fcntl_exec[ofdtx->type][ofdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return fcntl_exec[ofdtx->type][ofdtx->cc_mode](ofdtx, fildes, cmd, arg, cookie);
}

int
ofdtx_fcntl_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    return 0;
}

int
ofdtx_fcntl_undo(struct ofdtx *ofdtx, int fildes, int cookie)
{
    return 0;
}

/*
 * fsync()
 */

static int
ofdtx_fsync_exec_noundo(int fildes, int *cookie)
{
    return fsync(fildes);
}

static int
ofdtx_fsync_exec_regular_ts(int fildes, int *cookie)
{
    /* Signal apply/undo */
    *cookie = 0;

    return 0;
}

static int
ofdtx_fsync_exec_regular_2pl(int fildes, int *cookie)
{
    /* Signal apply/undo */
    *cookie = 0;

    return 0;
}

int
ofdtx_fsync_exec(struct ofdtx *ofdtx, int fildes, int noundo, int *cookie)
{
    static int (* const fsync_exec[][4])(int, int*) = {
        {ofdtx_fsync_exec_noundo, NULL,                        NULL,                         NULL},
        {ofdtx_fsync_exec_noundo, ofdtx_fsync_exec_regular_ts, ofdtx_fsync_exec_regular_2pl, NULL},
        {ofdtx_fsync_exec_noundo, NULL,                        NULL,                         NULL},
        {ofdtx_fsync_exec_noundo, NULL,                        NULL,                         NULL}};

    assert(ofdtx->type < sizeof(fsync_exec)/sizeof(fsync_exec[0]));

    if (noundo) {
        /* TX irrevokable */
        ofdtx->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !fsync_exec[ofdtx->type][ofdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return fsync_exec[ofdtx->type][ofdtx->cc_mode](fildes, cookie);
}

static int
ofdtx_fsync_apply_noundo(int fildes)
{
    return 0;
}

static int
ofdtx_fsync_apply_regular_ts(int fildes)
{
    return fsync(fildes);
}

static int
ofdtx_fsync_apply_regular_2pl(int fildes)
{
    return fsync(fildes);
}

int
ofdtx_fsync_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    static int (* const fsync_apply[][4])(int) = {
        {ofdtx_fsync_apply_noundo, NULL,                         NULL,                          NULL},
        {ofdtx_fsync_apply_noundo, ofdtx_fsync_apply_regular_ts, ofdtx_fsync_apply_regular_2pl, NULL},
        {ofdtx_fsync_apply_noundo, NULL,                         NULL,                          NULL},
        {ofdtx_fsync_apply_noundo, NULL,                         NULL,                          NULL}};

    assert(ofdtx->type < sizeof(fsync_apply)/sizeof(fsync_apply[0]));
    assert(fsync_apply[ofdtx->type][ofdtx->cc_mode]);

    return fsync_apply[ofdtx->type][ofdtx->cc_mode](fildes);
}

static int
ofdtx_fsync_undo_regular_ts(int fildes, int cookie)
{
    return 0;
}

static int
ofdtx_fsync_undo_regular_2pl(int fildes, int cookie)
{
    return 0;
}

int
ofdtx_fsync_undo(struct ofdtx *ofdtx, int fildes, int cookie)
{
    static int (* const fsync_undo[][4])(int, int) = {
        {NULL, NULL,                        NULL,                         NULL},
        {NULL, ofdtx_fsync_undo_regular_ts, ofdtx_fsync_undo_regular_2pl, NULL},
        {NULL, NULL,                        NULL,                         NULL},
        {NULL, NULL,                        NULL,                         NULL}};

    assert(ofdtx->type < sizeof(fsync_undo)/sizeof(fsync_undo[0]));
    assert(fsync_undo[ofdtx->type][ofdtx->cc_mode]);

    return fsync_undo[ofdtx->type][ofdtx->cc_mode](fildes, cookie);
}

/*
 * listen()
 */

static int
ofdtx_listen_exec_noundo(struct ofdtx *ofdtx, int sockfd, int backlog, int *cookie)
{
    return listen(sockfd, backlog);
}

static int
ofdtx_listen_exec_socket_2pl_ext(struct ofdtx *ofdtx, int sockfd, int backlog, int *cookie)
{
    int type;
    socklen_t typelen = sizeof(type);

    assert(ofdtx);

    /* Write-lock socket */

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    int err;

    if ((err = ofd_wrlock_state(ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
        return err;
    }

    /* Exclude non-stream sockets */

    if (getsockopt(sockfd, SOL_SOCKET, SO_TYPE, &type, &typelen) < 0) {
        return ERR_SYSTEM;
    }
    if (type != SOCK_STREAM) {
        return ERR_NOUNDO;
    }

    /* If the socket is in blocking mode, the transaction might wait forever
       and therefore block the whole system. Therefore only listen for a given
       amount of time and abort if no connection request was received. */

    int fl = fcntl(sockfd, F_GETFL);

    if (fl < 0) {
        return ERR_SYSTEM;
    } else if (fl&O_NONBLOCK) {

        fd_set rdset;
        FD_ZERO(&rdset);
        FD_SET(sockfd, &rdset);

        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;

        int numfds = TEMP_FAILURE_RETRY(select(FD_SETSIZE, &rdset, NULL, NULL, &tv));

        if (numfds < 0) {
            return ERR_SYSTEM;
        }

        int isset = FD_ISSET(sockfd, &rdset);

        FD_ZERO(&rdset);

        if (!isset) {
            return ERR_CONFLICT;
        }
    }

    return listen(sockfd, backlog);
}

int
ofdtx_listen_exec(struct ofdtx *ofdtx, int sockfd,
                                       int backlog,
                                       int *cookie,
                                       int noundo)
{
    static int (* const listen_exec[][4])(struct ofdtx*,
                                          int,
                                          int,
                                          int*) = {
        {ofdtx_listen_exec_noundo, NULL, NULL, NULL},
        {ofdtx_listen_exec_noundo, NULL, NULL, NULL},
        {ofdtx_listen_exec_noundo, NULL, NULL, NULL},
        {ofdtx_listen_exec_noundo, NULL, NULL, ofdtx_listen_exec_socket_2pl_ext}};

    assert(ofdtx->type < sizeof(listen_exec)/sizeof(listen_exec[0]));
    assert(listen_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !listen_exec[ofdtx->type][ofdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return listen_exec[ofdtx->type][ofdtx->cc_mode](ofdtx, sockfd, backlog, cookie);
}

static int
ofdtx_listen_apply_noundo()
{
    return 0;
}

static int
ofdtx_listen_apply_socket_2pl_ext()
{
    return 0;
}

int
ofdtx_listen_apply(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    static int (* const listen_apply[][4])(void) = {
        {ofdtx_listen_apply_noundo, NULL, NULL, NULL},
        {ofdtx_listen_apply_noundo, NULL, NULL, NULL},
        {ofdtx_listen_apply_noundo, NULL, NULL, NULL},
        {ofdtx_listen_apply_noundo, NULL, NULL, ofdtx_listen_apply_socket_2pl_ext}};

    assert(ofdtx->type < sizeof(listen_apply)/sizeof(listen_apply[0]));
    assert(listen_apply[ofdtx->type]);

    return listen_apply[ofdtx->type][ofdtx->cc_mode]();
}

int
ofdtx_listen_undo_socket_2pl_ext(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    return 0;
}

int
ofdtx_listen_undo(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    static int (* const listen_undo[][4])(struct ofdtx*, int, int) = {
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, ofdtx_listen_undo_socket_2pl_ext}};

    assert(ofdtx->type < sizeof(listen_undo)/sizeof(listen_undo[0]));
    assert(listen_undo[ofdtx->type]);

    return listen_undo[ofdtx->type][ofdtx->cc_mode](ofdtx, sockfd, cookie);
}

/*
 * lseek()
 */

static off_t
filesize(int fildes)
{
    struct stat buf;

    if (fstat(fildes, &buf) < 0) {
        return ERR_SYSTEM;
    }

    return buf.st_size;
}

static off_t
ofdtx_lseek_exec_noundo(struct ofdtx *ofdtx, int fildes,
                               off_t offset, int whence,
                                  int *cookie)
{
    return TEMP_FAILURE_RETRY(lseek(fildes, offset, whence));
}

static off_t
ofdtx_lseek_exec_regular_ts(struct ofdtx *ofdtx, int fildes,
                                                 off_t offset, int whence,
                                                 int *cookie)
{
    int err;

	/* fastpath: read current position */
	if (!offset && (whence == SEEK_CUR)) {
		ofdtx->flags |= OFDTX_FL_LOCALSTATE;
		return ofdtx->offset;
	}

    ofdtx->size = llmax(ofdtx->offset, ofdtx->size);

    /* fetch version number */
    if ((err = ofdtx_ts_get_state_version(ofdtx)) < 0) {
        return err;
    }

    /* Compute absolute position */

    off_t pos;

    switch (whence) {
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_CUR:
            pos = ofdtx->offset + offset;
            break;
        case SEEK_END:
            {
                const off_t fs = filesize(fildes);

                if (fs == (off_t)-1) {
                    pos = (off_t)ERR_SYSTEM;
                    break;
                }

                pos = llmax(ofdtx->size, fs)+offset;

                /* validate, as file size might have changed */
                int valid = !ofdtx_ts_validate_state(ofdtx);

                if (!valid) {
                    return ERR_CONFLICT;
                }
            }
            break;
        default:
            pos = -1;
            break;
    }

    if (pos < 0) {
        errno = EINVAL;
        pos = (off_t)ERR_SYSTEM;
    }

    if ((pos == (off_t)-2) || (pos == (off_t)-1)) {
        return pos;
    }

    if (cookie) {
        *cookie = seekoptab_append(&ofdtx->seektab,
                                   &ofdtx->seektablen,
                                    ofdtx->offset, offset, whence);

        if (*cookie < 0) {
            abort();
        }
    }

    ofdtx->offset = pos; /* update file pointer */
	ofdtx->flags |= OFDTX_FL_LOCALSTATE|OFDTX_FL_TL_INCVER;

    return pos;
}

static off_t
ofdtx_lseek_exec_regular_2pl(struct ofdtx *ofdtx, int fildes,
                                         off_t offset, int whence,
                                             int *cookie)
{
    int err;

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* Read-lock open file description */

    if ((err = ofd_rdlock_state(ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
        return (off_t)err;
    }

	/* Fastpath: Read current position */
	if (!offset && (whence == SEEK_CUR)) {
		return ofdtx->offset;
	}

    /* Write-lock open file description to change position */

    if ((err = ofd_wrlock_state(ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
        return (off_t)err;
    }

    /* Compute absolute position */

    ofdtx->size = llmax(ofdtx->offset, ofdtx->size);

    off_t pos;

    switch (whence) {
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_CUR:
            pos = ofdtx->offset + offset;
            break;
        case SEEK_END:
            {
                const off_t fs = filesize(fildes);

                if (fs == (off_t)ERR_SYSTEM) {
                    pos = (off_t)ERR_SYSTEM;
                    break;
                }

                pos = llmax(ofdtx->size, fs)+offset;
            }
            break;
        default:
            pos = -1;
            break;
    }

    if (pos < 0) {
        errno = EINVAL;
        pos = (off_t)ERR_SYSTEM;
    }

    if ((pos == (off_t)-2) || (pos == (off_t)-1)) {
        return pos;
    }

    if (cookie) {
        *cookie = seekoptab_append(&ofdtx->seektab,
                                   &ofdtx->seektablen,
                                    ofdtx->offset, offset, whence);

        if (*cookie < 0) {
            abort();
        }
    }

    ofdtx->offset = pos; /* Update file pointer */

    return pos;
}

static off_t
ofdtx_lseek_exec_fifo_ts(struct ofdtx *ofdtx, int fildes,
                                              off_t offset, int whence,
                                              int *cookie)
{
    errno = EPIPE;

    return ERR_SYSTEM;
}

off_t
ofdtx_lseek_exec(struct ofdtx *ofdtx, int fildes,  off_t offset,
                                            int whence, int *cookie,
                                            int noundo)
{
    static off_t (* const lseek_exec[][4])(struct ofdtx*,
                                           int,
                                           off_t,
                                           int,
                                           int*) = {
        {ofdtx_lseek_exec_noundo, NULL,                        NULL,                         NULL},
        {ofdtx_lseek_exec_noundo, ofdtx_lseek_exec_regular_ts, ofdtx_lseek_exec_regular_2pl, NULL},
        {ofdtx_lseek_exec_noundo, ofdtx_lseek_exec_fifo_ts,    NULL,                         NULL},
        {ofdtx_lseek_exec_noundo, NULL,                        NULL,                         NULL}};

    assert(ofdtx->type < sizeof(lseek_exec)/sizeof(lseek_exec[0]));
    assert(lseek_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !lseek_exec[ofdtx->type][ofdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return lseek_exec[ofdtx->type][ofdtx->cc_mode](ofdtx, fildes, offset, whence, cookie);
}

static int
ofdtx_lseek_apply_noundo(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    return 0;
}

static int
ofdtx_lseek_apply_regular(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    int err = 0;

    while (n && !err) {
        const off_t pos = lseek(fildes, ofdtx->seektab[event->cookie].offset,
                                        ofdtx->seektab[event->cookie].whence);

        if (pos == (off_t)-1) {
            err = ERR_SYSTEM;
            break;
        }

        struct ofd *ofd = ofdtab+ofdtx->ofd;

        ofd->data.regular.offset = pos;

        --n;
        ++event;
    }

    return err;
}

int
ofdtx_lseek_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    static int (* const lseek_apply[][4])(struct ofdtx*, int, const struct com_fd_event*, size_t) = {
        {ofdtx_lseek_apply_noundo, NULL,                      NULL,                      NULL},
        {ofdtx_lseek_apply_noundo, ofdtx_lseek_apply_regular, ofdtx_lseek_apply_regular, NULL},
        {ofdtx_lseek_apply_noundo, NULL,                      NULL,                      NULL},
        {ofdtx_lseek_apply_noundo, NULL,                      NULL,                      NULL}};

    assert(ofdtx->type < sizeof(lseek_apply)/sizeof(lseek_apply[0]));
    assert(lseek_apply[ofdtx->type][ofdtx->cc_mode]);

    return lseek_apply[ofdtx->type][ofdtx->cc_mode](ofdtx, fildes, event, n);
}

static int
ofdtx_lseek_undo_regular(void)
{
    return 0;
}

int
ofdtx_lseek_undo(struct ofdtx *ofdtx, int fildes, int cookie)
{
    static int (* const lseek_undo[][4])() = {
        {NULL, NULL,                     NULL,                     NULL},
        {NULL, ofdtx_lseek_undo_regular, ofdtx_lseek_undo_regular, NULL},
        {NULL, NULL,                     NULL,                     NULL},
        {NULL, NULL,                     NULL,                     NULL}};

    assert(ofdtx->type < sizeof(lseek_undo)/sizeof(lseek_undo[0]));
    assert(lseek_undo[ofdtx->type][ofdtx->cc_mode]);

    return lseek_undo[ofdtx->type][ofdtx->cc_mode]();
}

/*
 * pread()
 */

/*

Busy-wait instead of syscall

typedef unsigned long long ticks;
static __inline__ ticks getticks(void)
{
     unsigned a, d;
     __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));
     return ((ticks)a) | (((ticks)d) << 32);
}

static ssize_t
pread_wait(int fildes, void *buf, size_t nbyte, off_t off)
{
    ticks t0 = getticks();
    memset(buf, 0, nbyte);
    ticks t1 = getticks();

    if (!nbyte) {
        while ( (t1-t0) < 631 ) {
            t1 = getticks();
        }
    } else {

        // approximation of single-thread cycles
        long limit = 1020 + (339*nbyte)/1000;

        while ( (t1-t0) < limit ) {
            t1 = getticks();
        }
    }

    return nbyte;
}*/

static ssize_t
ofdtx_pread_exec_noundo(struct ofdtx *ofdtx, int fildes,
                                             void *buf,
                                             size_t nbyte,
                                             off_t offset,
                                             int *cookie,
                                             enum picotm_libc_validation_mode val_mode)
{
    return TEMP_FAILURE_RETRY(pread(fildes, buf, nbyte, offset));
}

static ssize_t
ofdtx_pread_exec_regular_ts(struct ofdtx *ofdtx, int fildes, void *buf,
                                                 size_t nbyte, off_t offset,
                                                 int *cookie,
                                                 enum picotm_libc_validation_mode val_mode)
{
    int err;
    ssize_t len, len2;

    assert(ofdtx);

    if ((err = ofdtx_ts_get_region_versions(ofdtx, nbyte, offset)) < 0) {
        return err;
    }

    if ((len = TEMP_FAILURE_RETRY(pread(fildes, buf, nbyte, offset))) < 0) {
        return len;
    }

    /* signal conflict, if global version numbers changed */
    if ((val_mode == PICOTM_LIBC_VALIDATE_OP) &&
        !!ofdtx_ts_validate_region(ofdtx, nbyte, offset)) {
        return ERR_CONFLICT;
    }

    if ((len2 = iooptab_read(ofdtx->wrtab,
                             ofdtx->wrtablen, buf, nbyte, offset,
                             ofdtx->wrbuf)) < 0) {
        return len2;
    }

    ssize_t res = llmax(len, len2);

    /* add to read set for later validation */
    if ((err = ofdtx_append_to_readset(ofdtx, res, offset, NULL)) < 0) {
        return err;
    }

    ofdtx->flags |= OFDTX_FL_LOCALBUF;

    return res;
}

static ssize_t
ofdtx_pread_exec_regular_2pl(struct ofdtx *ofdtx, int fildes, void *buf,
                                                  size_t nbyte, off_t offset,
                                                  int *cookie,
                                                  enum picotm_libc_validation_mode val_mode)
{
    int err;
    ssize_t len, len2;

    /* lock region */
    if ((err = ofdtx_2pl_lock_region(ofdtx, nbyte, offset, 0)) < 0) {
        return err;
    }

    /* read from file */
    if ((len = TEMP_FAILURE_RETRY(pread(fildes, buf, nbyte, offset))) < 0) {
        return len;
    }

    /* read from local write set */
    if ((len2 = iooptab_read(ofdtx->wrtab,
                             ofdtx->wrtablen,
                             buf, nbyte, offset, ofdtx->wrbuf)) < 0) {
        return len2;
    }

    return llmax(len, len2);
}

static ssize_t
ofdtx_pread_exec_fifo(struct ofdtx *ofdtx, int fildes, void *buf, size_t nbyte,
                                            off_t offset, int *cookie,
                                            enum picotm_libc_validation_mode val_mode)
{
    errno = ESPIPE;
    return ERR_SYSTEM;
}

ssize_t
ofdtx_pread_exec(struct ofdtx *ofdtx, int fildes, void *buf,
                                      size_t nbyte, off_t offset,
                                      int *cookie, int noundo,
                                      enum picotm_libc_validation_mode val_mode)
{
    static ssize_t (* const pread_exec[][4])(struct ofdtx*,
                                             int,
                                             void*,
                                             size_t,
                                             off_t,
                                             int*,
                                             enum picotm_libc_validation_mode) = {
        {ofdtx_pread_exec_noundo, NULL,                        NULL,                         NULL},
        {ofdtx_pread_exec_noundo, ofdtx_pread_exec_regular_ts, ofdtx_pread_exec_regular_2pl, NULL},
        {ofdtx_pread_exec_noundo, ofdtx_pread_exec_fifo,       ofdtx_pread_exec_fifo,        NULL},
        {ofdtx_pread_exec_noundo, NULL,                        NULL,                         NULL}};

    assert(ofdtx->type < sizeof(pread_exec)/sizeof(pread_exec[0]));
    assert(pread_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !pread_exec[ofdtx->type][ofdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return pread_exec[ofdtx->type][ofdtx->cc_mode](ofdtx, fildes, buf, nbyte, offset, cookie, val_mode);
}

static ssize_t
ofdtx_pread_apply_any(void)
{
    return 0;
}

ssize_t
ofdtx_pread_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    static ssize_t (* const pread_apply[][4])(void) = {
        {ofdtx_pread_apply_any, NULL,                  NULL,                  NULL},
        {ofdtx_pread_apply_any, ofdtx_pread_apply_any, ofdtx_pread_apply_any, NULL},
        {ofdtx_pread_apply_any, NULL,                  NULL,                  NULL},
        {ofdtx_pread_apply_any, NULL,                  NULL,                  NULL}};

    assert(ofdtx->type < sizeof(pread_apply)/sizeof(pread_apply[0]));
    assert(pread_apply[ofdtx->type]);

    int err;

    while (n && !err) {
        err = pread_apply[ofdtx->type][ofdtx->cc_mode]();
        --n;
    }

    return err;
}

static int
ofdtx_pread_undo_any(void)
{
    return 0;
}

int
ofdtx_pread_undo(struct ofdtx *ofdtx, int fildes, int cookie)
{
    static int (* const pread_undo[][4])(void) = {
        {NULL, NULL,                 NULL,                 NULL},
        {NULL, ofdtx_pread_undo_any, ofdtx_pread_undo_any, NULL},
        {NULL, NULL,                 NULL,                 NULL},
        {NULL, NULL,                 NULL,                 NULL}};

    assert(ofdtx->type < sizeof(pread_undo)/sizeof(pread_undo[0]));
    assert(pread_undo[ofdtx->type]);

    return pread_undo[ofdtx->type][ofdtx->cc_mode]();
}

/*
 * pwrite()
 */

/*

Busy-wait instead of syscall

typedef unsigned long long ticks;
static __inline__ ticks getticks(void)
{
     unsigned a, d;
     __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));
     return ((ticks)a) | (((ticks)d) << 32);
}

static ssize_t
pwrite_wait(int fildes, const void *buf, size_t nbyte, off_t off)
{
    ticks t0 = getticks();
    ticks t1 = getticks();

    if (!nbyte) {
        while ( (t1-t0) < 765 ) {
            t1 = getticks();
        }
    } else {

        // approximation of single-thread cycles
        long limit = 1724 + (1139*nbyte)/1000;

        while ( (t1-t0) < limit ) {
            t1 = getticks();
        }
    }

    return nbyte;
}*/

static ssize_t
ofdtx_pwrite_exec_noundo(struct ofdtx *ofdtx, int fildes,
                                              const void *buf,
                                              size_t nbyte,
                                              off_t offset,
                                              int *cookie)
{
    return TEMP_FAILURE_RETRY(pwrite(fildes, buf, nbyte, offset));
}

static ssize_t
ofdtx_pwrite_exec_regular_ts(struct ofdtx *ofdtx, int fildes, const void *buf,
                                                  size_t nbyte, off_t offset,
                                                  int *cookie)
{
    if (__builtin_expect(!!cookie, 1)) {

        /* add data to write set */
        if ((*cookie = ofdtx_append_to_writeset(ofdtx, nbyte, offset, buf)) < 0) {
            return *cookie;
        }
    }

    return nbyte;
}

static ssize_t
ofdtx_pwrite_exec_regular_2pl(struct ofdtx *ofdtx, int fildes, const void *buf,
                                                   size_t nbyte, off_t offset,
                                                   int *cookie)
{
    /* register written data */

    if (__builtin_expect(!!cookie, 1)) {

        int err;

        /* lock region */

        if ((err = ofdtx_2pl_lock_region(ofdtx, nbyte, offset, 1)) < 0) {
            return err;
        }

        /* add data to write set */
        if ((*cookie = ofdtx_append_to_writeset(ofdtx,
                                                nbyte,
                                                offset, buf)) < 0) {
            return *cookie;
        }
    }

    return nbyte;
}

static ssize_t
ofdtx_pwrite_exec_fifo_ts(struct ofdtx *ofdtx, int fildes, const void *buf,
                                               size_t nbyte, off_t offset,
                                               int *cookie)
{
    errno = ESPIPE;
    return ERR_SYSTEM;
}

ssize_t
ofdtx_pwrite_exec(struct ofdtx *ofdtx, int fildes, const void *buf,
                                       size_t nbyte, off_t offset,
                                       int *cookie,
                                       int noundo)
{
    static ssize_t (* const pwrite_exec[][4])(struct ofdtx*,
                                              int,
                                              const void*,
                                              size_t,
                                              off_t,
                                              int*) = {
        {ofdtx_pwrite_exec_noundo, NULL,                         NULL,                          NULL},
        {ofdtx_pwrite_exec_noundo, ofdtx_pwrite_exec_regular_ts, ofdtx_pwrite_exec_regular_2pl, NULL},
        {ofdtx_pwrite_exec_noundo, ofdtx_pwrite_exec_fifo_ts,    NULL,                          NULL},
        {ofdtx_pwrite_exec_noundo, NULL,                         NULL,                          NULL}};

    assert(ofdtx->type < sizeof(pwrite_exec)/sizeof(pwrite_exec[0]));
    assert(pwrite_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !pwrite_exec[ofdtx->type][ofdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return pwrite_exec[ofdtx->type][ofdtx->cc_mode](ofdtx, fildes, buf, nbyte, offset, cookie);
}

static ssize_t
ofdtx_pwrite_apply_noundo(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    return 0;
}

static ssize_t
ofdtx_pwrite_apply_regular(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    assert(ofdtx);
    assert(fildes >= 0);
    assert(event || !n);

    ssize_t len = 0;

    while (n && !(len < 0)) {

        /* Combine writes to adjacent locations */

        off_t  off = ofdtx->wrtab[event->cookie].off;
        size_t nbyte = ofdtx->wrtab[event->cookie].nbyte;
        off_t  bufoff = ofdtx->wrtab[event->cookie].bufoff;

        int m = 1;

        while ((m < n)
                && (ofdtx->wrtab[event[m].cookie].off == off+nbyte)
                && (ofdtx->wrtab[event[m].cookie].bufoff == bufoff+nbyte)) {

            nbyte += ofdtx->wrtab[event[m].cookie].nbyte;

            ++m;
        }

        len = TEMP_FAILURE_RETRY(pwrite(fildes,
                                        ofdtx->wrbuf+bufoff,
                                        nbyte, off));

        n -= m;
        event += m;
    }

    return len < 0 ? len : 0;
}

ssize_t
ofdtx_pwrite_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    static ssize_t (* const pwrite_apply[][4])(struct ofdtx*, int, const struct com_fd_event*, size_t) = {
        {ofdtx_pwrite_apply_noundo, NULL,                       NULL,                       NULL},
        {ofdtx_pwrite_apply_noundo, ofdtx_pwrite_apply_regular, ofdtx_pwrite_apply_regular, NULL},
        {ofdtx_pwrite_apply_noundo, NULL,                       NULL,                       NULL},
        {ofdtx_pwrite_apply_noundo, NULL,                       NULL,                       NULL}};

    assert(ofdtx->type < sizeof(pwrite_apply)/sizeof(pwrite_apply[0]));
    assert(pwrite_apply[ofdtx->type]);

    return pwrite_apply[ofdtx->type][ofdtx->cc_mode](ofdtx, fildes, event, n);
}

static int
ofdtx_pwrite_any_undo(void)
{
    return 0;
}

int
ofdtx_pwrite_undo(struct ofdtx *ofdtx, int fildes, int cookie)
{
    static int (* const pwrite_undo[][4])(void) = {
        {NULL, NULL,                  NULL,                  NULL},
        {NULL, ofdtx_pwrite_any_undo, ofdtx_pwrite_any_undo, NULL},
        {NULL, NULL,                  NULL,                  NULL},
        {NULL, NULL,                  NULL,                  NULL}};

    assert(ofdtx->type < sizeof(pwrite_undo)/sizeof(pwrite_undo[0]));
    assert(pwrite_undo[ofdtx->type]);

    return pwrite_undo[ofdtx->type][ofdtx->cc_mode]();
}

/*
 * read()
 */

static ssize_t
ofdtx_read_exec_noundo(struct ofdtx *ofdtx, int fildes, void *buf,
                                                      size_t nbyte,
                                                      int *cookie,
                                                      enum picotm_libc_validation_mode val_mode)
{
    return TEMP_FAILURE_RETRY(read(fildes, buf, nbyte));
}

static ssize_t
ofdtx_read_exec_regular_ts(struct ofdtx *ofdtx, int fildes, void *buf,
                                                      size_t nbyte,
                                                      int *cookie,
                                                      enum picotm_libc_validation_mode val_mode)
{
    int err;

    assert(ofdtx);

    if ((err = ofdtx_ts_get_state_version(ofdtx)) < 0) {
        return err;
    }
    if ((err = ofdtx_ts_get_region_versions(ofdtx, nbyte, ofdtx->offset)) < 0) {
        return err;
    }

    /* read from file descriptor */
    ssize_t len = pread(fildes, buf, nbyte, ofdtx->offset);

    if (len < 0) {
        return len;
    }

    if (!!ofdtx_ts_validate_state(ofdtx)
        || ((val_mode == PICOTM_LIBC_VALIDATE_OP)
            && !!ofdtx_ts_validate_region(ofdtx, nbyte, ofdtx->offset))) {
        return ERR_CONFLICT;
    }

    /* read from write set */

    ssize_t len2 = iooptab_read(ofdtx->wrtab,
                                ofdtx->wrtablen, buf, nbyte,
                                ofdtx->offset,
                                ofdtx->wrbuf);

    if (len2 < 0) {
        return len2;
    }

    ssize_t res = llmax(len, len2);

    /* register read data for later validation */
    if ((err = ofdtx_append_to_readset(ofdtx, res, ofdtx->offset, NULL)) < 0) {
        return err;
    }

    /* update file pointer */
    ofdtx->offset += res;
    ofdtx->flags  |= OFDTX_FL_LOCALBUF|OFDTX_FL_LOCALSTATE|OFDTX_FL_TL_INCVER;

    return res;
}

static ssize_t
ofdtx_read_exec_regular_2pl(struct ofdtx *ofdtx, int fildes,
                                                 void *buf,
                                                 size_t nbyte,
                                                 int *cookie,
                                                 enum picotm_libc_validation_mode val_mode)
{
    int err;

    assert(ofdtx);

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* write-lock open file description, because we change the file position */

    if ((err = ofd_wrlock_state(ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
        return err;
    }

    /* read-lock region */

    if ((err = ofdtx_2pl_lock_region(ofdtx, nbyte, ofdtx->offset, 0)) < 0) {
        return err;
    }

    /* read from file */

    ssize_t len = TEMP_FAILURE_RETRY(pread(fildes, buf, nbyte, ofdtx->offset));

    if (len < 0) {
        return len;
    }

    /* read from local write set */

    ssize_t len2 = iooptab_read(ofdtx->wrtab,
                                ofdtx->wrtablen, buf, nbyte,
                                ofdtx->offset,
                                ofdtx->wrbuf);

    if (len2 < 0) {
        return len2;
    }

    ssize_t res = llmax(len, len2);

    /* update file pointer */
    ofdtx->offset += res;

    return res;
}

ssize_t
ofdtx_read_exec(struct ofdtx *ofdtx, int fildes, void *buf,
                                           size_t nbyte,
                                           int *cookie, int noundo,
                                           enum picotm_libc_validation_mode val_mode)
{
    static ssize_t (* const read_exec[][4])(struct ofdtx*,
                                            int,
                                            void*,
                                            size_t,
                                            int*,
                                            enum picotm_libc_validation_mode) = {
        {ofdtx_read_exec_noundo, NULL,                       NULL,                        NULL},
        {ofdtx_read_exec_noundo, ofdtx_read_exec_regular_ts, ofdtx_read_exec_regular_2pl, NULL},
        {ofdtx_read_exec_noundo, NULL,                       NULL,                        NULL},
        {ofdtx_read_exec_noundo, NULL,                       NULL,                        NULL}};

    assert(ofdtx->type < sizeof(read_exec)/sizeof(read_exec[0]));
    assert(read_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !read_exec[ofdtx->type][ofdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return read_exec[ofdtx->type][ofdtx->cc_mode](ofdtx, fildes, buf, nbyte, cookie, val_mode);
}

static ssize_t
ofdtx_read_apply_noundo(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    return 0;
}

static ssize_t
ofdtx_read_apply_regular(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    int err = 0;

    while (n && !err) {
        ofdtab[ofdtx->ofd].data.regular.offset += ofdtx->rdtab[event->cookie].nbyte;

        lseek(fildes, ofdtab[ofdtx->ofd].data.regular.offset, SEEK_SET);

        --n;
        ++event;
    }

    return err;
}

static ssize_t
ofdtx_read_apply_socket(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    return 0;
}

ssize_t
ofdtx_read_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    static ssize_t (* const read_apply[][4])(struct ofdtx*, int, const struct com_fd_event*, size_t) = {
        {ofdtx_read_apply_noundo, NULL,                     NULL,                     NULL},
        {ofdtx_read_apply_noundo, ofdtx_read_apply_regular, ofdtx_read_apply_regular, NULL},
        {ofdtx_read_apply_noundo, NULL,                     NULL,                     NULL},
        {ofdtx_read_apply_noundo, NULL,                     NULL,                     ofdtx_read_apply_socket}};

    assert(ofdtx->type < sizeof(read_apply)/sizeof(read_apply[0]));
    assert(read_apply[ofdtx->type]);

    return read_apply[ofdtx->type][ofdtx->cc_mode](ofdtx, fildes, event, n);
}

static off_t
ofdtx_read_any_undo(void)
{
    return 0;
}

off_t
ofdtx_read_undo(struct ofdtx *ofdtx, int fildes, int cookie)
{
    static off_t (* const read_undo[][4])(void) = {
        {NULL, NULL,                NULL,                NULL},
        {NULL, ofdtx_read_any_undo, ofdtx_read_any_undo, NULL},
        {NULL, NULL,                NULL,                NULL},
        {NULL, NULL,                NULL,                ofdtx_read_any_undo}};

    assert(ofdtx->type < sizeof(read_undo)/sizeof(read_undo[0]));
    assert(read_undo[ofdtx->type]);

    return read_undo[ofdtx->type][ofdtx->cc_mode]();
}

/*
 * recv()
 */

static ssize_t
ofdtx_recv_exec_noundo(struct ofdtx *ofdtx, int sockfd,
                                            void *buffer,
                                            size_t length,
                                            int flags,
                                            int *cookie)
{
    return TEMP_FAILURE_RETRY(recv(sockfd, buffer, length, flags));
}

ssize_t
ofdtx_recv_exec(struct ofdtx *ofdtx, int sockfd,
                                     void *buffer,
                                     size_t length,
                                     int flags,
                                     int *cookie, int noundo)
{
    static ssize_t (* const recv_exec[][4])(struct ofdtx*,
                                            int,
                                            void*,
                                            size_t,
                                            int,
                                            int*) = {
        {ofdtx_recv_exec_noundo, NULL, NULL, NULL},
        {ofdtx_recv_exec_noundo, NULL, NULL, NULL},
        {ofdtx_recv_exec_noundo, NULL, NULL, NULL},
        {ofdtx_recv_exec_noundo, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(recv_exec)/sizeof(recv_exec[0]));
    assert(recv_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !recv_exec[ofdtx->type][ofdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return recv_exec[ofdtx->type][ofdtx->cc_mode](ofdtx, sockfd, buffer, length, flags, cookie);
}

static ssize_t
ofdtx_recv_apply_noundo(void)
{
    return 0;
}

int
ofdtx_recv_apply(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    static ssize_t (* const recv_apply[][4])(void) = {
        {ofdtx_recv_apply_noundo, NULL, NULL, NULL},
        {ofdtx_recv_apply_noundo, NULL, NULL, NULL},
        {ofdtx_recv_apply_noundo, NULL, NULL, NULL},
        {ofdtx_recv_apply_noundo, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(recv_apply)/sizeof(recv_apply[0]));
    assert(recv_apply[ofdtx->type]);

    return recv_apply[ofdtx->type][ofdtx->cc_mode]();
}

int
ofdtx_recv_undo(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    static int (* const recv_undo[][4])(struct ofdtx *, int, int) = {
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(recv_undo)/sizeof(recv_undo[0]));
    assert(recv_undo[ofdtx->type]);

    return recv_undo[ofdtx->type][ofdtx->cc_mode](ofdtx, sockfd, cookie);
}

/*
 * send()
 */

static ssize_t
ofdtx_send_exec_noundo(struct ofdtx *ofdtx, int sockfd,
                                      const void *buffer,
                                            size_t length,
                                            int flags,
                                            int *cookie)
{
    return TEMP_FAILURE_RETRY(send(sockfd, buffer, length, flags));
}
static ssize_t
ofdtx_send_exec_socket_ts(struct ofdtx *ofdtx, int sockfd,
                                         const void *buf,
                                               size_t nbyte,
                                               int flags,
                                               int *cookie)
{
    /* Become irrevocable if any flags are selected */
    if (flags) {
        return ERR_NOUNDO;
    }

    /* Register write data */

    if (cookie) {

        if ((*cookie = ofdtx_append_to_writeset(ofdtx, nbyte, 0, buf)) < 0) {
            return *cookie;
        }
    }

    return nbyte;
}

static ssize_t
ofdtx_send_exec_socket_2pl(struct ofdtx *ofdtx, int sockfd,
                                          const void *buf,
                                                size_t nbyte,
                                                int flags,
                                                int *cookie)
{
    int err;

    /* Become irrevocable if any flags are selected */
    if (flags) {
        return ERR_NOUNDO;
    }

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* Write-lock open file description, because we change the file position */

    if ((err = ofd_wrlock_state(ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
        return err;
    }

    /* Register write data */

    if (cookie) {

        if ((*cookie = ofdtx_append_to_writeset(ofdtx, nbyte, 0, buf)) < 0) {
            return *cookie;
        }
    }

    return nbyte;
}

ssize_t
ofdtx_send_exec(struct ofdtx *ofdtx, int sockfd,
                               const void *buffer,
                                     size_t length,
                                     int flags,
                                     int *cookie, int noundo)
{
    static ssize_t (* const send_exec[][4])(struct ofdtx*,
                                            int,
                                      const void*,
                                            size_t,
                                            int,
                                            int*) = {
        {ofdtx_send_exec_noundo, NULL,                      NULL,                       NULL},
        {ofdtx_send_exec_noundo, NULL,                      NULL,                       NULL},
        {ofdtx_send_exec_noundo, NULL,                      NULL,                       NULL},
        {ofdtx_send_exec_noundo, ofdtx_send_exec_socket_ts, ofdtx_send_exec_socket_2pl, NULL}};

    assert(ofdtx->type < sizeof(send_exec)/sizeof(send_exec[0]));
    assert(send_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !send_exec[ofdtx->type][ofdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return send_exec[ofdtx->type][ofdtx->cc_mode](ofdtx, sockfd, buffer, length, flags, cookie);
}

static int
ofdtx_send_apply_noundo(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    return 0;
}

static int
ofdtx_send_apply_socket_ts(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    assert(ofdtx);
    assert(sockfd >= 0);

    /* FIXME: Use select() to prevent blocking? */

    const ssize_t len =
        TEMP_FAILURE_RETRY(send(sockfd,
                                ofdtx->wrbuf+ofdtx->wrtab[cookie].bufoff,
                                ofdtx->wrtab[cookie].nbyte, 0));

    if (len < 0) {
        return ERR_SYSTEM;
    }

    return 0;
}

static int
ofdtx_send_apply_socket_2pl(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    assert(ofdtx);
    assert(sockfd >= 0);

    /* FIXME: Use select() to prevent blocking? */

    const ssize_t len =
        TEMP_FAILURE_RETRY(send(sockfd,
                                ofdtx->wrbuf+ofdtx->wrtab[cookie].bufoff,
                                ofdtx->wrtab[cookie].nbyte, 0));

    if (len < 0) {
        return ERR_SYSTEM;
    }

    return 0;
}

static int
ofdtx_send_apply_socket_2pl_ext(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    return 0;
}

int
ofdtx_send_apply(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    static int (* const send_apply[][4])(struct ofdtx*, int, int) = {
        {ofdtx_send_apply_noundo, NULL,                       NULL,                        NULL},
        {ofdtx_send_apply_noundo, NULL,                       NULL,                        NULL},
        {ofdtx_send_apply_noundo, NULL,                       NULL,                        NULL},
        {ofdtx_send_apply_noundo, ofdtx_send_apply_socket_ts, ofdtx_send_apply_socket_2pl, ofdtx_send_apply_socket_2pl_ext}};

    assert(ofdtx->type < sizeof(send_apply)/sizeof(send_apply[0]));
    assert(send_apply[ofdtx->type][ofdtx->cc_mode]);

    int err = 0;

    while (n && !err) {
        err = send_apply[ofdtx->type][ofdtx->cc_mode](ofdtx, sockfd, event->cookie);
        --n;
        ++event;
    }

    return err;
}

static int
ofdtx_send_undo_socket_ts(void)
{
    return 0;
}

static int
ofdtx_send_undo_socket_2pl(void)
{
    return 0;
}

static int
ofdtx_send_undo_socket_2pl_ext(void)
{
    return 0;
}

int
ofdtx_send_undo(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    static int (* const send_undo[][4])(void) = {
        {NULL, NULL,                      NULL,                       NULL},
        {NULL, NULL,                      NULL,                       NULL},
        {NULL, NULL,                      NULL,                       NULL},
        {NULL, ofdtx_send_undo_socket_ts, ofdtx_send_undo_socket_2pl, ofdtx_send_undo_socket_2pl_ext}};

    assert(ofdtx->type < sizeof(send_undo)/sizeof(send_undo[0]));
    assert(send_undo[ofdtx->type][ofdtx->cc_mode]);

    return send_undo[ofdtx->type][ofdtx->cc_mode]();
}

/*
 * shutdown()
 */

static int
ofdtx_shutdown_exec_noundo(struct ofdtx *ofdtx, int sockfd, int how,
                                                            int *cookie)
{
    return shutdown(sockfd, how);
}

int
ofdtx_shutdown_exec(struct ofdtx *ofdtx, int sockfd,
                                         int how,
                                         int *cookie,
                                         int noundo)
{
    static int (* const shutdown_exec[][4])(struct ofdtx*,
                                            int,
                                            int,
                                            int*) = {
        {ofdtx_shutdown_exec_noundo, NULL, NULL, NULL},
        {ofdtx_shutdown_exec_noundo, NULL, NULL, NULL},
        {ofdtx_shutdown_exec_noundo, NULL, NULL, NULL},
        {ofdtx_shutdown_exec_noundo, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(shutdown_exec)/sizeof(shutdown_exec[0]));
    assert(shutdown_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !shutdown_exec[ofdtx->type][ofdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return shutdown_exec[ofdtx->type][ofdtx->cc_mode](ofdtx, sockfd, how, cookie);
}

static int
ofdtx_shutdown_apply_noundo(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    return 0;
}

int
ofdtx_shutdown_apply(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    static int (* const shutdown_apply[][4])(struct ofdtx*,
                                             int,
                                             const struct com_fd_event*, size_t) = {
        {ofdtx_shutdown_apply_noundo, NULL, NULL, NULL},
        {ofdtx_shutdown_apply_noundo, NULL, NULL, NULL},
        {ofdtx_shutdown_apply_noundo, NULL, NULL, NULL},
        {ofdtx_shutdown_apply_noundo, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(shutdown_apply)/sizeof(shutdown_apply[0]));
    assert(shutdown_apply[ofdtx->type]);

    return shutdown_apply[ofdtx->type][ofdtx->cc_mode](ofdtx, sockfd, event, n);
}

int
ofdtx_shutdown_undo(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    static int (* const shutdown_undo[][4])(struct ofdtx*, int, int) = {
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(shutdown_undo)/sizeof(shutdown_undo[0]));
    assert(shutdown_undo[ofdtx->type]);

    return shutdown_undo[ofdtx->type][ofdtx->cc_mode](ofdtx, sockfd, cookie);
}

/*
 * write()
 */

static ssize_t
ofdtx_write_exec_noundo(struct ofdtx *ofdtx, int fildes,
                                       const void *buf,
                                             size_t nbyte,
                                             int *cookie)
{
    return TEMP_FAILURE_RETRY(write(fildes, buf, nbyte));
}

static ssize_t
ofdtx_write_exec_regular_ts(struct ofdtx *ofdtx, int fildes, const void *buf,
                                                 size_t nbyte, int *cookie)
{
    assert(ofdtx);

    /* register write data */

    if (__builtin_expect(!!cookie, 1)) {

        /* add data to write set */
        if ((*cookie = ofdtx_append_to_writeset(ofdtx,
                                                nbyte,
                                                ofdtx->offset, buf)) < 0) {
            return *cookie;
        }
    }

    /* update file pointer */
    ofdtx->offset += nbyte;
    ofdtx->flags  |= OFDTX_FL_TL_INCVER;

    return nbyte;
}

static ssize_t
ofdtx_write_exec_regular_2pl(struct ofdtx *ofdtx, int fildes, const void *buf,
                                                   size_t nbyte,
                                                   int *cookie)
{
    /* register written data */

    if (__builtin_expect(!!cookie, 1)) {

        int err;

        struct ofd *ofd = ofdtab+ofdtx->ofd;

        /* write-lock open file description, because we change the file position */

        if ((err = ofd_wrlock_state(ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
            return err;
        }

        /* write-lock region */

        if ((err = ofdtx_2pl_lock_region(ofdtx, nbyte, ofdtx->offset, 1)) < 0) {
            return err;
        }

        /* add buf to write set */
        if ((*cookie = ofdtx_append_to_writeset(ofdtx,
                                                nbyte,
                                                ofdtx->offset, buf)) < 0) {
            return *cookie;
        }
    }

    /* update file pointer */
    ofdtx->offset += nbyte;

    return nbyte;
}

static ssize_t
ofdtx_write_exec_fifo_ts(struct ofdtx *ofdtx, int fildes, const void *buf,
                                              size_t nbyte, int *cookie)
{
    /* Register write data */

    if (cookie) {

        if ((*cookie = ofdtx_append_to_writeset(ofdtx, nbyte, 0, buf)) < 0) {
            return *cookie;
        }
    }

    return nbyte;
}

static ssize_t
ofdtx_write_exec_fifo_2pl(struct ofdtx *ofdtx, int fildes, const void *buf,
                                               size_t nbyte,
                                               int *cookie)
{
    int err;

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* Write-lock open file description, because we change the file position */

    if ((err = ofd_wrlock_state(ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
        return err;
    }

    /* Register write data */

    if (cookie) {
        if ((*cookie = ofdtx_append_to_writeset(ofdtx, nbyte, 0, buf)) < 0) {
            return *cookie;
        }
    }

    return nbyte;
}

static ssize_t
ofdtx_write_exec_socket_ts(struct ofdtx *ofdtx, int fildes, const void *buf,
                                                size_t nbyte, int *cookie)
{
    /* Register write data */

    if (cookie) {
        if ((*cookie = ofdtx_append_to_writeset(ofdtx, nbyte, 0, buf)) < 0) {
            return *cookie;
        }
    }

    return nbyte;
}

static ssize_t
ofdtx_write_exec_socket_2pl(struct ofdtx *ofdtx, int fildes, const void *buf,
                                                 size_t nbyte,
                                                 int *cookie)
{
    int err;

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* Write-lock open file description, because we change the file position */

    if ((err = ofd_wrlock_state(ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
        return err;
    }

    /* Register write data */

    if (cookie) {

        if ((*cookie = ofdtx_append_to_writeset(ofdtx, nbyte, 0, buf)) < 0) {
            return *cookie;
        }
    }

    return nbyte;
}

ssize_t
ofdtx_write_exec(struct ofdtx *ofdtx, int fildes, const void *buf,
                                      size_t nbyte, int *cookie, int noundo)
{
    static ssize_t (* const write_exec[][4])(struct ofdtx*,
                                             int,
                                             const void*,
                                             size_t,
                                             int*) = {
        {ofdtx_write_exec_noundo, NULL,                        NULL,                         NULL},
        {ofdtx_write_exec_noundo, ofdtx_write_exec_regular_ts, ofdtx_write_exec_regular_2pl, NULL},
        {ofdtx_write_exec_noundo, ofdtx_write_exec_fifo_ts,    ofdtx_write_exec_fifo_2pl,    NULL},
        {ofdtx_write_exec_noundo, ofdtx_write_exec_socket_ts,  ofdtx_write_exec_socket_2pl,  NULL}};

    assert(ofdtx->type < sizeof(write_exec)/sizeof(write_exec[0]));
    assert(write_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !write_exec[ofdtx->type][ofdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return write_exec[ofdtx->type][ofdtx->cc_mode](ofdtx, fildes, buf, nbyte, cookie);
}

static int
ofdtx_write_apply_noundo(struct ofdtx *ofdtx, int fildes, int cookie)
{
    return 0;
}

static int
ofdtx_write_apply_regular_ts(struct ofdtx *ofdtx, int fildes, int cookie)
{
    assert(ofdtx);
    assert(fildes >= 0);

    off_t offset;

    /* If no dependencies, use current global offset */
    if (ofdtx->flags&OFDTX_FL_LOCALSTATE) {
        offset = ofdtx->wrtab[cookie].off;
    } else {
        offset = ofd_get_offset_nolock(ofdtab+ofdtx->ofd);
    }

    /* FIXME: Use select() to prevent blocking? */

    const ssize_t len = TEMP_FAILURE_RETRY(pwrite(fildes, ofdtx->wrbuf+ofdtx->wrtab[cookie].bufoff,
                                                          ofdtx->wrtab[cookie].nbyte, offset));

    if (len < 0) {
        return ERR_SYSTEM;
    }

    /* Update file position */
    ofdtab[ofdtx->ofd].data.regular.offset = offset+len;

    lseek(fildes, ofdtab[ofdtx->ofd].data.regular.offset, SEEK_SET);

    return 0;
}

static int
ofdtx_write_apply_regular_2pl(struct ofdtx *ofdtx, int fildes, int cookie)
{
    assert(ofdtx);
    assert(fildes >= 0);

    /* FIXME: Use select() to prevent blocking? */

    const ssize_t len = TEMP_FAILURE_RETRY(pwrite(fildes, ofdtx->wrbuf+ofdtx->wrtab[cookie].bufoff,
                                                          ofdtx->wrtab[cookie].nbyte,
                                                          ofdtx->wrtab[cookie].off));

    if (len < 0) {
        return ERR_SYSTEM;
    }

    /* Update file position */
    ofdtab[ofdtx->ofd].data.regular.offset = ofdtx->wrtab[cookie].off+len;

    lseek(fildes, ofdtab[ofdtx->ofd].data.regular.offset, SEEK_SET);

    return 0;
}

static int
ofdtx_write_apply_fifo(struct ofdtx *ofdtx, int fildes, int cookie)
{
    assert(ofdtx);
    assert(fildes >= 0);

    /* FIXME: Use select() to prevent blocking? */

    const ssize_t len =
        TEMP_FAILURE_RETRY(write(fildes,
                                 ofdtx->wrbuf+ofdtx->wrtab[cookie].bufoff,
                                 ofdtx->wrtab[cookie].nbyte));

    if (len < 0) {
        return ERR_SYSTEM;
    }

    return 0;
}

static int
ofdtx_write_apply_socket(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    assert(ofdtx);
    assert(sockfd >= 0);

    /* FIXME: Use select() to prevent blocking? */

    const ssize_t len =
        TEMP_FAILURE_RETRY(write(sockfd,
                                 ofdtx->wrbuf+ofdtx->wrtab[cookie].bufoff,
                                 ofdtx->wrtab[cookie].nbyte));

    if (len < 0) {
        return ERR_SYSTEM;
    }

    return 0;
}

ssize_t
ofdtx_write_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    static int (* const write_apply[][4])(struct ofdtx*, int, int) = {
        {ofdtx_write_apply_noundo, NULL,                         NULL,                          NULL},
        {ofdtx_write_apply_noundo, ofdtx_write_apply_regular_ts, ofdtx_write_apply_regular_2pl, NULL},
        {ofdtx_write_apply_noundo, ofdtx_write_apply_fifo,       ofdtx_write_apply_fifo,        NULL},
        {ofdtx_write_apply_noundo, ofdtx_write_apply_socket,     ofdtx_write_apply_socket,      NULL}};

    assert(ofdtx->type < sizeof(write_apply)/sizeof(write_apply[0]));
    assert(write_apply[ofdtx->type][ofdtx->cc_mode]);

    int err = 0;

    while (n && !err) {
        err = write_apply[ofdtx->type][ofdtx->cc_mode](ofdtx, fildes, event->cookie);
        --n;
        ++event;
    }

    return err;
}

static int
ofdtx_write_any_undo(void)
{
    return 0;
}

int
ofdtx_write_undo(struct ofdtx *ofdtx, int fildes, int cookie)
{
    static int (* const write_undo[][4])(void) = {
        {NULL, NULL,                 NULL,                 NULL},
        {NULL, ofdtx_write_any_undo, ofdtx_write_any_undo, NULL},
        {NULL, ofdtx_write_any_undo, ofdtx_write_any_undo, NULL},
        {NULL, ofdtx_write_any_undo, ofdtx_write_any_undo, ofdtx_write_any_undo}};

    assert(ofdtx->type < sizeof(write_undo)/sizeof(write_undo[0]));
    assert(write_undo[ofdtx->type][ofdtx->cc_mode]);

    return write_undo[ofdtx->type][ofdtx->cc_mode]();
}
