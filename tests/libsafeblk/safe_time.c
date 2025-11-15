/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann
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

#include "safe_time.h"
#include <errno.h>
#include "safeblk.h"
#include "taputils.h"

struct tm*
safe_gmtime_r(const time_t* restrict timer, struct tm* restrict result)
{
    struct tm* res = gmtime_r(timer, result);
    if (!res) {
        tap_error_errno("gmtime_r()", errno);
        abort_safe_block();
    }
    return res;
}

time_t
safe_time(time_t* tloc)
{
    time_t res = time(tloc);
    if (res == (time_t)-1) {
        tap_error_errno("time()", errno);
        abort_safe_block();
    }
    return res;
}
