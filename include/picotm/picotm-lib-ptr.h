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

#include "compiler.h"
#include <stddef.h>
#include <stdint.h>

/**
 * \ingroup group_lib
 * \file
 */

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_lib
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
 * \ingroup group_lib
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
 * \ingroup group_lib
 * Returns the container of a data structure.
 * \param   ptr     A pointer to a data structure.
 * \param   type    The type of the container.
 * \param   member  The name of the container's member field.
 * \returns A pointer to the container data structure.
 */
#define picotm_containerof(ptr, type, member) \
    ((type*)(((unsigned char*)(ptr)) - offsetof(type, member)))

/**
 * \ingroup group_lib
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
 * \ingroup group_lib
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
 * \ingroup group_lib
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
 * \ingroup group_lib
 * \brief Returns the number of size and alignment bytes of a given type.
 * \param   __type  The type.
 * \returns The number of bytes to hold a value of the given type and
 *          its aligment bytes.
 */
#define picotm_sizeof_align(__type)         \
    (sizeof(__type) + _Alignof(__type) - 1)

PICOTM_END_DECLS
