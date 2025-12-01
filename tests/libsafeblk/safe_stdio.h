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

#include <stdio.h>

/*
 * Interfaces with variable number of arguments
 */

int
safe_snprintf(char* str, size_t size, const char* format, ...);

int
safe_sscanf(const char* str, const char* format, ...);

/*
 * Interfaces with va_list argument
 */

int
safe_vsnprintf(char* str, size_t size, const char* format, va_list ap);

int
safe_vsscanf(const char* str, const char* format, va_list ap);
