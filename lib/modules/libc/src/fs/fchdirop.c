/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <stdio.h>
#include "fchdirop.h"

int
fchdirop_init(struct fchdirop *fchdirop, int oldcwd, int newcwd)
{
    assert(fchdirop);

    fchdirop->oldcwd = oldcwd;
    fchdirop->newcwd = newcwd;

    return 0;
}

void
fchdirop_dump(struct fchdirop *fchdirop)
{
    fprintf(stderr, "seekop %p %d %d", (void*)fchdirop, fchdirop->oldcwd,
                                                        fchdirop->newcwd);
}

