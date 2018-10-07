/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "get_current_dir_name.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

char*
__picotm_libc_get_current_dir_name()
{
    size_t siz = PATH_MAX;

    char* buf = NULL;

    do {
        void* mem = malloc(siz);
        if (!mem) {
            free(buf);
        }
        buf = mem;
        char* cwd = getcwd(buf, siz);
        if (cwd) {
            return cwd;
        }
        siz *= 2;
    } while (errno == ERANGE);

    free(buf);

    return NULL;
}
