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

#include "taputils.h"
#include <string.h>
#include "tap.h"

/*
 * TAP output with different levels of verbosity
 */

enum tap_verbosity g_tap_verbosity = TAP_ERROR;

void
tap_error_va(const char* msg, va_list ap)
{
    if (g_tap_verbosity < TAP_ERROR) {
        return;
    }
    tap_diag_va(msg, ap);
}

void
tap_info_va(const char* msg, va_list ap)
{
    if (g_tap_verbosity < TAP_INFO) {
        return;
    }
    tap_diag_va(msg, ap);
}

void
tap_debug_va(const char* msg, va_list ap)
{
    if (g_tap_verbosity < TAP_DEBUG) {
        return;
    }
    tap_diag_va(msg, ap);
}

void
tap_error(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    tap_error_va(msg, ap);
    va_end(ap);
}

void
tap_info(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    tap_info_va(msg, ap);
    va_end(ap);
}

void
tap_debug(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    tap_debug_va(msg, ap);
    va_end(ap);
}

/*
 * Error reporting
 */

void
tap_error_errno_at(const char* function, int errnum, const char* filename,
                   unsigned long line)
{
    char buf[100];

    tap_error("%s:%lu: %s failed: %d (%s)", filename, line, function,
              errnum, strerror_r(errnum, buf, sizeof(buf)));
}
