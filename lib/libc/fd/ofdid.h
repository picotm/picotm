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

#ifndef OFDID_H
#define OFDID_H

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

