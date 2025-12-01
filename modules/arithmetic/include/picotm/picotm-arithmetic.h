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

#include "picotm/config/picotm-arithmetic-config.h"
#include "picotm/compiler.h"

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_arithmetic
 * \file
 *
 * \brief Public interfaces of picotm's arithmetic module.
 */

PICOTM_NOTHROW
/**
 * \ingroup group_arithmetic
 * \internal
 * Reports a division-by-zero error to the transaction manager.
 * \warning This is an internal interface. Don't use it in application code.
 */
void
__picotm_arithmetic_error_divbyzero_tx(void);

PICOTM_NOTHROW
/**
 * \ingroup group_arithmetic
 * \internal
 * Reports an overflow error to the transaction manager.
 * \warning This is an internal interface. Don't use it in application code.
 */
void
__picotm_arithmetic_error_overflow_tx(void);

PICOTM_NOTHROW
/**
 * \ingroup group_arithmetic
 * \internal
 * Reports an underflow error to the transaction manager.
 * \warning This is an internal interface. Don't use it in application code.
 */
void
__picotm_arithmetic_error_underflow_tx(void);

/**
 * \ingroup group_arithmetic
 * \internal
 * Tests if a signed type uses two's complement.
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_ARITHMETIC_STATIC_ASSERT_TWOCPL(__type, __min, __max)      \
    PICOTM_STATIC_ASSERT((__min) == (-(__max) - 1),                         \
                         "Type " #__type " does not use two's complement.")

/**
 * \ingroup group_arithmetic
 * \internal
 * Tests if a type is unsigned.
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __PICOTM_ARITHMETIC_STATIC_ASSERT_UNSIGNED(__type, __min)           \
    PICOTM_STATIC_ASSERT(!(__min), "Type " #__type " is not unsigned.")

/*
 * Addition
 */

/**
 * \ingroup group_arithmetic
 * Defines a C function that adds two values of the signed C type `__type`.
 * \param   __name  The name of the value's type.
 * \param   __type  The C type of the value.
 * \param   __min   The minimum value of the type.
 * \param   __max   The maximum value of the type.
 * \param   __idt   The identity element (i.e., zero) of the type.
 * \param   __add   The add operation.
 * \param   __sub   The subtract operation.
 */
#define PICOTM_ARITHMETIC_ADD_S_TX(__name, __type, __min, __max, __idt,     \
                                   __add, __sub)                            \
    /**
        Adds two values of type `__type`. The function detects overflows
        and underflows of the type's range and reports them as errno code
        to the transaction's recovery phase.
        \param  lhs The left-hand-side addend.
        \param  rhs The right-hand-side addend.
        \returns The sum of lhs and rhs.
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        addition's result overflows the type's range.
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        addition's result underflows the type's range.
     */                                                                     \
    static inline __type                                                    \
    add_ ## __name ## _tx(__type lhs, __type rhs)                           \
    {                                                                       \
        __PICOTM_ARITHMETIC_STATIC_ASSERT_TWOCPL(__type, __min, __max);     \
        /* Instanciating constants avoids compiler warnings. */             \
        static const __type minval = (__min);                               \
        static const __type maxval = (__max);                               \
        static const __type idtval = (__idt);                               \
        if (rhs == idtval) {                                                \
            return lhs;                                                     \
        } else if (rhs > idtval) {                                          \
            if (__sub(maxval, rhs) < lhs) {                                 \
                __picotm_arithmetic_error_overflow_tx();                    \
            }                                                               \
        } else /* (rhs < idtval) */ {                                       \
            if (lhs == idtval) {                                            \
                return rhs;                                                 \
            } else /* (lhs > idtval) || (lhs < idtval) */ {                 \
                if (__sub(minval, rhs) > lhs) {                             \
                    __picotm_arithmetic_error_underflow_tx();               \
                }                                                           \
            }                                                               \
        }                                                                   \
        return __add(lhs, rhs);                                             \
    }

/**
 * \ingroup group_arithmetic
 * Defines a C function that adds two values of the unsigned C type `__type`.
 * \param   __name  The name of the value's type.
 * \param   __type  The C type of the value.
 * \param   __min   The minimum value of the type.
 * \param   __max   The maximum value of the type.
 * \param   __idt   The identity element (i.e., zero) of the type.
 * \param   __add   The add operation.
 * \param   __sub   The subtract operation.
 */
#define PICOTM_ARITHMETIC_ADD_U_TX(__name, __type, __min, __max, __idt,     \
                                   __add, __sub)                            \
    /**
        Adds two values of type `__type`. The function detects overflows
        of the type's range and reports them as errno code to the transaction's
        recovery phase.
        \param  lhs The left-hand-side addend.
        \param  rhs The right-hand-side addend.
        \returns The sum of lhs and rhs.
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        addition's result overflows the type's range.
     */                                                                     \
    static inline __type                                                    \
    add_ ## __name ## _tx(__type lhs, __type rhs)                           \
    {                                                                       \
        __PICOTM_ARITHMETIC_STATIC_ASSERT_UNSIGNED(__type, __min);          \
        /* Instanciating constants avoids compiler warnings. */             \
        static const __type maxval = (__max);                               \
        if (__sub(maxval, rhs) < lhs) {                                     \
            __picotm_arithmetic_error_overflow_tx();                        \
        }                                                                   \
        return __add(lhs, rhs);                                             \
    }

/**
 * \ingroup group_arithmetic
 * Defines a C function that adds two values of the floating-point C
 * type `__type`.
 * \param   __name  The name of the value's type.
 * \param   __type  The C type of the value.
 * \param   __ext   The type's extension.
 * \param   __idt   The identity element (i.e., zero) of the type.
 * \param   __add   The add operation.
 * \param   __sub   The subtract operation.
 */
#define PICOTM_ARITHMETIC_ADD_F_TX(__name, __type, __ext, __idt,            \
                                   __add, __sub)                            \
    /**
        Adds two values of type `__type`. The function detects overflows
        and underflows of the type's range and reports them as errno code
        to the transaction's recovery phase.
        \param  lhs The left-hand-side addend.
        \param  rhs The right-hand-side addend.
        \returns The sum of lhs and rhs.
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        addition's result overflows the type's range.
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        addition's result underflows the type's range.
     */                                                                     \
    static inline __type                                                    \
    add_ ## __name ## _tx(__type lhs, __type rhs)                           \
    {                                                                       \
        /* Instanciating constants avoids compiler warnings. */             \
        static const __type minval = -(__ext);                              \
        static const __type maxval =  (__ext);                              \
        static const __type idtval =  (__idt);                              \
        if (rhs == idtval) {                                                \
            return lhs;                                                     \
        } else if (rhs > idtval) {                                          \
            if (__sub(maxval, rhs) < lhs) {                                 \
                __picotm_arithmetic_error_overflow_tx();                    \
            }                                                               \
        } else /* (rhs < idtval) */ {                                       \
            if (lhs == idtval) {                                            \
                return rhs;                                                 \
            } else /* (lhs > idtval) || (lhs < idtval) */ {                 \
                if (__sub(minval, rhs) > lhs) {                             \
                    __picotm_arithmetic_error_underflow_tx();               \
                }                                                           \
            }                                                               \
        }                                                                   \
        return __add(lhs, rhs);                                             \
    }

/*
 * Subtraction
 */

/**
 * \ingroup group_arithmetic
 * Defines a C function that subtracts one value of the signed C type
 * `__type` from another value of the same type.
 * \param   __name  The name of the value's type.
 * \param   __type  The C type of the value.
 * \param   __min   The minimum value of the type.
 * \param   __max   The maximum value of the type.
 * \param   __idt   The identity element (i.e., zero) of the type.
 * \param   __add   The add operation.
 * \param   __sub   The subtract operation.
 */
#define PICOTM_ARITHMETIC_SUB_S_TX(__name, __type, __min, __max, __idt,     \
                                   __add, __sub)                            \
    /**
        Subtracts one value of type `__type` from another value of the same
        type. The function detects overflows and underflows of the type's
        range and reports them as errno code to the transaction's recovery
        phase.
        \param  lhs The left-hand-side minuend.
        \param  rhs The right-hand-side subtrahend.
        \returns The difference from rhs to lhs.
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        subtraction's result overflows the type's range.
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        subtraction's result underflows the type's range.
     */                                                                     \
    static inline __type                                                    \
    sub_ ## __name ## _tx(__type lhs, __type rhs)                           \
    {                                                                       \
        __PICOTM_ARITHMETIC_STATIC_ASSERT_TWOCPL(__type, __min, __max);     \
        /* Instanciating constants avoids compiler warnings. */             \
        static const __type minval = (__min);                               \
        static const __type maxval = (__max);                               \
        static const __type idtval = (__idt);                               \
        if (rhs == idtval) {                                                \
            return lhs;                                                     \
        } else if (rhs > idtval) {                                          \
            if (__add(minval, rhs) > lhs) {                                 \
                __picotm_arithmetic_error_underflow_tx();                   \
            }                                                               \
        } else /* (rhs < idtval) */ {                                       \
            if (lhs == idtval) {                                            \
                return __sub(idtval, rhs);                                  \
            } else /* (lhs > idtval) || (lhs < idtval) */ {                 \
                if (__add(maxval, rhs) < lhs) {                             \
                    __picotm_arithmetic_error_overflow_tx();                \
                }                                                           \
            }                                                               \
        }                                                                   \
        return __sub(lhs, rhs);                                             \
    }

/**
 * \ingroup group_arithmetic
 * Defines a C function that subtracts one value of the unsigned C type
 * `__type` from another value of the same type.
 * \param   __name  The name of the value's type.
 * \param   __type  The C type of the value.
 * \param   __min   The minimum value of the type.
 * \param   __max   The maximum value of the type.
 * \param   __idt   The identity element (i.e., zero) of the type.
 * \param   __add   The add operation.
 * \param   __sub   The subtract operation.
 */
#define PICOTM_ARITHMETIC_SUB_U_TX(__name, __type, __min, __max, __idt,     \
                                   __add, __sub)                            \
    /**
        Subtracts one value of type `__type` from another value of the same
        type. The function detects underflows of the type's range and reports
        them as errno code to the transaction's recovery phase.
        \param  lhs The left-hand-side minuend.
        \param  rhs The right-hand-side subtrahend.
        \returns The difference from rhs to lhs.
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        subtraction's result underflows the type's range.
     */                                                                     \
    static inline __type                                                    \
    sub_ ## __name ## _tx(__type lhs, __type rhs)                           \
    {                                                                       \
        __PICOTM_ARITHMETIC_STATIC_ASSERT_UNSIGNED(__type, __min);          \
        /* Instanciating constants avoids compiler warnings. */             \
        static const __type minval = (__min);                               \
        static const __type idtval = (__idt);                               \
        if (rhs > idtval) {                                                 \
            if (__add(minval, rhs) > lhs) {                                 \
                __picotm_arithmetic_error_underflow_tx();                   \
            }                                                               \
        }                                                                   \
        return __sub(lhs, rhs);                                             \
    }

/**
 * \ingroup group_arithmetic
 * Defines a C function that subtracts one value of the floating-point C type
 * `__type` from another value of the same type.
 * \param   __name  The name of the value's type.
 * \param   __type  The C type of the value.
 * \param   __ext   The type's extension.
 * \param   __idt   The identity element (i.e., zero) of the type.
 * \param   __add   The add operation.
 * \param   __sub   The subtract operation.
 */
#define PICOTM_ARITHMETIC_SUB_F_TX(__name, __type, __ext, __idt,            \
                                   __add, __sub)                            \
    /**
        Subtracts one value of type `__type` from another value of the same
        type. The function detects overflows and underflows of the type's
        range and reports them as errno code to the transaction's recovery
        phase.
        \param  lhs The left-hand-side minuend.
        \param  rhs The right-hand-side subtrahend.
        \returns The difference from rhs to lhs.
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        subtraction's result overflows the type's range.
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        subtraction's result underflows the type's range.
     */                                                                     \
    static inline __type                                                    \
    sub_ ## __name ## _tx(__type lhs, __type rhs)                           \
    {                                                                       \
        /* Instanciating constants avoids compiler warnings. */             \
        static const __type minval = -(__ext);                              \
        static const __type maxval =  (__ext);                              \
        static const __type idtval =  (__idt);                              \
        if (rhs == idtval) {                                                \
            return lhs;                                                     \
        } else if (rhs > idtval) {                                          \
            if (__add(minval, rhs) > lhs) {                                 \
                __picotm_arithmetic_error_underflow_tx();                   \
            }                                                               \
        } else /* (rhs < idtval) */ {                                       \
            if (lhs == idtval) {                                            \
                return __sub(idtval, rhs);                                  \
            } else /* (lhs > idtval) || (lhs < idtval) */ {                 \
                if (__add(maxval, rhs) < lhs) {                             \
                    __picotm_arithmetic_error_overflow_tx();                \
                }                                                           \
            }                                                               \
        }                                                                   \
        return __sub(lhs, rhs);                                             \
    }

/*
 * Multiplication
 */

/**
 * \ingroup group_arithmetic
 * Defines a C function that multiplies two values of the signed C type
 * `__type`.
 * \param   __name  The name of the value's type.
 * \param   __type  The C type of the value.
 * \param   __min   The type's minimum value.
 * \param   __max   The type's maximum value.
 * \param   __idt   The type's identity element (i.e., one).
 * \param   __asb   The type's absorbing element (i.e., zero).
 * \param   __mul   The multiply operation. Should be `&&` for _Bool and `*`
 *                  for everything else.
 * \param   __div   The divide operation. Divisions shall always round
 *                  towards zero.
 */
#define PICOTM_ARITHMETIC_MUL_S_TX(__name, __type, __min, __max, __idt,     \
                                   __asb, __mul, __div)                     \
    /**
        Multiplies two values of type `__type`. The function detects
        overflows and underflows of the type's range and reports them
        as errno code to the transaction's recovery phase.
        \param  lhs The left-hand-side factor.
        \param  rhs The right-hand-side factor.
        \returns The product of lhs and rhs.
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        multiplication's result overflows the type's range.
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        multiplication's result underflows the type's range.
     */                                                                     \
    static inline __type                                                    \
    mul_ ## __name ## _tx(__type lhs, __type rhs)                           \
    {                                                                       \
        __PICOTM_ARITHMETIC_STATIC_ASSERT_TWOCPL(__type, __min, __max);     \
        /* Instanciating constants avoids compiler warnings. */             \
        static const __type minval = (__min);                               \
        static const __type maxval = (__max);                               \
        static const __type idtval = (__idt);                               \
        static const __type asbval = (__asb);                               \
        if ((rhs == asbval) || (lhs == asbval)) {                           \
            return asbval;                                                  \
        } else if (rhs == idtval) {                                         \
            return lhs;                                                     \
        } else if (rhs > idtval) {                                          \
            if (lhs == idtval) {                                            \
                return rhs;                                                 \
            } else if (lhs > idtval) {                                      \
                if (__div(maxval, rhs) < lhs) {                             \
                    __picotm_arithmetic_error_overflow_tx();                \
                }                                                           \
            } else /* (lhs < idtval) */ {                                   \
                if (__div(minval, rhs) > lhs) {                             \
                    __picotm_arithmetic_error_underflow_tx();               \
                }                                                           \
            }                                                               \
        } else /* (rhs < idtval) */ {                                       \
            if (lhs == idtval) {                                            \
                return rhs;                                                 \
            } else if (lhs > idtval) {                                      \
                if (__div(-maxval, -rhs) < -lhs) {                          \
                    __picotm_arithmetic_error_underflow_tx();               \
                }                                                           \
            } else /* (lhs < idtval) */ {                                   \
                if (lhs == minval) {                                        \
                    __picotm_arithmetic_error_overflow_tx();                \
                } else if (__div(maxval, rhs) < lhs) {                      \
                    __picotm_arithmetic_error_overflow_tx();                \
                }                                                           \
            }                                                               \
        }                                                                   \
        return __mul(lhs, rhs);                                             \
    }

/**
 * \ingroup group_arithmetic
 * Defines a C function that multiplies two values of the unsigned C type
 * `__type`.
 * \param   __name  The name of the value's type.
 * \param   __type  The C type of the value.
 * \param   __min   The type's minimum value.
 * \param   __max   The type's maximum value.
 * \param   __idt   The type's identity element (i.e., one).
 * \param   __asb   The type's absorbing element (i.e., zero).
 * \param   __mul   The multiply operation. Should be `&&` for _Bool and `*`
 *                  for everything else.
 * \param   __div   The divide operation. Divisions shall always round
 *                  towards zero.
 */
#define PICOTM_ARITHMETIC_MUL_U_TX(__name, __type, __min, __max, __idt,     \
                                   __asb, __mul, __div)                     \
    /**
        Multiplies two values of type `__type`. The function detects
        overflows of the type's range and reports them as errno code
        to the transaction's recovery phase.
        \param  lhs The left-hand-side factor.
        \param  rhs The right-hand-side factor.
        \returns The product of lhs and rhs.
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        multiplication's result overflows the type's range.
     */                                                                     \
    static inline __type                                                    \
    mul_ ## __name ## _tx(__type lhs, __type rhs)                           \
    {                                                                       \
        __PICOTM_ARITHMETIC_STATIC_ASSERT_UNSIGNED(__type, __min);          \
        /* Instanciating constants avoids compiler warnings. */             \
        static const __type maxval = (__max);                               \
        static const __type asbval = (__asb);                               \
        if (rhs == asbval) {                                                \
            return asbval;                                                  \
        } else /* (rhs > absval) */ {                                       \
            if (__div(maxval, rhs) < lhs) {                                 \
                __picotm_arithmetic_error_overflow_tx();                    \
            }                                                               \
        }                                                                   \
        return __mul(lhs, rhs);                                             \
    }

/**
 * \ingroup group_arithmetic
 * Defines a C function that multiplies two values of the floating-point C
 * type `__type`.
 * \param   __name  The name of the value's type.
 * \param   __type  The C type of the value.
 * \param   __ext   The type's extension.
 * \param   __idt   The type's identity element (i.e., one).
 * \param   __asb   The type's absorbing element (i.e., zero).
 * \param   __mul   The multiply operation. Should be `&&` for _Bool and `*`
 *                  for everything else.
 * \param   __div   The divide operation. Divisions shall always round
 *                  towards zero.
 */
#define PICOTM_ARITHMETIC_MUL_F_TX(__name, __type, __ext, __idt, __asb,     \
                                   __mul, __div)                            \
    /**
        Multiplies two values of type `__type`. The function detects
        overflows and underflows of the type's range and reports them
        as errno code to the transaction's recovery phase.
        \param  lhs The left-hand-side factor.
        \param  rhs The right-hand-side factor.
        \returns The product of lhs and rhs.
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        multiplication's result overflows the type's range.
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        multiplication's result underflows the type's range.
     */                                                                     \
    static inline __type                                                    \
    mul_ ## __name ## _tx(__type lhs, __type rhs)                           \
    {                                                                       \
        /* Instanciating constants avoids compiler warnings. */             \
        static const __type minval = -(__ext);                              \
        static const __type maxval =  (__ext);                              \
        static const __type idtval =  (__idt);                              \
        static const __type asbval =  (__asb);                              \
        if ((rhs == asbval) || (lhs == asbval)) {                           \
            return asbval;                                                  \
        } else if (rhs == idtval) {                                         \
            return lhs;                                                     \
        } else if (rhs > idtval) {                                          \
            if (lhs > asbval) {                                             \
                if (__div(maxval, rhs) < lhs) {                             \
                    __picotm_arithmetic_error_overflow_tx();                \
                }                                                           \
            } else /* (lhs < absval) */ {                                   \
                if (__div(minval, rhs) > lhs) {                             \
                    __picotm_arithmetic_error_underflow_tx();               \
                }                                                           \
            }                                                               \
        } else /* (rhs < -idtval) */ {                                      \
            if (lhs > asbval) {                                             \
                if (__div(minval, rhs) < lhs) {                             \
                    __picotm_arithmetic_error_underflow_tx();               \
                }                                                           \
            } else /* (lhs < absval) */ {                                   \
                if (__div(maxval, rhs) > lhs) {                             \
                    __picotm_arithmetic_error_overflow_tx();                \
                }                                                           \
            }                                                               \
        }                                                                   \
        return __mul(lhs, rhs);                                             \
    }

/*
 * Division
 */

/**
 * \ingroup group_arithmetic
 * Defines a C function that divides a values of the signed C type `__type`
 * by a value of the same type.
 * \param   __name  The name of the value's type.
 * \param   __type  The C type of the value.
 * \param   __min   The type's minimum value.
 * \param   __max   The type's maximum value.
 * \param   __idt   The type's identity element (i.e., one).
 * \param   __asb   The type's absorbing element (i.e., zero).
 * \param   __mul   The multiply operation. Should be `&&` for _Bool and `*`
 *                  for everything else.
 * \param   __div   The divide operation. Divisions shall always round
 *                  towards zero.
 */
#define PICOTM_ARITHMETIC_DIV_S_TX(__name, __type, __min, __max, __idt,     \
                                   __asb, __mul, __div)                     \
    /**
        Divides a value of type `__type` by another value of the same
        type. The function detects divisions by zero, overflows and
        underflows of the type's range and reports them as errno code to
        the transaction's recovery phase.
        \param  lhs The left-hand-side dividend.
        \param  rhs The right-hand-side divisor.
        \returns The fraction of lhs divided by rhs.
        \retval EDOM    Reported to the transaction's recovery phase if the
                        transaction tries to performed a division by *0.*
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        division's result overflows the type's range.
     */                                                                     \
    static inline __type                                                    \
    div_ ## __name ## _tx(__type lhs, __type rhs)                           \
    {                                                                       \
        __PICOTM_ARITHMETIC_STATIC_ASSERT_TWOCPL(__type, __min, __max);     \
        /* Instanciating constants avoids compiler warnings. */             \
        static const __type minval = (__min);                               \
        static const __type idtval = (__idt);                               \
        static const __type asbval = (__asb);                               \
        if (rhs == asbval) {                                                \
            /* Dividing by 0 is an error. */                                \
            __picotm_arithmetic_error_divbyzero_tx();                       \
        } else if ((lhs == minval) && (rhs == -idtval)) {                   \
            /* Dividing minval value by -1 results in (maxval + 1). */      \
            __picotm_arithmetic_error_overflow_tx();                        \
        }                                                                   \
        return __div(lhs, rhs);                                             \
    }

/**
 * \ingroup group_arithmetic
 * Defines a C function that divides a values of the unsigned C type `__type`
 * by a value of the same type.
 * \param   __name  The name of the value's type.
 * \param   __type  The C type of the value.
 * \param   __min   The type's minimum value.
 * \param   __max   The type's maximum value.
 * \param   __idt   The type's identity element (i.e., one).
 * \param   __asb   The type's absorbing element (i.e., zero).
 * \param   __mul   The multiply operation. Should be `&&` for _Bool and `*`
 *                  for everything else.
 * \param   __div   The divide operation. Divisions shall always round
 *                  towards zero.
 */
#define PICOTM_ARITHMETIC_DIV_U_TX(__name, __type, __min, __max, __idt,     \
                                   __asb, __mul, __div)                     \
    /**
        Divides a value of type `__type` by another value of the same
        type. The function detects divisions by zero, overflows and reports
        them as errno code to the transaction's recovery phase.
        \param  lhs The left-hand-side dividend.
        \param  rhs The right-hand-side divisor.
        \returns The fraction of lhs divided by rhs.
        \retval EDOM    Reported to the transaction's recovery phase if the
                        transaction tries to performed a division by *0.*
    */                                                                      \
    static inline __type                                                    \
    div_ ## __name ## _tx(__type lhs, __type rhs)                           \
    {                                                                       \
        __PICOTM_ARITHMETIC_STATIC_ASSERT_UNSIGNED(__type, __min);          \
        /* Instanciating constants avoids compiler warnings. */             \
        static const __type asbval = (__asb);                               \
        if (rhs == asbval) {                                                \
            /* Dividing by 0 is an error. */                                \
            __picotm_arithmetic_error_divbyzero_tx();                       \
        }                                                                   \
        return __div(lhs, rhs);                                             \
    }

/**
 * \ingroup group_arithmetic
 * Defines a C function that divides a values of the floatng-point C type
 * `__type` by a value of the same type.
 * \param   __name  The name of the value's type.
 * \param   __type  The C type of the value.
 * \param   __ext   The type's extension.
 * \param   __idt   The type's identity element (i.e., one).
 * \param   __asb   The type's absorbing element (i.e., zero).
 * \param   __mul   The multiply operation. Should be `&&` for _Bool and `*`
 *                  for everything else.
 * \param   __div   The divide operation. Divisions shall always round
 *                  towards zero.
 */
#define PICOTM_ARITHMETIC_DIV_F_TX(__name, __type, __ext, __idt, __asb,     \
                                   __mul, __div)                            \
    /**
        Divides a value of type `__type` by another value of the same
        type. The function detects divisions by zero, overflows and
        underflows of the type's range and reports them as errno code to
        the transaction's recovery phase.
        \param  lhs The left-hand-side dividend.
        \param  rhs The right-hand-side divisor.
        \returns The fraction of lhs divided by rhs.
        \retval EDOM    Reported to the transaction's recovery phase if the
                        transaction tries to performed a division by *0.*
        \retval ERANGE  Reported to the transaction's recovery phase if the
                        division's result overflows the type's range.
     */                                                                     \
    static inline __type                                                    \
    div_ ## __name ## _tx(__type lhs, __type rhs)                           \
    {                                                                       \
        /* Instanciating constants avoids compiler warnings. */             \
        static const __type minval = -(__ext);                              \
        static const __type maxval =  (__ext);                              \
        static const __type idtval =  (__idt);                              \
        static const __type asbval =  (__asb);                              \
        if (rhs == asbval) {                                                \
            /* Dividing by 0 is an error. */                                \
            __picotm_arithmetic_error_divbyzero_tx();                       \
        } else if ((rhs > asbval) && (rhs < idtval)) {                      \
            /* Dividing without integral part is a multiplication. */       \
            if (lhs > asbval) {                                             \
                const __type limit = __mul(maxval, rhs);                    \
                if (lhs > limit) {                                          \
                    __picotm_arithmetic_error_overflow_tx();                \
                }                                                           \
            } else if (lhs < asbval) {                                      \
                const __type limit = __mul(minval, rhs);                    \
                if (lhs < limit) {                                          \
                    __picotm_arithmetic_error_underflow_tx();               \
                }                                                           \
            }                                                               \
        } else if ((rhs < asbval) && (rhs > -idtval)) {                     \
            /* Division without integral part is a multiplication. */       \
            if (lhs > asbval) {                                             \
                const __type limit = __mul(minval, rhs);                    \
                if (lhs > limit) {                                          \
                    __picotm_arithmetic_error_underflow_tx();               \
                }                                                           \
            } else if (lhs < asbval) {                                      \
                const __type limit = __mul(maxval, rhs);                    \
                if (lhs < limit) {                                          \
                    __picotm_arithmetic_error_overflow_tx();                \
                }                                                           \
            }                                                               \
        }                                                                   \
        return __div(lhs, rhs);                                             \
    }

PICOTM_END_DECLS

/**
 * \defgroup group_arithmetic The Arithmetic Module
 *
 * \brief The arithmetic module provides safe arithmetics for C types.
 *        Overflows or underflows in the type's range are reported as
 *        errors to the transaction.
 *
 * The arithmetic module provides safe arithmetic operations for C types.
 * Overflows, underflows and undefined operations are detected and reported
 * to the transaction's recovery code. Supported operations are additions,
 * subtractions, multiplications and divisions.
 *
 * Additions are in the form `add_<type>_tx()` where `<type>` is the
 * name of the C type. For example, overflow-safe addition of two long
 * integer values is performed by the following transaction.
 *
 * ~~~{.c}
 *  picotm_begin
 *      long res = add_long_tx(INT_MAX, INT_MAX);
 *  picotm_commit
 *      if ((picotm_error_status() == PICOTM_ERRNO) &&
 *          (picotm_error_as_errno() == ERANGE)) {
 *          // An error happened during an arithmetic operation.
 *      }
 *  picotm_end
 * ~~~
 *
 * The success of this operations depends on the size of type `long`. If
 * `long` is 64 bit in size, the addition will return the sum of both
 * operands. If `long` is only 32 bit in size, which is the same of `int`
 * on most platforms, the addition will overflow and the function will roll
 * the transaction back to the previous consistent state and report `ERANGE`
 * to the recovery code.
 *
 * The arithmetic module comes with support for all native C types. We can
 * define safe addition for application-specific types with the
 * C preprocessor macros `PICOTM_ARITHMETIC_ADD_S_TX()`,
 * `PICOTM_ARITHMETIC_ADD_U_TX()` and `PICOTM_ARITHMETIC_ADD_F_TX()`. The first
 * macro, `PICOTM_ARITHMETIC_ADD_S_TX()` supports signed integer types; the
 * second macro, `PICOTM_ARITHMETIC_ADD_U_TX()`,  supports unsigned integer
 * types. Finally, the third macro, `PICOTM_ARITHMETIC_ADD_F_TX()`, supports
 * floating-point types.
 *
 * For example, an application might use the specialized floating-point
 * type `float100`, which only contains values from minus 100 to plus 100.
 *
 * ~~~{.c}
 *  typedef float float100;
 *
 *  #define FLT100_MAX  100     // similar to FLT_MAX from <float.h>
 * ~~~
 *
 * We can use the macro `PICOTM_ARITHMETIC_ADD_F_TX()` to create an add
 * function that overflows at plus 100 and underflows at minus 100.
 *
 * ~~~{.c}
 *  float100
 *  float100_add(float100 lhs, float100 rhs)
 *  {
 *      return lhs + rhs;
 *  }
 *
 *  float100
 *  float100_sub(float100 lhs, float100 rhs)
 *  {
 *      return lhs - rhs;
 *  }
 *
 *  PICOTM_ARITHMETIC_ADD_F_TX(float100, float100, FLT100_MAX, 0.f,
 *                           float100_add, float_100_sub)
 * ~~~
 *
 * The first two parameters are name and C type of the function. The
 * third parameter is the type's range or extension. For `float100` it
 * ranges from `-FLT100_MAX` to `FLT100_MAX`. The forth paramater is the
 * value of the type's operation's identity element (i.e., the value
 * that does nothing for the operation). For additions, the identity
 * element is *0.* The two final macro parameters are the type's primitive
 * addition and subtraction functions. These functions perform the plain
 * operations without safety checks.
 *
 * The example macro invocation expands to the following function.
 *
 * ~~~{.c}
 *  static inline
 *  float100
 *  add_float100_tx(float100 lhs, float100 rhs)
 *  {
 *      ... // safe addition
 *  }
 * ~~~
 *
 * A transaction can use it like `add_long_tx()` in the previous
 * example.
 *
 * ~~~{.c}
 *  picotm_begin
 *      float100 res = add_float100_tx(75.f, 25.f);
 *  picotm_commit
 *      if ((picotm_error_status() == PICOTM_ERRNO) &&
 *          (picotm_error_as_errno() == ERANGE)) {
 *          // An error happened during an arithmetic operation.
 *      }
 *  picotm_end
 * ~~~
 *
 * Picotm provides equivalent generator macros for subtraction,
 * multiplication and division of signed integer types, unsigned
 * integer types and floating-point types. Defining subtraction,
 * multiplication and division completes elementary arithmetics for
 * `float100`. The full example code is shown below.
 *
 * ~~~{.c}
 *  float100
 *  float100_add(float100 lhs, float100 rhs)
 *  {
 *      return lhs + rhs;
 *  }
 *
 *  float100
 *  float100_sub(float100 lhs, float100 rhs)
 *  {
 *      return lhs - rhs;
 *  }
 *
 *  float100
 *  float100_mul(float100 lhs, float100 rhs)
 *  {
 *      return lhs * rhs;
 *  }
 *
 *  float100
 *  float100_div(float100 lhs, float100 rhs)
 *  {
 *      return lhs / rhs;
 *  }
 *
 *  PICOTM_ARITHMETIC_ADD_F_TX(float100, float100, FLT100_MAX, 0.f,
 *                           float100_add, float_100_sub)
 *  PICOTM_ARITHMETIC_SUB_F_TX(float100, float100, FLT100_MAX, 0.f,
 *                           float100_add, float_100_sub)
 *  PICOTM_ARITHMETIC_MUL_F_TX(float100, float100, FLT100_MAX, 1.f, 0.f,
 *                           float100_mul, float_100_div)
 *  PICOTM_ARITHMETIC_DIV_F_TX(float100, float100, FLT100_MAX, 1.f, 0.f,
 *                           float100_div, float_100_div)
 * ~~~
 *
 * The identity element for multiplication and division is *1.* In addition
 * to the identity element, the generator macros for multiplication and
 * division take an additional absorbing element. This is the value the maps
 * each factor to the absorbing element itself. For multiplication this value
 * is *0.* For divisions by the absorbing element, the generated division
 * function rolls the transaction back and returns the errno code `EDOM` to
 * the recovery mode.
 *
 * Results or parameters of arithmetics operations should be loaded into the
 * transaction or stored from the transaction using the Transactional Memory
 * module.
 */
