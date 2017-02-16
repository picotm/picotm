/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <tanger-stm-internal.h>
#include <tanger-stm-internal-errcode.h>
#include <tanger-stm-internal-extact.h>
#include <tanger-stm-ext-actions.h>
#include "types.h"
#include "seekop.h"
#include "rwlock.h"
#include "counter.h"
#include "pgtree.h"
#include "pgtreess.h"
#include "cmap.h"
#include "cmapss.h"
#include "rwlockmap.h"
#include "rwstatemap.h"
#include "openop.h"
#include "openoptab.h"
#include "fcntlop.h"
#include "ofdid.h"
#include "ofd.h"
#include "ofdtx.h"
#include "fd.h"
#include "fdtab.h"
#include "fdtx.h"
#include "comfd.h"
#include "fs/comfs.h"
#include "fs/comfstx.h"

#define DO_UNLINK(mode_) \
    ( ( (mode_)&(O_CREAT|O_EXCL) ) == (O_CREAT|O_EXCL) )

int
com_fd_exec_open(struct com_fd *data, const char *path, int oflag,
                                                        mode_t mode,
                                                        int isnoundo)
{
    struct com_fs *fsdata = com_fs_tx_aquire_data();
    assert(fsdata);

    /* O_TRUNC needs irrevocability */

    if ((mode&O_TRUNC) && !isnoundo) {
        return ERR_NOUNDO;
    }

    /* Open file */

    int fildes =
        TEMP_FAILURE_RETRY(openat(com_fs_get_cwd(fsdata), path, oflag, mode));

    if (fildes < 0) {
        return ERR_SYSTEM;
    }

    /* FIXME: Distinguish between open calls. Ofd only knows one file
              description, but each open creates a new open file description;
              File position might be wrong
              Ideas: Maybe introduce open index (0: outside of Tx,
              n>0 inside Tx), or maybe reset file position on commiting open */

    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    enum error_code err = fdtx_ref(fdtx, fildes, OFD_FL_WANTNEW);

    if (err) {
        if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
            perror("close");
        }
        return err;
    }

    int cookie = openoptab_append(&data->openoptab,
                                  &data->openoptablen, DO_UNLINK(mode));

    /* Inject event */
    if ((cookie >= 0) &&
        (com_fd_inject(data, ACTION_OPEN, fildes, cookie) < 0)) {
        if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
            perror("close");
        }
        return ERR_SYSTEM;
    }

    return fildes;
}

int
com_fd_apply_open(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    return 0;
}

int
com_fd_undo_open(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < MAXNUMFD);
    assert(cookie < data->openoptablen);

    if (data->openoptab[cookie].unlink) {

        char path[64];

        sprintf(path, "/proc/self/fd/%d", fildes);

        char *canonpath = canonicalize_file_name(path);

        if (canonpath) {

            struct stat buf[2];

            if (fstat(fildes, buf+0) != -1
                && stat(canonpath, buf+1) != -1
                && buf[0].st_dev == buf[1].st_dev
                && buf[0].st_ino == buf[1].st_ino) {

                if (unlink(canonpath) < 0) {
                    perror("unlink");
                }
            }

            free(canonpath);
        } else {
            perror("canonicalize_file_name");
        }
    }

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    /* Mark file descriptor to be closed */
    fdtx_signal_close(fdtx);

    return 0;
}

