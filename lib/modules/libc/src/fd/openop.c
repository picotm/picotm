/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "openop.h"
#include <assert.h>

void
openop_init(struct openop *openop, int unlink)
{
    assert(openop);

    openop->unlink = unlink;
}

#include <stdio.h>

void
openop_dump(struct openop *openop)
{
    fprintf(stderr, "openop %p %d\n", (void*)openop, (int)openop->unlink);
}

