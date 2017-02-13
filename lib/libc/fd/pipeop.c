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
#include <stdio.h>
#include <string.h>
#include "pipeop.h"

int
pipeop_init(struct pipeop *pipeop, int pipefd[2])
{
    assert(pipeop);

    memcpy(pipeop->pipefd, pipefd, sizeof(pipeop->pipefd));

    return 0;
}

void
pipeop_dump(struct pipeop *pipeop)
{
    fprintf(stderr, "pipeop %p %d %d", (void*)pipeop, pipeop->pipefd[0],
                                                      pipeop->pipefd[1]);
}

