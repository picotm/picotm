/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

