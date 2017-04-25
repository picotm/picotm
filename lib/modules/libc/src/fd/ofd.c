/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ofd.h"
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "cmapss.h"
#include "errcode.h"
#include "rwstatemap.h"

#define bitsof(_x)  (sizeof(_x)*CHAR_BIT)

static const unsigned long flag_bits = OFD_FL_LAST_BIT+4;

static void
ofd_set_type(struct ofd *ofd, enum picotm_libc_file_type type)
{
	assert(ofd);

	ofd->type = type;

    const unsigned long fshift = bitsof(ofd->flags)-flag_bits;

    ofd->flags = ((ofd->flags << fshift) >> fshift) |
                   ((unsigned long)type << flag_bits);
}

static void
ofd_set_ccmode(struct ofd *ofd, enum picotm_libc_cc_mode cc_mode)
{
    assert(ofd);

    ofd->cc_mode = cc_mode;
}

static off_t
recoffset(off_t off)
{
    return off/RECSIZE;
}

static size_t
reccount(size_t len)
{
    return 1 + len/RECSIZE;
}

int
ofd_init(struct ofd *ofd)
{
    int err;
    pthread_rwlockattr_t rwlockattr;

    assert(ofd);

    if ((err = pthread_rwlockattr_init(&rwlockattr))) {
        errno = err;
        return ERR_SYSTEM;
    }

    pthread_rwlockattr_setkind_np(&rwlockattr, PTHREAD_RWLOCK_PREFER_WRITER_NP);

    if ((err = pthread_rwlock_init(&ofd->lock, &rwlockattr))) {
        errno = err;
        return ERR_SYSTEM;
    }

    pthread_rwlockattr_destroy(&rwlockattr);

    ofdid_clear(&ofd->id);

    if ((err = counter_init(&ofd->ref)) < 0) {
        pthread_rwlock_destroy(&ofd->lock);
        return err;
    }

    ofd->flags = 0;
    ofd->type = PICOTM_LIBC_FILE_TYPE_OTHER;

    if ((err = counter_init(&ofd->ver)) < 0) {
        counter_uninit(&ofd->ref);
        pthread_rwlock_destroy(&ofd->lock);
        return err;
    }

    if ((err = rwlock_init(&ofd->rwlock)) < 0) {
        counter_uninit(&ofd->ver);
        counter_uninit(&ofd->ref);
        pthread_rwlock_destroy(&ofd->lock);
        return err;
    }

    ofd->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;

    /* Type specific data */

    ofd->data.regular.offset = 0;

    if ((err = cmap_init(&ofd->data.regular.cmap)) < 0) {
        return err;
    }
    if ((err = rwlockmap_init(&ofd->data.regular.rwlockmap)) < 0) {
        return err;
    }

    return 0;
}

void
ofd_uninit(struct ofd *ofd)
{
    cmap_uninit(&ofd->data.regular.cmap);

    rwlockmap_uninit(&ofd->data.regular.rwlockmap);

    rwlock_uninit(&ofd->rwlock);

    counter_uninit(&ofd->ver);

    counter_uninit(&ofd->ref);

    pthread_rwlock_destroy(&ofd->lock);
}

void
ofd_set_id(struct ofd *ofd, const struct ofdid *id)
{
    assert(ofd);
    assert(id);

    memcpy(&ofd->id, id, sizeof(ofd->id));
}

void
ofd_clear_id(struct ofd *ofd)
{
    assert(ofd);

    ofdid_clear(&ofd->id);
}

void
ofd_rdlock(struct ofd *ofd)
{
    assert(ofd);

    pthread_rwlock_rdlock(&ofd->lock);
}

void
ofd_wrlock(struct ofd *ofd)
{
    assert(ofd);

    pthread_rwlock_wrlock(&ofd->lock);
}

void
ofd_unlock(struct ofd *ofd)
{
    assert(ofd);

    pthread_rwlock_unlock(&ofd->lock);
}

enum picotm_libc_cc_mode
ofd_get_ccmode_nolock(const struct ofd *ofd)
{
    return ofd->cc_mode;
}

enum picotm_libc_file_type
ofd_get_type_nolock(const struct ofd *ofd)
{
	return ofd->type;
}

off_t
ofd_get_offset_nolock(const struct ofd *ofd)
{
    assert(ofd);

    off_t pos;

    switch (ofd_get_type_nolock(ofd)) {
        case PICOTM_LIBC_FILE_TYPE_REGULAR:
            pos = ofd->data.regular.offset;
            break;
        case PICOTM_LIBC_FILE_TYPE_OTHER:
        case PICOTM_LIBC_FILE_TYPE_FIFO:
        case PICOTM_LIBC_FILE_TYPE_SOCKET:
            pos = 0;
            break;
        default:
            abort();
    }

    return pos;
}

/*
 * Referencing
 */

static int
ofd_setup_from_fildes(struct ofd *ofd, int fildes)
{
    assert(ofd);
    assert(fildes >= 0);

    /* Any setup code goes here */

    if (ofdid_init_from_fildes(&ofd->id, fildes) < 0) {
        return -1;
    }

    /* Set type */

	struct stat buf;

	if (fstat(fildes, &buf) < 0) {
		perror("fstat");
		return ERR_SYSTEM;
	}

    if (S_ISREG(buf.st_mode)) {
        ofd_set_type(ofd, PICOTM_LIBC_FILE_TYPE_REGULAR);
    } else if (S_ISFIFO(buf.st_mode)){
        ofd_set_type(ofd, PICOTM_LIBC_FILE_TYPE_FIFO);
    } else if (S_ISSOCK(buf.st_mode)){
        ofd_set_type(ofd, PICOTM_LIBC_FILE_TYPE_SOCKET);
    } else {
        ofd_set_type(ofd, PICOTM_LIBC_FILE_TYPE_OTHER);
    }

    ofd_set_ccmode(ofd, picotm_libc_get_file_type_cc_mode(ofd_get_type_nolock(ofd)));

    switch (ofd_get_type_nolock(ofd)) {
        case PICOTM_LIBC_FILE_TYPE_REGULAR:
            {
                ofd->data.regular.offset = lseek(fildes, 0, SEEK_CUR);
                if (ofd->data.regular.offset == (off_t)-1) {
                    perror("lseek");
                    return ERR_SYSTEM;
                }
            }
            break;
        case PICOTM_LIBC_FILE_TYPE_FIFO:
        case PICOTM_LIBC_FILE_TYPE_OTHER:
        case PICOTM_LIBC_FILE_TYPE_SOCKET:
            break;
        default:
            abort();
    }

    ofd->flags = 0;

    return 0;
}

static int
ofd_ref_setup(struct ofd *ofd, int fildes, unsigned long flags)
{
    int err;

    assert(ofd);
    assert(fildes >= 0);

    /* Setup ofd for given file descriptor */
    if ((err = ofd_setup_from_fildes(ofd, fildes)) < 0) {
        return err;
    }

    /* Set flags */
    ofd->flags = flags & OFD_FL_UNLINK;

    return 0;
}

static int
ofd_ref_check(struct ofd *ofd, int fildes, unsigned long flags)
{
    assert(ofd);
    assert(fildes >= 0);

    if (flags&OFD_FL_WANTNEW) {
        return ERR_CONFLICT;
    }

    return 0;
}

count_type
ofd_ref_state(struct ofd *ofd, int fildes, unsigned long flags,
              enum picotm_libc_file_type *type,
              enum picotm_libc_cc_mode *ccmode,
              off_t *offset)
{
    static int (* const ref[])(struct ofd*, int, unsigned long) = {
        ofd_ref_setup,
        ofd_ref_check};

    assert(ofd);

    ofd_wrlock(ofd);

    int err = ref[!!counter_get(&ofd->ref)](ofd, fildes, flags);

    if (!err) {

        counter_inc(&ofd->ref);

        if (type) {
            *type = ofd_get_type_nolock(ofd);
        }
        if (ccmode) {
            *ccmode = ofd_get_ccmode_nolock(ofd);
        }
        if (offset) {
            *offset = ofd_get_offset_nolock(ofd);
        }
    }

    ofd_unlock(ofd);

    return err;
}

count_type
ofd_ref(struct ofd *ofd, int fildes, unsigned long flags)
{
    return ofd_ref_state(ofd, fildes, flags, NULL, NULL, NULL);
}

void
ofd_unref(struct ofd *ofd)
{
    assert(ofd);

    counter_dec(&ofd->ref);
}

/*
 * Optimistic CC
 */

count_type
ofd_ts_get_state_version(struct ofd *ofd)
{
    assert(ofd);
    assert(ofd_get_ccmode_nolock(ofd) == PICOTM_LIBC_CC_MODE_TS);

    return counter_get(&ofd->ver);
}

int
ofd_ts_get_region_versions(struct ofd *ofd, size_t nbyte, off_t offset, struct cmapss *cmapss)
{
    assert(ofd);
    assert(ofd_get_ccmode_nolock(ofd) == PICOTM_LIBC_CC_MODE_TS);
    assert(ofd_get_type_nolock(ofd) == PICOTM_LIBC_FILE_TYPE_REGULAR);
    assert(cmapss);

    return cmapss_get_region(cmapss, reccount(nbyte),
                                     recoffset(offset),
                                    &ofd->data.regular.cmap);
}

int
ofd_ts_validate_state(struct ofd *ofd, count_type ver)
{
    assert(ofd);
    assert(ofd_get_ccmode_nolock(ofd) == PICOTM_LIBC_CC_MODE_TS);

    int res;

    count_type ofdver = counter_get(&ofd->ver);

    if (ver == ofdver) {
        res = 0;
    } else if (ver < ofdver) {
        res = ERR_CONFLICT;
    } else {
        abort();
    }

    return res;
}

int
ofd_ts_validate_region(struct ofd *ofd, size_t nbyte, off_t offset, struct cmapss *cmapss)
{
    assert(ofd);
    assert(ofd_get_ccmode_nolock(ofd) == PICOTM_LIBC_CC_MODE_TS);
    assert(ofd_get_type_nolock(ofd) == PICOTM_LIBC_FILE_TYPE_REGULAR);
    assert(cmapss);

    return cmapss_validate_region(cmapss,
                                  reccount(nbyte),
                                  recoffset(offset),
                                 &ofd->data.regular.cmap);
}

long long
ofd_ts_inc_state_version(struct ofd *ofd)
{
    assert(ofd);
    assert(ofd_get_ccmode_nolock(ofd) == PICOTM_LIBC_CC_MODE_TS);

    return counter_inc(&ofd->ver);
}

int
ofd_ts_inc_region_versions(struct ofd *ofd, size_t nbyte, off_t offset, struct cmapss *cmapss)
{
    int res;

    assert(ofd);
    assert(ofd_get_ccmode_nolock(ofd) == PICOTM_LIBC_CC_MODE_TS);

    switch (ofd_get_type_nolock(ofd)) {
        case PICOTM_LIBC_FILE_TYPE_REGULAR:
            res = cmapss_inc_region(cmapss,
                                    reccount(nbyte),
                                    recoffset(offset),
                                   &ofd->data.regular.cmap);
            break;
        case PICOTM_LIBC_FILE_TYPE_OTHER:
        case PICOTM_LIBC_FILE_TYPE_FIFO:
        case PICOTM_LIBC_FILE_TYPE_SOCKET:
            res = 0;
            break;
        default:
            abort();
    }

    return res;
}

int
ofd_ts_lock_region(struct ofd *ofd, size_t nbyte, off_t offset, struct cmapss *cmapss)
{
    int res;

    assert(ofd);
    assert(ofd_get_ccmode_nolock(ofd) == PICOTM_LIBC_CC_MODE_TS);

    switch (ofd_get_type_nolock(ofd)) {
        case PICOTM_LIBC_FILE_TYPE_REGULAR:
            res = cmapss_lock_region(cmapss,
                                     reccount(nbyte),
                                     recoffset(offset),
                                    &ofd->data.regular.cmap);
            break;
        case PICOTM_LIBC_FILE_TYPE_OTHER:
        case PICOTM_LIBC_FILE_TYPE_FIFO:
        case PICOTM_LIBC_FILE_TYPE_SOCKET:
            res = 0;
            break;
        default:
            abort();
    }

    return res;
}

int
ofd_ts_unlock_region(struct ofd *ofd, size_t nbyte, off_t offset, struct cmapss *cmapss)
{
    int res;

    assert(ofd);
    assert(ofd_get_ccmode_nolock(ofd) == PICOTM_LIBC_CC_MODE_TS);

    switch (ofd_get_type_nolock(ofd)) {
        case PICOTM_LIBC_FILE_TYPE_REGULAR:
            res = cmapss_unlock_region(cmapss,
                                       reccount(nbyte),
                                       recoffset(offset),
                                      &ofd->data.regular.cmap);
            break;
        case PICOTM_LIBC_FILE_TYPE_OTHER:
        case PICOTM_LIBC_FILE_TYPE_FIFO:
        case PICOTM_LIBC_FILE_TYPE_SOCKET:
            res = 0;
            break;
        default:
            abort();
    }

    return res;
}

/*
 * Pessimistic CC
 */

int
ofd_rdlock_state(struct ofd *ofd, enum rwstate *rwstate)
{
    assert(ofd);
    assert(rwstate);

    if (!(*rwstate&RW_RDLOCK) && !(*rwstate&RW_WRLOCK)) {
        int err = rwlock_rdlock(&ofd->rwlock, *rwstate&RW_WRLOCK);

        if (err < 0) {
            return err;
        }
        *rwstate = RW_RDLOCK;
    }


    return 0;
}

int
ofd_wrlock_state(struct ofd *ofd, enum rwstate *rwstate)
{
    assert(ofd);
    assert(rwstate);

    if (!(*rwstate&RW_WRLOCK)) {
        int err = rwlock_wrlock(&ofd->rwlock, *rwstate&RW_RDLOCK);

        if (err < 0) {
            return err;
        }
        *rwstate = RW_WRLOCK;
    }

    return 0;
}

void
ofd_rwunlock_state(struct ofd *ofd, enum rwstate *rwstate)
{
    assert(ofd);
    assert(rwstate);

    if (*rwstate&RW_RDLOCK) {
        rwlock_rdunlock(&ofd->rwlock);
        *rwstate &= ~RW_RDLOCK;
    }

    if (*rwstate&RW_WRLOCK) {
        rwlock_wrunlock(&ofd->rwlock);
        *rwstate &= ~RW_WRLOCK;
    }

}

int
ofd_2pl_lock_region(struct ofd *ofd,
                    off_t off,
                    size_t nbyte,
                    int write,
                    struct rwstatemap *rwstatemap)
{
    static int (* const lock[])(struct rwstatemap*,
                                unsigned long long,
                                unsigned long long,
                                struct rwlockmap*) = {
        rwstatemap_rdlock,
        rwstatemap_wrlock};

    assert(ofd);

    return lock[!!write](rwstatemap,
                         reccount(nbyte),
                         recoffset(off),
                        &ofd->data.regular.rwlockmap);
}

void
ofd_2pl_unlock_region(struct ofd *ofd, off_t off,
                                       size_t nbyte,
                                       struct rwstatemap *rwstatemap)
{
    assert(ofd);

    if (rwstatemap_unlock(rwstatemap,
                          reccount(nbyte),
                          recoffset(off),
                         &ofd->data.regular.rwlockmap) < 0) {
        abort();
    }
}

void
ofd_dump(const struct ofd *ofd)
{
    fprintf(stderr, "%p: %ld\n", (const void*)ofd, (long)ofd->flags);
}

