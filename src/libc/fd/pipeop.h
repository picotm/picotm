/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PIPEOP_H
#define PIPEOP_H

struct pipeop
{
    int pipefd[2];
};

int
pipeop_init(struct pipeop *pipeop, int pipefd[2]);

void
pipeop_dump(struct pipeop *pipeop);

#endif

