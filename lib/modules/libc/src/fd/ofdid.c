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

    file_id_init(&id->fbuf_id, dev, ino);

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

    res = fcntl(fildes, F_GETFL);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
    int fl = res;

    ofdid_init(id, buf.st_dev, buf.st_ino, buf.st_mode, fl);
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

    return file_id_is_empty(&id->fbuf_id) &&
           (id->mode == (mode_t)0) &&
           (id->fl   == 0);
}

int
ofdidcmp(const struct ofdid *id1, const struct ofdid *id2)
{
    int diff_fbuf_id = file_id_cmp(&id1->fbuf_id, &id2->fbuf_id);

    if (diff_fbuf_id) {
        return diff_fbuf_id;
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

    fprintf(stderr, "ofdid={%d %d %d %d}\n", (int)id->fbuf_id.dev,
                                             (int)id->fbuf_id.ino,
                                             (int)id->mode,
                                             (int)id->fl);
}

