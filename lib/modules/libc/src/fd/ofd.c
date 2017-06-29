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

#include "ofd.h"
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <picotm/picotm-error.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "rwstatemap.h"

#define RECSIZE (1ul << RECBITS)

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

void
ofd_init(struct ofd* ofd, struct picotm_error* error)
{
    assert(ofd);

    pthread_rwlockattr_t rwlockattr;
    int err = pthread_rwlockattr_init(&rwlockattr);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }

    err = pthread_rwlockattr_setkind_np(&rwlockattr,
                                        PTHREAD_RWLOCK_PREFER_WRITER_NP);
    if (err) {
        picotm_error_set_errno(error, err);
        goto err_pthread_rwlockattr_setkind_np;
    }

    err = pthread_rwlock_init(&ofd->lock, &rwlockattr);
    if (err) {
        picotm_error_set_errno(error, err);
        goto err_pthread_rwlock_init;
    }

    ofdid_clear(&ofd->id);

    picotm_ref_init(&ofd->ref, 0);

    ofd->flags = 0;
    ofd->type = PICOTM_LIBC_FILE_TYPE_OTHER;

    picotm_rwlock_init(&ofd->rwlock);

    ofd->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;

    /* Type specific data */

    ofd->data.regular.offset = 0;

    rwlockmap_init(&ofd->data.regular.rwlockmap, error);
    if (picotm_error_is_set(error)) {
        goto err_rwlockmap_init;
    }

    pthread_rwlockattr_destroy(&rwlockattr);

    return;

err_rwlockmap_init:
    rwlockmap_uninit(&ofd->data.regular.rwlockmap);
    picotm_rwlock_uninit(&ofd->rwlock);
err_pthread_rwlock_init:
err_pthread_rwlockattr_setkind_np:
    pthread_rwlockattr_destroy(&rwlockattr);
}

void
ofd_uninit(struct ofd *ofd)
{
    rwlockmap_uninit(&ofd->data.regular.rwlockmap);

    picotm_rwlock_uninit(&ofd->rwlock);

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

static void
ofd_setup_from_fildes(struct ofd *ofd, int fildes, struct picotm_error* error)
{
    assert(ofd);
    assert(fildes >= 0);

    /* Any setup code goes here */

    ofdid_init_from_fildes(&ofd->id, fildes, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    /* Set type */

	struct stat buf;
    int res = fstat(fildes, &buf);
	if (res < 0) {
        picotm_error_set_errno(error, errno);
		return;
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
        case PICOTM_LIBC_FILE_TYPE_REGULAR: {
            off_t res = lseek(fildes, 0, SEEK_CUR);
            if (res == (off_t)-1) {
                picotm_error_set_errno(error, errno);
                return;
            }
            ofd->data.regular.offset = res;
            break;
        }
        case PICOTM_LIBC_FILE_TYPE_FIFO:
        case PICOTM_LIBC_FILE_TYPE_OTHER:
        case PICOTM_LIBC_FILE_TYPE_SOCKET:
            break;
        default:
            abort();
    }

    ofd->flags = 0;
}

void
ofd_ref_or_set_up(struct ofd* ofd, int fildes, bool want_new,
                  bool unlink_file, struct picotm_error* error)
{
    assert(ofd);
    assert(fildes >= 0);

    ofd_wrlock(ofd);

    bool first_ref = picotm_ref_up(&ofd->ref);

    if (!first_ref && !want_new) {
        /* we got a set-up instance; signal success */
        goto unlock;
    }

    if (!first_ref && want_new) {
        picotm_error_set_conflicting(error, NULL);
        goto err_want_new;
    }

    /* Setup ofd for given file descriptor */
    ofd_setup_from_fildes(ofd, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_ofd_setup_from_fildes;
    }

    /* Set flags */
    ofd->flags = 0;
    if (unlink_file) {
        ofd->flags |= OFD_FL_UNLINK;
    }

unlock:
    ofd_unlock(ofd);

    return;

err_ofd_setup_from_fildes:
err_want_new:
    ofd_unlock(ofd);
    ofd_unref(ofd);
}

void
ofd_ref(struct ofd *ofd)
{
    picotm_ref_up(&ofd->ref);
}

void
ofd_ref_state(struct ofd* ofd,
              enum picotm_libc_file_type* type,
              enum picotm_libc_cc_mode* ccmode,
              off_t* offset)
{
    ofd_ref(ofd);

    ofd_rdlock(ofd);

    if (type) {
        *type = ofd_get_type_nolock(ofd);
    }
    if (ccmode) {
        *ccmode = ofd_get_ccmode_nolock(ofd);
    }
    if (offset) {
        *offset = ofd_get_offset_nolock(ofd);
    }

    ofd_unlock(ofd);
}

void
ofd_unref(struct ofd *ofd)
{
    assert(ofd);

    picotm_ref_down(&ofd->ref);

    /* FIXME: Do we have to clean up? */
}

/*
 * Pessimistic CC
 */

void
ofd_rdlock_state(struct ofd *ofd, enum rwstate *rwstate,
                 struct picotm_error* error)
{
    assert(ofd);
    assert(rwstate);

    if (!(*rwstate&RW_RDLOCK) && !(*rwstate&RW_WRLOCK)) {

        picotm_rwlock_try_rdlock(&ofd->rwlock, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        *rwstate = RW_RDLOCK;
    }
}

void
ofd_wrlock_state(struct ofd *ofd, enum rwstate *rwstate,
                 struct picotm_error* error)
{
    assert(ofd);
    assert(rwstate);

    if (!(*rwstate&RW_WRLOCK)) {

        picotm_rwlock_try_wrlock(&ofd->rwlock, (*rwstate) & RW_RDLOCK, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        *rwstate = RW_WRLOCK;
    }
}

void
ofd_rwunlock_state(struct ofd *ofd, enum rwstate *rwstate)
{
    assert(ofd);
    assert(rwstate);

    if (*rwstate & (RW_RDLOCK | RW_WRLOCK)) {
        picotm_rwlock_unlock(&ofd->rwlock);
        *rwstate &= ~(RW_RDLOCK | RW_WRLOCK);
    }
}

void
ofd_2pl_lock_region(struct ofd *ofd,
                    off_t off,
                    size_t nbyte,
                    int write,
                    struct rwstatemap *rwstatemap,
                    struct picotm_error* error)
{
    static bool (* const lock[])(struct rwstatemap*,
                                 unsigned long long,
                                 unsigned long long,
                                 struct rwlockmap*,
                                 struct picotm_error*) = {
        rwstatemap_rdlock,
        rwstatemap_wrlock
    };

    assert(ofd);

    lock[!!write](rwstatemap, reccount(nbyte), recoffset(off),
                  &ofd->data.regular.rwlockmap, error);

    if (picotm_error_is_set(error)) {
        return;
    }
}

void
ofd_2pl_unlock_region(struct ofd *ofd, off_t off,
                                       size_t nbyte,
                                       struct rwstatemap *rwstatemap)
{
    assert(ofd);

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    rwstatemap_unlock(rwstatemap,
                      reccount(nbyte),
                      recoffset(off),
                      &ofd->data.regular.rwlockmap,
                      &error);
    if (picotm_error_is_set(&error)) {
        abort();
    }
}

void
ofd_dump(const struct ofd *ofd)
{
    fprintf(stderr, "%p: %ld\n", (const void*)ofd, (long)ofd->flags);
}

