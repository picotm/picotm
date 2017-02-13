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

#ifndef CMAPSS_H
#define CMAPSS_H

struct cmapss
{
    struct pgtreess super;
};

int
cmapss_init(struct cmapss *cmapss);

void
cmapss_uninit(struct cmapss *cmapss);

void
cmapss_clear(struct cmapss *cmapss);

int
cmapss_get_region(struct cmapss *cmapss, size_t length,
                                         off_t offset, struct cmap *cmap);

int
cmapss_validate_region(struct cmapss *cmapss, size_t length,
                                              off_t offset, struct cmap *cmap);

int
cmapss_inc_region(struct cmapss *cmapss, size_t length,
                                         off_t offset, struct cmap *cmap);

int
cmapss_lock_region(struct cmapss *cmapss, size_t length,
                                          off_t offset, struct cmap *cmap);

int
cmapss_unlock_region(struct cmapss *cmapss, size_t length,
                                            off_t offset, struct cmap *cmap);

#endif

