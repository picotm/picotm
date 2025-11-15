/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "safe_sys_time.h"
#include <errno.h>
#include "safeblk.h"
#include "taputils.h"

int
safe_gettimeofday(struct timeval* restrict tv, struct timezone* tz)
{
    int res = gettimeofday(tv, tz);
    if (res < 0) {
        tap_error_errno("gettimeofday()", errno);
        abort_safe_block();
    }
    return res;
}
