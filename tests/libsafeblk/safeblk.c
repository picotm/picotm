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

#include "safeblk.h"
#include <assert.h>

static __thread sigjmp_buf* g_thread_jmpbuf;

void
_safe_block_set_jmpbuf(sigjmp_buf* jmpbuf, sigjmp_buf** old_jmpbuf)
{
    if (old_jmpbuf) {
        *old_jmpbuf = g_thread_jmpbuf;
    }
    g_thread_jmpbuf = jmpbuf;
}

void
abort_safe_block()
{
    assert(g_thread_jmpbuf);

    siglongjmp(*g_thread_jmpbuf, 1);
}
