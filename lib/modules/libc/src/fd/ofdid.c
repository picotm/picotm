/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ofdid.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <picotm/picotm-error.h>
#include <stdio.h>
#include <sys/stat.h>

void
ofdid_init(struct ofdid *id, dev_t dev, ino_t ino, mode_t mode, int fl)
{
    assert(id);

    id->dev = dev;
    id->ino = ino;
    id->mode = mode;
    id->fl = fl;
}

void
ofdid_init_from_fildes(struct ofdid* id, int fildes,
                       struct picotm_error* error)
{
    assert(id);
    assert(fildes >= 0);

    struct stat buf;

    int res = fstat(fildes, &buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }

    id->dev  = buf.st_dev;
    id->ino  = buf.st_ino;
    id->mode = buf.st_mode;

    res = fcntl(fildes, F_GETFL);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }

    id->fl = res;
}

void
ofdid_clear(struct ofdid *id)
{
    ofdid_init(id, (dev_t)-1, (ino_t)-1, 0, 0);
}

int
ofdid_is_empty(const struct ofdid *id)
{
    assert(id);

    return (id->dev  == (dev_t)-1) &&
           (id->ino  == (ino_t)-1) &&
           (id->mode == (mode_t)0) &&
           (id->fl   == 0);
}

int
ofdidcmp(const struct ofdid *id1, const struct ofdid *id2)
{
    const long long ddev = id1->dev-id2->dev;

    if (ddev < 0) {
        return -1;
    } else if (ddev > 0) {
        return 1;
    }

    const long long dino = id1->ino-id2->ino;

    if (dino < 0) {
        return -1;
    } else if (dino > 0) {
        return 1;
    }

    const long dmode = id1->mode-id2->mode;

    if (dmode < 0) {
        return -1;
    } else if (dmode > 0) {
        return 1;
    }

    if (S_ISFIFO(id1->mode)) {

        /* Distinguish between FIFO's read and write end */

        const long dfl = id1->fl-id2->fl;

        if (dfl < 0) {
            return -1;
        } else if (dfl > 0) {
            return 1;
        }
    }

    return 0;
}

void
ofdid_print(const struct ofdid *id)
{
    assert(id);

    fprintf(stderr, "ofdid={%d %d %d %d}\n", (int)id->dev, (int)id->ino, (int)id->mode, (int)id->fl);
}

