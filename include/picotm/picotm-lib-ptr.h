/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "compiler.h"
#include <stddef.h>
#include <stdint.h>

/**
 * \ingroup group_modules
 * \file
 */

PICOTM_BEGIN_DECLS

/**
 * \brief Rounds an address downwards to the nearest address with the given
 *        alignment.
 * \param   addr    A memory address.
 * \param   algn    The alignment of the result address.
 * \returns The next lower address with the given alignment.
 */
static inline uintptr_t
picotm_address_floor(uintptr_t addr, size_t algn)
{
    return addr - (addr % algn);
}

/**
 * \brief Rounds an address upwards to the nearest address with the given
 *        alignment.
 * \param   addr    A memory address.
 * \param   algn    The alignment of the result address.
 * \returns The next higher address with the given alignment.
 */
static inline uintptr_t
picotm_address_ceil(uintptr_t addr, size_t algn)
{
    uintptr_t moda = addr % algn;
    return addr + (algn - moda) * !!moda;
}

/**
 * Returns the container of a data structure.
 * \param   ptr     A pointer to a data structure.
 * \param   type    The type of the container.
 * \param   member  The name of the container's member field.
 * \returns A pointer to the container data structure.
 */
#define picotm_containerof(ptr, type, member) \
    ((type*)(((unsigned char*)(ptr)) - offsetof(type, member)))

/**
 * \brief Rounds a memory location upwards to the nearest address
 *        with the given alignment.
 * \param   ptr     A pointer to a memory location.
 * \param   algn    The alignment of the result memory location.
 * \returns The next higher memory location with the given alignment.
 */
static inline void*
picotm_ptr_ceil(const void* ptr, size_t algn)
{
    return (void*)picotm_address_ceil((uintptr_t)ptr, algn);
}

/**
 * \brief Subtracts two memory locations and returns the number of
 *        raw bytes between them.
 * \param hi    The higher memory location.
 * \param lo    The lower memory location.
 * \returns The number of bytes between lo an hi.
 */
static inline ptrdiff_t
picotm_ptr_diff(const void* hi, const void* lo)
{
    return (const uint8_t*)hi - (const uint8_t*)lo;
}

/**
 * \brief Rounds a memory location downwards to the nearest address
 *        with the given alignment.
 * \param   ptr     A pointer to a memory location.
 * \param   algn    The alignment of the result memory location.
 * \returns The next lower memory location with the given alignment.
 */
static inline void*
picotm_ptr_floor(const void* ptr, size_t algn)
{
    return (void*)picotm_address_floor((uintptr_t)ptr, algn);
}

/**
 * \brief Returns the number of size and alignment bytes of a given type.
 * \param   __type  The type.
 * \returns The number of bytes to hold a value of the given type and
 *          its aligment bytes.
 */
#define picotm_sizeof_align(__type)         \
    (sizeof(__type) + _Alignof(__type) - 1)

PICOTM_END_DECLS
