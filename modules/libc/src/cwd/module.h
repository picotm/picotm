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

#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * \cond impl || libc_impl || libc_impl_cwd
 * \ingroup libc_impl
 * \ingroup libc_impl_cwd
 * \file
 * \endcond
 */

struct picotm_error;

int
cwd_module_chdir(const char* path, struct picotm_error* error);

char*
cwd_module_getcwd(char* buf, size_t siz, struct picotm_error* error);

char*
cwd_module_realpath(const char* path, char* resolved_path,
                    struct picotm_error* error);
