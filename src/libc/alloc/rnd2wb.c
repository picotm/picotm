/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <sys/types.h>
#include "rnd2wb.h"

size_t
rnd2wb(size_t size)
{
    const unsigned int mask = sizeof(void*)-1;

    return (size + mask) & ~mask;
}

