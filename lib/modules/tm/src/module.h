/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * \cond impl || tm_impl
 * \ingroup tm_impl
 * \file
 * \endcond
 */

void
tm_module_load(uintptr_t addr, void* buf, size_t siz);

void
tm_module_store(uintptr_t addr, const void* buf, size_t siz);

void
tm_module_loadstore(uintptr_t laddr, uintptr_t saddr, size_t siz);

void
tm_module_privatize(uintptr_t addr, size_t siz, unsigned long flags);

void
tm_module_privatize_c(uintptr_t addr, int c, unsigned long flags);
