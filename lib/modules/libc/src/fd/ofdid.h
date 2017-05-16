/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OFDID_H
#define OFDID_H

#include <sys/types.h>

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct ofdid
{
    dev_t  dev;
    ino_t  ino;
    mode_t mode;
    int    fl;
};

/** \brief Initialize id with values */
void
ofdid_init(struct ofdid *id, dev_t dev, ino_t ino, mode_t mode, int fl);

/** \brief initialize id from file descriptor */
int
ofdid_init_from_fildes(struct ofdid *id, int fildes);

/** \brief Clear file descriptor */
void
ofdid_clear(struct ofdid *id);

/** \brief Return non-zero value id id has been initialized */
int
ofdid_is_empty(const struct ofdid *id);

/** Compare two ids, return value as with strcmp */
int
ofdidcmp(const struct ofdid *id1, const struct ofdid *id2);

/** Dump id to stderr */
void
ofdid_print(const struct ofdid *id);

#endif

