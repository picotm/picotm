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

#ifndef OFD_H
#define OFD_H

#include <picotm/picotm-lib-ref.h>
#include <picotm/picotm-lib-rwlock.h>
#include <stdatomic.h>
#include "ofdid.h"
#include "picotm/picotm-libc.h"
#include "rwlockmap.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

#define OFD_FL_UNLINK   (1<<0)
#define OFD_FL_WANTNEW  (1<<2)
#define OFD_FL_LAST_BIT (2)

struct picotm_error;
struct picotm_rwstate;
struct rwcountermap;

struct ofd
{
    struct picotm_shared_ref16 ref;

    pthread_rwlock_t lock;

    struct ofdid   id;

    unsigned long flags;
    enum picotm_libc_file_type type;

    enum picotm_libc_cc_mode cc_mode;
    struct picotm_rwlock  rwlock; /**< \brief Lock, pessimistic CC */

    struct {
        struct {
            /** \brief global file position */
            off_t                offset;
            /** \brief Lock table, pessimistic CC */
            struct rwlockmap     rwlockmap;
        } regular;
    } data;
};

/** \brief Initializes the open file description. */
void
ofd_init(struct ofd *ofd, struct picotm_error* error);

/** \brief Uninitializes the open file description. */
void
ofd_uninit(struct ofd *ofd);

void
ofd_set_id(struct ofd *ofd, const struct ofdid *id);

void
ofd_clear_id(struct ofd *ofd);

/** \brief References the open file description. */
void
ofd_ref_or_set_up(struct ofd* ofd, int fildes, bool want_new,
                  bool unlink_file, struct picotm_error* error);

/** \brief References the open file description. */
void
ofd_ref(struct ofd* ofd);

/** \brief References the open file description and returns its state. */
void
ofd_ref_state(struct ofd* ofd,
              enum picotm_libc_file_type* type,
              enum picotm_libc_cc_mode* ccmode,
              off_t* offset);

/**
 * \brief Compares the ofd's id to an id and acquires a reference if both
 *        id's are equal.
 * \param   ofd An ofd structure.
 * \param   id  The id to compare to.
 * \returns A value less than, equal to, or greater than if the ofd's id is
 *          less than, equal to, or greater than the given id.
 */
int
ofd_cmp_and_ref(struct ofd* ofd, const struct ofdid* id);

/**
 * \brief Compares the ofd's id to an id and acquires a reference if both
 *        id's are equal. The ofd structure is aet up from the provided
 *        file descriptor.
 * \param       ofd         An ofd structure.
 * \param       id          The id to compare to.
 * \param       fildes      A file descriptor refereing to the open file description.
 * \param       want_new    True to request a new instance.
 * \param       unlink_file True to request unlinking the file during rollback.
 * \param[out]  error       Returns an error.
 * \returns A value less than, equal to, or greater than if the ofd's id is
 *          less than, equal to, or greater than the given id.
 */
int
ofd_cmp_and_ref_or_set_up(struct ofd* ofd, const struct ofdid* id,
                          int fildes, bool want_new, bool unlink_file,
                          struct picotm_error* error);

/** \brief Unreferences the open file description. */
void
ofd_unref(struct ofd *ofd);

/** \brief Returns the current CC mode of the open file description. */
enum picotm_libc_cc_mode
ofd_get_ccmode_nolock(const struct ofd *ofd);

/** \brief Returns the type of the open file description. */
enum picotm_libc_file_type
ofd_get_type_nolock(const struct ofd *ofd);

/** \brief Returns the file offset of the open file description. */
off_t
ofd_get_offset_nolock(const struct ofd *ofd);

/** \brief Write the open file description to stderr. */
void
ofd_dump(const struct ofd *ofd);

void
ofd_rdlock(struct ofd *ofd);

void
ofd_wrlock(struct ofd *ofd);

void
ofd_unlock(struct ofd *ofd);

/*
 * Pessimistic CC
 */

/** \brief Locks the open file description for reading. */
void
ofd_rdlock_state(struct ofd *ofd, struct picotm_rwstate* rwstate,
                 struct picotm_error* error);

/** \brief Locks the open file description for writing. */
void
ofd_wrlock_state(struct ofd *ofd, struct picotm_rwstate* rwstate,
                 struct picotm_error* error);

/** \brief Unlocks the open file description. */
void
ofd_rwunlock_state(struct ofd *ofd, struct picotm_rwstate* rwstate);

/** \brief Locks a region of the underlying file buffer. */
void
ofd_2pl_lock_region(struct ofd *ofd, off_t off,
                                     size_t nbyte,
                                     int write,
                                     struct rwcountermap *rwcountermap,
                                     struct picotm_error* error);

/** \brief Unlocks the underlying file buffer. */
void
ofd_2pl_unlock_region(struct ofd *ofd, off_t off,
                                       size_t nbyte,
                                       struct rwcountermap *rwcountermap);

#endif

