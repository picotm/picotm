/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <picotm/compiler.h>
#include <picotm/picotm-tm.h>
#include <stdlib.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <stdlib.h>.
 */

PICOTM_TM_LOAD_TX(div_t, div_t);
PICOTM_TM_LOAD_TX(ldiv_t, ldiv_t);
PICOTM_TM_LOAD_TX(lldiv_t, lldiv_t);
/*PICOTM_TM_LOAD_TX(size_t, size_t);*/ /* defined in stddef.h */
/*PICOTM_TM_LOAD_TX(_t, wchar_t);*/ /* defined in stddef.h */

PICOTM_TM_STORE_TX(div_t, div_t);
PICOTM_TM_STORE_TX(ldiv_t, ldiv_t);
PICOTM_TM_STORE_TX(lldiv_t, lldiv_t);
/*PICOTM_TM_STORE_TX(size_t, size_t);*/ /* defined in stddef.h */
/*PICOTM_TM_LOAD_TX(_t, wchar_t);*/ /* defined in stddef.h */

PICOTM_NOTHROW
PICOTM_NORETURN
/**
 * A transaction-safe implementation of [_Exit()][posix::_Exit].
 *
 * \bug Calling _Exit() simply terminates the process. It probably should not
 *      be available in a transaction.
 *
 * [posix::_Exit]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/_Exit.html
 */
void
_Exit_tx(int status);

PICOTM_NOTHROW
PICOTM_NORETURN
/**
 * A transaction-safe implementation of [abort()][posix::abort].
 *
 * \bug Calling abort() simply terminates the process. It probably should not
 *      be available in a transaction.
 *
 * [posix::abort]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/abort.html
 */
void
abort_tx(void);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [calloc()][posix::calloc].
 *
 * [posix::calloc]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/calloc.html
 */
void*
calloc_tx(size_t nmemb, size_t size);

PICOTM_NOTHROW
PICOTM_NORETURN
/**
 * A transaction-safe implementation of [exit()][posix::exit].
 *
 * \bug Calling exit() simply terminates the process. It probably should not
 *      be available in a transaction.
 *
 * [posix::exit]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/exit.html
 */
void
exit_tx(int status);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [free()][posix::free].
 *
 * [posix::free]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/free.html
 */
void
free_tx(void* ptr);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [malloc()][posix::malloc].
 *
 * [posix::malloc]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/malloc.html
 */
void*
malloc_tx(size_t size);

PICOTM_EXPORT
/**
 * A transaction-safe implementation of [mkdtemp()][posix::mkdtemp].
 *
 * [posix::mkdtemp]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/mkdtemp.html
 */
char*
mkdtemp_tx(char* template);

PICOTM_EXPORT
/**
 * A transaction-safe implementation of [mkstemp()][posix::mkstemp].
 *
 * [posix::mkstemp]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/mkstemp.html
 */
int
mkstemp_tx(char* template);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [posix_memalign()][posix::posix_memalign].
 *
 * [posix::posix_memalign]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_memalign.html
 */
int
posix_memalign_tx(void** memptr, size_t alignment, size_t size);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [qsort()][posix::qsort].
 *
 * [posix::qsort]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/qsort.html
 */
void
qsort_tx(void* base, size_t nel, size_t width,
         int (*compar)(const void*, const void*));

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [realloc()][posix::realloc].
 *
 * [posix::realloc]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/realloc.html
 */
void*
realloc_tx(void* ptr, size_t size);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [rand_r()][posix::rand_r].
 *
 * [posix::rand_r]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/rand_r.html
 */
int
rand_r_tx(unsigned int* seed);

PICOTM_END_DECLS
