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

#include <assert.h>
#include "openop.h"

int
openop_init(struct openop *openop, int unlink)
{
    assert(openop);

    openop->unlink = unlink;

    return 0;
}

#include <stdio.h>

void
openop_dump(struct openop *openop)
{
    fprintf(stderr, "openop %p %d\n", (void*)openop, (int)openop->unlink);
}

