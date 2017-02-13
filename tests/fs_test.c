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

