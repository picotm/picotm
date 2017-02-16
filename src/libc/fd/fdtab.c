/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include "types.h"
#include "counter.h"
#include "table.h"
#include "fcntlop.h"
#include "fd.h"

struct fd fdtab[MAXNUMFD];

/* Initializer */

static void fdtab_init(void) __attribute__((constructor));

static int
fdtab_fd_init_walk(void *fd)
{
    return fd_init(fd) < 0 ? -1 : 1;
}

static void
fdtab_init()
{
    if (tabwalk_1(fdtab,
                  sizeof(fdtab)/sizeof(fdtab[0]), sizeof(fdtab[0]),
                  fdtab_fd_init_walk) < 0) {
        abort();
    }
}

/* End of initalizer */

/* Destructor */

static void fdtab_uninit(void) __attribute__((destructor));

static int
fdtab_fd_uninit_walk(void *fd)
{
    fd_uninit(fd);
    return 1;
}

static void
fdtab_uninit()
{
    if (tabwalk_1(fdtab,
                  sizeof(fdtab)/sizeof(fdtab[0]), sizeof(fdtab[0]),
                  fdtab_fd_uninit_walk) < 0) {
        abort();
    }
}

/* End of destructor */

#include "fdtab.h"

