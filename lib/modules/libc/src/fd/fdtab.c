/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "fdtab.h"
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-tab.h>
#include <stdlib.h>

struct fd fdtab[MAXNUMFD];

/* Initializer */

static void fdtab_init(void) __attribute__((constructor));

static size_t
fdtab_fd_init_walk(void *fd, struct picotm_error* error)
{
    fd_init(fd, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static void
fdtab_init(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(fdtab, sizeof(fdtab)/sizeof(fdtab[0]), sizeof(fdtab[0]),
                     fdtab_fd_init_walk, &error);
    if (picotm_error_is_set(&error)) {
        abort();
    }
}

/* End of initalizer */

/* Destructor */

static void fdtab_uninit(void) __attribute__((destructor));

static size_t
fdtab_fd_uninit_walk(void* fd, struct picotm_error* error)
{
    fd_uninit(fd);
    return 1;
}

static void
fdtab_uninit(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(fdtab, sizeof(fdtab)/sizeof(fdtab[0]), sizeof(fdtab[0]),
                     fdtab_fd_uninit_walk, &error);
    if (picotm_error_is_set(&error)) {
        abort();
    }
}

/* End of destructor */

#include "fdtab.h"

