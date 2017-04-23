/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "errcode.h"
#include "comfs.h"

int
com_fs_exec_mkstemp(struct com_fs *data, char *pathname)
{
    assert(data);
    assert(pathname);

    /* Construct absolute pathname */

    char *abspath = com_fs_absolute_path(data, pathname);

    if (!abspath) {
        return -1;
    }

    /* Make call */

    int fildes = mkstemp(abspath);

    if (fildes < 0) {
        free(abspath);
        return -1;
    }

    /* Copy trailing filled XXXXXX back to pathname */
    memcpy(pathname+strlen(pathname)-6, abspath+strlen(abspath)-6, 6);

    free(abspath);

    if (com_fs_inject(data, ACTION_MKSTEMP, fildes) < 0) {
        return -1;
    }

    return fildes;
}

int
com_fs_apply_mkstemp(struct com_fs *data, int cookie)
{
    return 0;
}

/* Remove temporary file in a quite reliable, but Linux-only, way. This is
 * only possible because it is certain that the transaction created that file
 * initially. Note that there is still a race condition. An attacker could
 * replace the file at 'canonpath' while the process is between stat and
 * unlink.
 */
int
com_fs_undo_mkstemp(struct com_fs *data, int cookie)
{
    char path[64];

    sprintf(path, "/proc/self/fd/%d", cookie);

    char *canonpath = canonicalize_file_name(path);

    if (canonpath) {

        struct stat buf[2];

        if (fstat(cookie, buf+0) != -1
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

    return TEMP_FAILURE_RETRY(close(cookie));
}

