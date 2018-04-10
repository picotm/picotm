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

#include "picotm/config/picotm-tm-config.h"
#include "picotm/compiler.h"
#include <stddef.h>
#include <stdint.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_tm
 * \file
 *
 * \brief Public interfaces of picotm's Transactional Memory module.
 */

/*
 * Marks a value as TM value; required for load/store ops.
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_TM_ADDRESS(__value) ((uintptr_t)(__value))

PICOTM_NOTHROW
/*
 * Loads the memory at address into buffer.
 * \warning This is an internal interface. Don't use it in application code.
 */
void
__picotm_tm_load(uintptr_t addr, void* buf, size_t siz);

/**
 * Loads the memory at address into a buffer.
 * \param   addr    The address to load from.
 * \param   buf     The transaction-local buffer to store the loaded value in.
 * \param   siz     The number of bytes to load.
 */
static inline void
load_tx(const void* addr, void* buf, size_t siz)
{
    __picotm_tm_load(__PICOTM_TM_ADDRESS(addr), buf, siz);
}

/**
 * Loads a pointer into a buffer.
 * \param   addr    The address to load from.
 * \returns The transaction-local pointer.
 */
static inline void*
load_ptr_tx(const void* addr)
{
    void* ptr;
    load_tx(addr, &ptr, sizeof(ptr));
    return ptr;
}

/**
 * Defines a C function for conveniently loading a value of a specific type
 * into a transaction. The helper function's name is load_<__name>_tx.
 * \param   __name  The name of the type.
 * \param   __type  The C type.
 */
#define PICOTM_TM_LOAD_TX(__name, __type)                                   \
    /**
        Loads a value of type '__type' with transactional semantics.
        \param   addr   The source address.
        \returns The transaction-local value loaded from address 'addr'.
     */                                                                     \
    static inline __type                                                    \
    load_ ## __name ## _tx(const __type* addr)                              \
    {                                                                       \
        __type value;                                                       \
        load_tx(addr, &value, sizeof(value));                               \
        return value;                                                       \
    }

PICOTM_NOTHROW
/*
 * Stores the buffer at address in memory.
 * \warning This is an internal interface. Don't use it in application code.
 */
void
__picotm_tm_store(uintptr_t addr, const void* buf, size_t siz);

/**
 * Stores the buffer at address in memory.
 * \param   addr    The address to store to.
 * \param   buf     The transaction-local buffer to load the loaded value from.
 * \param   siz     The number of bytes to store.
 */
static inline void
store_tx(void* addr, const void* buf, size_t siz)
{
    __picotm_tm_store(__PICOTM_TM_ADDRESS(addr), buf, siz);
}

/**
 * Stores the pointer in memory.
 * \param   addr    The address to store to.
 * \param   ptr     The pointer value to store.
 */
static inline void
store_ptr_tx(void* addr, const void* ptr)
{
    store_tx(addr, &ptr, sizeof(ptr));
}

/**
 * Defines a C function for conveniently storing a value of a specific type
 * into a transaction. The helper function's name is store_<__name>_tx.
 * \param   __name  The name of the type.
 * \param   __type  The C type.
 */
#define PICOTM_TM_STORE_TX(__name, __type)                                  \
    /**
        Stores a value of type '__type' with transactional semantics.
        \param   addr   The destination address.
        \param   value  The value to store at 'addr'.
     */                                                                     \
    static inline void                                                      \
    store_ ## __name ## _tx(__type* addr, __type value)                     \
    {                                                                       \
        store_tx(addr, &value, sizeof(value));                              \
    }

PICOTM_NOTHROW
/*
 * Copies data between memory regions.
 * \warning This is an internal interface. Don't use it in application code.
 */
void
__picotm_tm_loadstore(uintptr_t laddr, uintptr_t saddr, size_t siz);

/**
 * Copies data between non-transactional memory regions.
 * \param   laddr   The address of the source region.
 * \param   saddr   The address of the destination region.
 * \param   siz     The number of bytes ot copy.
 */
static inline void
loadstore_tx(const void* laddr, void* saddr, size_t siz)
{
    __picotm_tm_loadstore(__PICOTM_TM_ADDRESS(laddr),
                          __PICOTM_TM_ADDRESS(saddr), siz);
}

/** Flags for memory privatizations. */
enum {
    /** Privatizes a memory region for loading. */
    PICOTM_TM_PRIVATIZE_LOAD = 1 << 0,
    /** Privatizes a memory region for storing. */
    PICOTM_TM_PRIVATIZE_STORE = 1 << 1
};

/** Privatizes a memory region for loading and storing. */
#define PICOTM_TM_PRIVATIZE_LOADSTORE    \
    (PICOTM_TM_PRIVATIZE_LOAD | PICOTM_TM_PRIVATIZE_STORE)

/*
 * Privatizes the memory region starting at address.
 * \warning This is an internal interface. Don't use it in application code.
 */
void
__picotm_tm_privatize(uintptr_t addr, size_t siz, unsigned long flags);

PICOTM_NOTHROW
/*
 * Privatizes the memory region starting at address, ending
 * at character 'c'.
 * \warning This is an internal interface. Don't use it in application code.
 */
void
__picotm_tm_privatize_c(uintptr_t addr, int c, unsigned long flags);

/**
 * Privatizes the memory region starting at address.
 * \param   addr    The address to privatize.
 * \param   siz     The number of bytes to privatize.
 * \param   flags   Privatizes for loading and/or storing. Not setting flags
 *                  discards the buffer.
 */
static inline void
privatize_tx(const void* addr, size_t siz, unsigned long flags)
{
    __picotm_tm_privatize(__PICOTM_TM_ADDRESS(addr), siz, flags);
}

/**
 * Privatizes the memory region starting at address, ending
 * at character 'c'.
 * \param   addr    The address to privatize.
 * \param   c       The region's terminating character.
 * \param   flags   Privatizes for loading and/or storing. Not setting flags
 *                  discards the buffer.
 */
static inline void
privatize_c_tx(const void* addr, int c, unsigned long flags)
{
    __picotm_tm_privatize_c(__PICOTM_TM_ADDRESS(addr), c, flags);
}

/**
 * Defines a C function for conveniently privatizing a value of a specific
 * type into a transaction. The helper function's name is store_<__name>_tx.
 * \param   __name  The name of the type.
 * \param   __type  The C type.
 */
#define PICOTM_TM_PRIVATIZE_TX(__name, __type)                              \
    /**
        Privatizes a value of type '__type'.
        \param   addr   The address to privatize.\n
        \param   flags  Privatizes for loading and/or storing. Not setting
                        flags discards the buffer.
     */                                                                     \
    static inline void                                                      \
    privatize_ ## __name ## _tx(const __type* addr, unsigned long flags)    \
    {                                                                       \
        privatize_tx(addr, sizeof(*addr), flags);                           \
    }

PICOTM_END_DECLS

/**
 * \defgroup group_tm The Transactional Memory Module
 *
 * \brief The Transactional Memory module provides transactional semantics
 *        when accessing main memory. You can load and store variables in
 *        main memory, or adopt memory regions into a transactions.
 *
 * The Transactional Memory module provides load and store operations for
 * main memory. In order to avoid conflicting access to shared memory
 * locations, picotm has to know which transaction uses which locations. The
 * Transactional Memory module maintains these information.
 *
 * A call to `load_tx()` copies a memory location's value into a transaction,
 * as illustrated in the example below.
 *
 * ~~~{.c}
 *  int x;
 *
 *  picotm_begin
 *
 *      int x_tx;
 *      load_tx(&x, &x_tx, sizeof(x));
 *
 *      // The value of 'x' is now stored in 'x_tx'.
 *
 *  picotm_commit
 *  picotm_end
 * ~~~
 *
 * This copies the value of `x` into our transaction's variable `x_tx` and
 * puts the memory location of `x` under control of the transaction manager.
 * We are free to change the value of `x_tx` at will, since it's transaction
 * local. If we change it, the original non-transactional value in `x`
 * remains unchanged.
 *
 * In a similarly way we store a copy of a transactional variable in a
 * memory location. This is done with `store_tx()`.
 *
 * ~~~{.c}
 *  int x;
 *
 *  picotm_begin
 *
 *      int x_tx;
 *      store_tx(&x, &x_tx, sizeof(x));
 *
 *      // The value of 'x_tx' will be committed into 'x'.
 *
 *  picotm_commit
 *  picotm_end
 * ~~~
 *
 * As with all transactional modifications, the store will not be executed
 * immediately, but only become permanent after the transaction successfully
 * committed.
 *
 * We don't have to deal with all these addresses ourselves. The TM module
 * comes with helper functions for the basic C types. With `load_int_tx()`
 * and `store_int_tx()` we can rewrite the example transactions as shown
 * below.
 *
 * ~~~{.c}
 *  int x;
 *
 *  picotm_begin
 *
 *      int x_tx = load_int_tx(&x);
 *
 *      // The value of 'x' is now stored in 'x_tx'.
 *
 *      store_int_tx(&x, x_tx);
 *
 *      // The value of 'x_tx' will be committed into 'x'.
 *
 *  picotm_commit
 *  picotm_end
 * ~~~
 *
 * Besides `load_int_tx()` and `store_int_tx()`, the Transactional Memory
 * module provides similar functions for the basic C types. Each is defined
 * via the macros `PICOTM_TM_LOAD_TX()` and `PICOTM_TM_STORE_TX()`. We can
 * use these macros to define load and store functions for our application's
 * data types. Both macros expand to inline C functions, so we don't loose
 * performance compared to `load_tx()` and `store_tx()`.
 *
 * ~~~{.c}
 *  // An application-specific type
 *  typedef unsigned int    my_int_type;
 *
 *  // Define load_my_int_type_tx()
 *  PICOTM_TM_LOAD_TX(my_int_type, my_int_type);
 *
 *  // Define store_my_int_type_tx()
 *  PICOTM_TM_STORE_TX(my_int_type, my_int_type);
 * ~~~
 *
 * Since address and pointer handling can be tricky, there are also helpers
 * for loading and storing pointers. These functions load and store the
 * address stored in a pointer variable, but not the value stored at that
 * address.
 *
 * ~~~{.c}
 *  int* x_ptr;
 *
 *  picotm_begin
 *
 *      int* x_ptr_tx = load_ptr_tx(&x_ptr);
 *
 *      // The value of 'x_ptr' is now stored in 'x_ptr_tx'.
 *
 *      store_ptr_tx(&x_ptr, &x_ptr_tx);
 *
 *      // The value of 'x_ptr_tx' will be committed into 'x_ptr'.
 *
 *  picotm_commit
 *  picotm_end
 * ~~~
 *
 * Loads and stores always copy values into or out of a transaction. There
 * are cases where we don't want a copy, but the exact memory location
 * of a value. This is called *privatization.* For example, the function
 * `memcpy()` loads and stores an indefinite amount of data between memory
 * buffers. The actual buffer size is often not known in advance. It would
 * be wasteful to first load the data into a transaction-local buffer and
 * then further store it in another buffer.
 *
 * The functions `privatize_tx()` and `privatize_c_tx()` offer privatization
 * of memory locations.
 *
 * ~~~{.c}
 *  int x;
 *
 *  picotm_begin
 *
 *      privatize_tx(&x, sizeof(x), PICOTM_TM_PRIVATIZE_LOAD);
 *
 *      // The memory location of 'x' is now available within the transaction.
 *
 *      privatize_tx(&x, sizeof(x), PICOTM_TM_PRIVATIZE_STORE);
 *
 *      // Changes to 'x' will be committed into the memory location of 'x'.
 *
 *  picotm_commit
 *  picotm_end
 * ~~~
 *
 * This privatizes `x` for transactional access from within the transaction.
 * The number of bytes is given in the second argument. The flags argument is
 * a bitmask of `PICOTM_TM_PRIVATIZE_LOAD` and `PICOTM_TM_PRIVATIZE_STORE`.
 * These flags control how picotm handles the memory location. If we only
 * privatized a memory location for loading *or* storing, we may never
 * invoke the other operation. Setting no flags at all will discard the
 * memory location. This signals to other transactions that the memory is
 * invalid and to be freed.
 *
 * There can be cases where we don't know in advance how large in size the
 * privatized buffer is going to be. For example, if we privatize a C string,
 * the length is not explicitly stored in the string, but given by the
 * location of the terminating `\0` character. To privatize a memory region
 * up to and including a specific character, there is `privatize_c_tx()`.
 *
 * ~~~{.c}
 *  char* str = "foo";
 *
 *  picotm_begin
 *
 *      privatize_c_tx(str, '\0', PICOTM_TM_PRIVATIZE_LOAD);
 *
 *      // The string at 'str' is now available within the transaction.
 *
 *      privatize_c_tx(str, '\0', PICOTM_TM_PRIVATIZE_STORE);
 *
 *      // Changes to 'str' will be committed into the string at 'str'.
 *
 *  picotm_commit
 *  picotm_end
 * ~~~
 *
 * Using `privatize_c_tx()` with strings is the most common use case, but
 * arbitrary characters are possible.
 *
 * Finally, there is `loadstore_tx()`. Even through the name suggests
 * load-and-store, the function is a mixture of load, store and
 * privatization. It privatizes an input buffer for loading and an
 * output buffer for storing, and copies the input buffer's content
 * into the output buffer.
 *
 * ~~~{.c}
 *  int ibuf[20];
 *  int obuf[20];
 *
 *  picotm_begin
 *
 *      loadstore_tx(ibuf, obuf, sizeof(obuf));
 *
 *      // The data in 'ibuf' will be committed into the memory of 'obuf'.
 *
 *  picotm_commit
 *  picotm_end
 * ~~~
 *
 * A call to `loadstore_tx()` is like a call to `memcpy()` that privatizes
 * its input buffers. It's an optimization for platform without transactional
 * C library.
 */
