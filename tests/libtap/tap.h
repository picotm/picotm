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

#include <stdarg.h>

void
tap_version(unsigned long version);

void
tap_plan(unsigned long n);

/*
 * Interfaces with variable number of arguments
 */

void
tap_ok(unsigned long testnum, const char* msg, ...);

void
tap_ok_todo(unsigned long testnum, const char* msg, ...);

void
tap_ok_skip(unsigned long testnum, const char* msg, ...);

void
tap_not_ok(unsigned long testnum, const char* msg, ...);

void
tap_not_ok_todo(unsigned long testnum, const char* msg, ...);

void
tap_not_ok_skip(unsigned long testnum, const char* msg, ...);

void
tap_bail_out(const char* desc, ...);

void
tap_diag(const char* msg, ...);

/*
 * Interfaces with va_list argument
 */

void
tap_ok_va(unsigned long testnum, const char* msg, va_list ap);

void
tap_ok_todo_va(unsigned long testnum, const char* msg, va_list ap);

void
tap_ok_skip_va(unsigned long testnum, const char* msg, va_list ap);

void
tap_not_ok_va(unsigned long testnum, const char* msg, va_list ap);

void
tap_not_ok_todo_va(unsigned long testnum, const char* msg, va_list ap);

void
tap_not_ok_skip_va(unsigned long testnum, const char* msg, va_list ap);

void
tap_bail_out_va(const char* desc, va_list ap);

void
tap_diag_va(const char* msg, va_list ap);
