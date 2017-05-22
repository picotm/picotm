/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OFD_H
#define OFD_H

#include "counter.h"
#include "ofdid.h"
#include "picotm/picotm-libc.h"
#include "rwlock.h"
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
struct rwstatemap;

struct ofd
{
    pthread_rwlock_t lock;

    struct ofdid   id;
    struct counter ref;

    unsigned long flags;
    enum picotm_libc_file_type type;

    enum picotm_libc_cc_mode cc_mode;
    struct rwlock  rwlock; /**< \brief Lock, pessimistic CC */

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
int
ofd_init(struct ofd *ofd);

/** \brief Uninitializes the open file description. */
void
ofd_uninit(struct ofd *ofd);

void
ofd_set_id(struct ofd *ofd, const struct ofdid *id);

void
ofd_clear_id(struct ofd *ofd);

/** \brief References the open file description. */
count_type
ofd_ref(struct ofd *ofd, int fildes, unsigned long flags);

/** \brief References the open file description and returns its state. */
count_type
ofd_ref_state(struct ofd *ofd, int fildes, unsigned long flags,
              enum picotm_libc_file_type *type,
              enum picotm_libc_cc_mode *ccmode,
              off_t *offset);

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
int
ofd_rdlock_state(struct ofd *ofd, enum rwstate *rwstate);

/** \brief Locks the open file description for writing. */
int
ofd_wrlock_state(struct ofd *ofd, enum rwstate *rwstate);

/** \brief Unlocks the open file description. */
void
ofd_rwunlock_state(struct ofd *ofd, enum rwstate *rwstate);

/** \brief Locks a region of the underlying file buffer. */
int
ofd_2pl_lock_region(struct ofd *ofd, off_t off,
                                     size_t nbyte,
                                     int write,
                                     struct rwstatemap *rwstatemap);

/** \brief Unlocks the underlying file buffer. */
void
ofd_2pl_unlock_region(struct ofd *ofd, off_t off,
                                       size_t nbyte,
                                       struct rwstatemap *rwstatemap);

#endif

