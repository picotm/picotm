/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <tanger-stm.h>
#include <tanger-stm-internal.h>
#include <tanger-stm-std-errno.h>
#include <tanger-stm-std-stdlib.h>
#include <tanger-stm-std-string.h>
#include <tanger-stm-std-stdio.h>
#include <tanger-stm-std-unistd.h>
#include <tanger-stm-std-sched.h>
#include "fs_test.h"

void
tanger_stm_fs_test_1(unsigned int tid)
{
    char path[32];

    snprintf(path, sizeof(path), "/tmp/dir.%d", tid);

	tanger_begin();

    char *cwd = getcwd(NULL, 0);
    fprintf(stderr, "%d %s\n", tid, cwd);

    if (chdir(path) < 0) {
        perror("chdir");
    }

    sched_yield();

    int i = 0;

    while (i++ < 5) {

        cwd = getcwd(NULL, 0);
        fprintf(stderr, "%d %s\n", tid, cwd);

        sleep(1);
    }

	tanger_commit();
}

