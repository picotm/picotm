/*
 * MIT License
 * Copyright (c) 2018   Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "picotm/config/picotm-cast-config.h"
#include "picotm/compiler.h"

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_cast
 * \file
 *
 * \brief Public interfaces of picotm's type-casting module.
 */

PICOTM_NOTHROW
/**
 * Reports an overflow error to the transaction manager.
 * \warning This is an internal interface. Don't use it in application code.
 */
void
__picotm_cast_error_overflow_tx(void);

PICOTM_NOTHROW
/**
 * Reports an underflow error to the transaction manager.
 * \warning This is an internal interface. Don't use it in application code.
 */
void
__picotm_cast_error_underflow_tx(void);

/**
 * Defines a C function that casts a value from a source type to a
 * destination type.
 * \param   __sname The name of the source type.
 * \param   __stype The C type of the source value.
 * \param   __dname The name of the destination type.
 * \param   __dtype The C type of the destination value.
 * \param   __dmin  The minimum value of the destination type.
 * \param   __dmax  The maximum value of the destination type.
 */
#define PICOTM_CAST_TX(__sname, __stype, __dname, __dtype, __dmin, __dmax)  \
    /**
        Casts a value of type `__stype` to type `__dtype`. The function
        detects overflows and underflows of the destination type's range
        and reports them as errno code to the transaction's recovery phase.
        \param  value   The value.
        \returns The parameter's value casted to type `__dtype`.
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        addition's result overflows the type's range.
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        addition's result underflows the type's range.
     */                                                                     \
    static inline __dtype                                                   \
    cast_ ## __sname ## _to_ ## __dname ## _tx(__stype value)               \
    {                                                                       \
        /* Instanciating variables for min/max avoid compiler warnings. */  \
        const __dtype dmin = (__dmin);                                      \
        const __dtype dmax = (__dmax);                                      \
        const _Bool ssgn = (((__stype)-1) < 0);                             \
        const _Bool dsgn = (((__dtype)-1) < 0);                             \
        if (ssgn == dsgn) {                                                 \
            /* Source and destination types both have same signess. C       \
             * compilers convert both values to the larger type. */         \
            if (value < dmin) {                                             \
                __picotm_cast_error_underflow_tx();                         \
            } else if (value > dmax) {                                      \
                __picotm_cast_error_overflow_tx();                          \
            }                                                               \
        } else if (ssgn) {                                                  \
            /* Source type is signed; destination type is unsigned. Source  \
             * value might be bit-wise converted to 'unsigned' type during  \
             * compare operations! */                                       \
            if (value > 0) {                                                \
                if (value > dmax) {                                         \
                    __picotm_cast_error_overflow_tx();                      \
                } else if ((dmin > 0) && (value < dmin)) {                  \
                    /* Can only underflow in the rare case that the         \
                     * destination type's minimal value is larger than      \
                     * 0. */                                                \
                    __picotm_cast_error_underflow_tx();                     \
                }                                                           \
            } else if (!value) {                                            \
                if (dmin > 0) {                                             \
                    __picotm_cast_error_underflow_tx();                     \
                } else if (dmax < value) {                                  \
                    __picotm_cast_error_overflow_tx();                      \
                }                                                           \
            }  else /* (value < 0) */ {                                     \
                __picotm_cast_error_underflow_tx();                         \
            }                                                               \
        } else /* (dsgn) */ {                                               \
            /* Source type is unsigned; destination type is signed.         \
             * Destination minimal and maximal values might be bit-         \
             * wise converted to 'unsigned' type during compare             \
             * operations. */                                               \
            if (value > dmax) {                                             \
                __picotm_cast_error_overflow_tx();                          \
            } else if ((dmin > 0) && (value < dmin)) {                      \
                /* Can only underflow in the rare case that the             \
                 * destination type's minimal value is larger than          \
                 * 0. */                                                    \
                __picotm_cast_error_underflow_tx();                         \
            }                                                               \
        }                                                                   \
        return (__dtype)value;                                              \
    }

PICOTM_END_DECLS

/**
 * \defgroup group_cast The Type-Casting Module
 *
 * \brief The type-casting module provides safe casting between C types.
 *        Overflows or underflows in the destination type's range are
 *        reported as errors to the transaction.
 *
 * The type-casting module provides safe casting between C types. Type
 * casting is subject to overflows or underflows of the destination
 * type's range. For example, casting the value of *512* from 16-bit-wide
 * `short` to 8-bit-wide `signed char` overflows the range of `signed char`.
 * Similarly, casting from signed to unsigned types underflows the
 * destination type's range if the source value is negative.
 *
 * All type-casting functions are in the form of
 * `cast_<source type>_to_<destination type>_tx()`, where `<source type>` and
 * `<destination type>` are names for C types. Each function takes a value
 * of the source type and returns the same value casted to the destination
 * type.
 *
 * Picotm's type-casting module comes with pre-defined functions for converting
 * between all C-native integer types, and for converting from C-native integer
 * to floating-point types. Here's an example transaction that casts a value from
 * `long` to `int`.
 *
 * ~~~{.c}
 *  picotm_begin
 *      int dst = cast_long_to_int_tx(LONG_MAX);
 *  picotm_commit
 *      if ((picotm_error_status() == PICOTM_ERRNO) &&
 *          (picotm_error_as_errno() == ERANGE)) {
 *          // An error happened during type casting.
 *      }
 *  picotm_end
 * ~~~
 *
 * Depending on the system's architecture, type `long` is either
 * 32 bit of 64 bit in size. The example succeeds on 32-bit
 * systems but reports an error on 64-bit systems.
 *
 * The C preprocessor macro `PICOTM_CAST_TX()` creates a cast function for
 * a specific pair of types and limits. We can use it to define cast
 * functions for application-specific types. All pre-defined conversion
 * function are created with this macro, so casts for application types
 * share the same behaviour and features.
 *
 * For example, our application might define the floating-point type
 * `float100`, which is only defined from minus 100 to plus 100.
 *
 * ~~~{.c}
 *  typedef float float100;
 *
 *  #define FLT100_MAX  100     // similar to FLT_MAX from <float.h>
 * ~~~
 *
 * We can now create a function for casting from float to float100 using
 * `PICOTM_CAST_TX()`.
 *
 * ~~~{.c}
 *  PICOTM_CAST_TX(float, float, float100, float100, -FLT100_MAX, FLT100_MAX);
 * ~~~
 *
 * This macro expands to a function that casts from type `float` to type
 * `float100`.
 *
 * ~~~{.c}
 *  static inline float100
 *  cast_float_to_float100_tx(float value)
 *  {
 *      ... // safe type-casting code
 *  }
 * ~~~
 *
 * The return value of this function is of type `float100` and spans values
 * between `-FLT_MAX` and `FLT_MAX`, which has been defined to `100`. If the
 * parameter overflows or underflows this range, the function signals `ERANGE`
 * to the transaction's recovery code.
 *
 * Similarly, we can define a function for casting from `unsigned char` to
 * `float100`.
 *
 * ~~~{.c}
 *  PICOTM_CAST_TX(uchar, unsigned char, float100, float100, -FLT100_MAX, FLT100_MAX);
 * ~~~
 *
 * This expands to the function shown below. Note that we used *uchar* for
 * function names that involve `unsigned char`.
 *
 * ~~~{.c}
 *  static inline float100
 *  cast_uchar_to_float100_tx(unsigned char value)
 *  {
 *      ... // safe type-casting code
 *  }
 * ~~~
 *
 * We can also cast from `float100` to another type. The cast's minimum and
 * maximum values are always relative to the destination type. So for a cast
 * from `float100` to `unsigned char` we have to use `0` and `UCHAR_MAX`
 * as the limits.
 *
 * ~~~{.c}
 *  PICOTM_CAST_TX(float100, float100, uchar, unsigned char, 0, UCHAR_MAX);
 * ~~~
 *
 * Conversions from floating-point to integer types are not supported
 * by the type-casting module, as they involve floating-point
 * rounding modes. Transactional conversion functions from picotm's
 * C Math Library module perform such operations.
 *
 * Also not supported by the type-casting module are conversions from
 * and to C strings. The `sscanf_tx()` and `snprintf_tx()` families of
 * functions from picotm's C Standard Library Module perform these
 * operations.
 *
 * Results or parameters of type-casting operations should be loaded
 * into the transaction or stored from the transaction using the
 * Transactional memory module.
 */
