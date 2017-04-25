/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "pipeop.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

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

