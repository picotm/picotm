/*
 * picotm - A system-level transaction manager
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

#include "picotm/sched.h"
#include "picotm/picotm-module.h"
#include <errno.h>
#include "error/module.h"

#if defined(PICOTM_LIBC_HAVE_SCHED_YIELD) && PICOTM_LIBC_HAVE_SCHED_YIELD
PICOTM_EXPORT
int
sched_yield_tx()
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        error_module_save_errno(&error);
        if (!picotm_error_is_set(&error)) {
            break;
        }
        picotm_recover_from_error(&error);
    } while (true);

    int res;

    do {
        res = sched_yield();
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
#endif
