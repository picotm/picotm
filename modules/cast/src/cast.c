/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "cast.h"
#include "picotm/picotm-module.h"
#include <errno.h>

static void
report_errno(int errno_code)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    picotm_error_set_errno(&error, errno_code);
    picotm_recover_from_error(&error);
}

PICOTM_EXPORT
void
__picotm_cast_error_underflow_tx()
{
    report_errno(ERANGE);
}

PICOTM_EXPORT
void
__picotm_cast_error_overflow_tx()
{
    report_errno(ERANGE);
}
