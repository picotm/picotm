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

#pragma once

#include <unistd.h>

/**
 * \brief Returns the current working directory in newly allocated memory.
 *
 * This interface was taken from GNU.
 */
char*
__picotm_libc_get_current_dir_name(void);

#if !defined(HAVE_DECL_GET_CURRENT_DIR_NAME) || !HAVE_DECL_GET_CURRENT_DIR_NAME
#define get_current_dir_name    __picotm_libc_get_current_dir_name
#endif
