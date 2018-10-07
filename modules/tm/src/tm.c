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

#include "picotm/picotm-tm.h"
#include "module.h"

PICOTM_EXPORT
void
__picotm_tm_load(uintptr_t addr, void* buf, size_t siz)
{
    tm_module_load(addr, buf, siz);
}

PICOTM_EXPORT
void
__picotm_tm_store(uintptr_t addr, const void* buf, size_t siz)
{
    tm_module_store(addr, buf, siz);
}

PICOTM_EXPORT
void
__picotm_tm_loadstore(uintptr_t laddr, uintptr_t saddr, size_t siz)
{
    tm_module_loadstore(laddr, saddr, siz);
}

PICOTM_EXPORT
void
__picotm_tm_privatize(uintptr_t addr, size_t siz, unsigned long flags)
{
    tm_module_privatize(addr, siz, flags);
}

PICOTM_EXPORT
void
__picotm_tm_privatize_c(uintptr_t addr, int c, unsigned long flags)
{
    tm_module_privatize_c(addr, c, flags);
}
