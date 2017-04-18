/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include "compiler.h"

PICOTM_BEGIN_DECLS

/**
 * Marks a value as TM value; required for load/store ops.
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_TM_ADDRESS(__value) ((uintptr_t)(__value))

PICOTM_NOTHROW
/**
 * Loads the memory at address into buffer.
 * \warning This is an internal interface. Don't use it in application code.
 */
void
__picotm_tm_load(uintptr_t addr, void* buf, size_t siz);

/**
 * Loads the memory at address into buffer.
 */
static inline void
load_tx(const void* addr, void* buf, size_t siz)
{
    __picotm_tm_load(__PICOTM_TM_ADDRESS(addr), buf, siz);
}

/**
 * Loads a pointer into buffer.
 */
static inline void*
load_ptr_tx(const void* addr)
{
    void* ptr;
    load_tx(addr, &ptr, sizeof(ptr));
    return ptr;
}

/**
 * Defines a C function for conveniently loading a value of a specifc type.
 */
#define PICOTM_TM_LOAD_TX(__name, __type)               \
    static inline __type                                \
    load_ ## __name ## _tx(const __type* addr)          \
    {                                                   \
        __type value;                                   \
        load_tx(addr, &value, sizeof(value));           \
        return value;                                   \
    }

PICOTM_TM_LOAD_TX(bool, _Bool)
PICOTM_TM_LOAD_TX(char, char)
PICOTM_TM_LOAD_TX(schar, signed char)
PICOTM_TM_LOAD_TX(uchar, unsigned char)
PICOTM_TM_LOAD_TX(short, short)
PICOTM_TM_LOAD_TX(ushort, unsigned short)
PICOTM_TM_LOAD_TX(int, int)
PICOTM_TM_LOAD_TX(uint, unsigned int)
PICOTM_TM_LOAD_TX(long, long)
PICOTM_TM_LOAD_TX(ulong, unsigned long)
PICOTM_TM_LOAD_TX(llong, long long)
PICOTM_TM_LOAD_TX(ullong, unsigned long long)
PICOTM_TM_LOAD_TX(float, float)
PICOTM_TM_LOAD_TX(double, double)

PICOTM_NOTHROW
/**
 * Stores the buffer at address in memory.
 * \warning This is an internal interface. Don't use it in application code.
 */
void
__picotm_tm_store(uintptr_t addr, const void* buf, size_t siz);

/**
 * Stores the buffer at address in memory.
 */
static inline void
store_tx(void* addr, const void* buf, size_t siz)
{
    __picotm_tm_store(__PICOTM_TM_ADDRESS(addr), buf, siz);
}

/**
 * Stores the pointer in memory.
 */
static inline void
store_ptr_tx(void* addr, const void* ptr)
{
    store_tx(addr, &ptr, sizeof(ptr));
}

/**
 * Defines a C function for conveniently storing a value of a specifc type.
 */
#define PICOTM_TM_STORE_TX(__name, __type)              \
    static inline void                                  \
    store_ ## __name ## _tx(__type* addr, __type value) \
    {                                                   \
        store_tx(addr, &value, sizeof(value));          \
    }

PICOTM_TM_STORE_TX(bool, _Bool)
PICOTM_TM_STORE_TX(char, char)
PICOTM_TM_STORE_TX(schar, signed char)
PICOTM_TM_STORE_TX(uchar, unsigned char)
PICOTM_TM_STORE_TX(short, short)
PICOTM_TM_STORE_TX(ushort, unsigned short)
PICOTM_TM_STORE_TX(int, int)
PICOTM_TM_STORE_TX(uint, unsigned int)
PICOTM_TM_STORE_TX(long, long)
PICOTM_TM_STORE_TX(ulong, unsigned long)
PICOTM_TM_STORE_TX(llong, long long)
PICOTM_TM_STORE_TX(ullong, unsigned long long)
PICOTM_TM_STORE_TX(float, float)
PICOTM_TM_STORE_TX(double, double)

PICOTM_NOTHROW
/**
 * Copies data between memory regions.
 * \warning This is an internal interface. Don't use it in application code.
 */
void
__picotm_tm_loadstore(uintptr_t laddr, uintptr_t saddr, size_t siz);

/**
 * Copies data between memory regions.
 */
static inline void
loadstore_tx(const void* laddr, void* saddr, size_t siz)
{
    __picotm_tm_loadstore(__PICOTM_TM_ADDRESS(laddr),
                         __PICOTM_TM_ADDRESS(saddr), siz);
}

enum {
    PICOTM_TM_PRIVATIZE_LOAD = 1 << 0,
    PICOTM_TM_PRIVATIZE_STORE = 1 << 1
};

#define PICOTM_TM_PRIVATIZE_LOADSTORE    \
    (PICOTM_TM_PRIVATIZE_LOAD | PICOTM_TM_PRIVATIZE_STORE)

/**
 * Privatizes the memory region starting at address.
 * \warning This is an internal interface. Don't use it in application code.
 */
void
__picotm_tm_privatize(uintptr_t addr, size_t siz, unsigned long flags);

PICOTM_NOTHROW
/**
 * Privatizes the memory region starting at address, ending
 * at character 'c'.
 * \warning This is an internal interface. Don't use it in application code.
 */
void
__picotm_tm_privatize_c(uintptr_t addr, int c, unsigned long flags);

/**
 * Privatizes the memory region starting at address.
 */
static inline void
privatize_tx(const void* addr, size_t siz, unsigned long flags)
{
    __picotm_tm_privatize(__PICOTM_TM_ADDRESS(addr), siz, flags);
}

/**
 * Privatizes the memory region starting at address, ending
 * at character 'c'.
 */
static inline void
privatize_c_tx(const void* addr, int c, unsigned long flags)
{
    __picotm_tm_privatize_c(__PICOTM_TM_ADDRESS(addr), c, flags);
}

/**
 * Defines a C function for conveniently privatizing a
 * value of a specific type.
 */
#define PICOTM_TM_PRIVATIZE_TX(__name, __type)                          \
    static inline void                                                  \
    privatize_ ## __name ## _tx(const __type* ptr, unsigned long flags) \
    {                                                                   \
        privatize_tx(ptr, sizeof(*ptr), flags);                         \
    }

PICOTM_TM_PRIVATIZE_TX(bool, _Bool)
PICOTM_TM_PRIVATIZE_TX(char, char)
PICOTM_TM_PRIVATIZE_TX(schar, signed char)
PICOTM_TM_PRIVATIZE_TX(uchar, unsigned char)
PICOTM_TM_PRIVATIZE_TX(short, short)
PICOTM_TM_PRIVATIZE_TX(ushort, unsigned short)
PICOTM_TM_PRIVATIZE_TX(int, int)
PICOTM_TM_PRIVATIZE_TX(uint, unsigned int)
PICOTM_TM_PRIVATIZE_TX(long, long)
PICOTM_TM_PRIVATIZE_TX(ulong, unsigned long)
PICOTM_TM_PRIVATIZE_TX(llong, long long)
PICOTM_TM_PRIVATIZE_TX(ullong, unsigned long long)
PICOTM_TM_PRIVATIZE_TX(float, float)
PICOTM_TM_PRIVATIZE_TX(double, double)

PICOTM_END_DECLS
