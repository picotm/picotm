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

#include <setjmp.h>
#include <stddef.h>

void
_safe_block_set_jmpbuf(sigjmp_buf* jmpbuf, sigjmp_buf** old_jmpbuf);

#define begin_safe_block(_aborted)                          \
    {                                                       \
        sigjmp_buf _jmpbuf;                                 \
        sigjmp_buf* _old_jmpbuf;                            \
        _safe_block_set_jmpbuf(&_jmpbuf, &_old_jmpbuf);     \
        int _jmpval = sigsetjmp(_jmpbuf, 1);                \
        _aborted = !!_jmpval;                               \
        if (!_aborted) {

#define end_safe_block                              \
        }                                           \
        _safe_block_set_jmpbuf(_old_jmpbuf, NULL);  \
    }

void
abort_safe_block(void);
