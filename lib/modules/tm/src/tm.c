/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
