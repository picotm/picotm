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

#include "picotm/string.h"
#include "picotm/picotm.h"
#include "picotm/picotm-tm.h"
#include "picotm/stdlib.h"
#include "picotm/string-tm.h"
#include <errno.h>
#include <stdbool.h>

/*
 * Memory functions
 */

#if defined(PICOTM_LIBC_HAVE_MEMCCPY) && PICOTM_LIBC_HAVE_MEMCCPY
PICOTM_EXPORT
void*
memccpy_tx(void* restrict dest, const void* restrict src, int c, size_t n)
{
    uint8_t* dest8 = dest;
    const uint8_t* src8 = src;

    dest = NULL; /* Return NULL by default */

    while (n) {

        uint8_t buf[32];

        size_t siz = n > sizeof(buf) ? sizeof(buf) : n;

        load_tx(src8, buf, siz);

        for (size_t i = 0; i < siz; ++i) {
            if (buf[i] == c) {
                dest = dest8 + i + 1; /* Return the location after 'c' */
                siz = i; /* Stop looping */
            }
        }

        store_tx(dest8, buf, siz);
        dest8 += siz;
        src8 += siz;
        n -= siz;
    }
    return dest;
}
#endif

#if defined(PICOTM_LIBC_HAVE_MEMCHR) && PICOTM_LIBC_HAVE_MEMCHR
PICOTM_EXPORT
void*
memchr_tx(const void* s, int c, size_t n)
{
    privatize_tx(s, n, PICOTM_TM_PRIVATIZE_LOAD);
    return memchr_tm(s, c, n);
}
#endif

#if defined(PICOTM_LIBC_HAVE_MEMCMP) && PICOTM_LIBC_HAVE_MEMCMP
PICOTM_EXPORT
int
memcmp_tx(const void* s1, const void* s2, size_t n)
{
    privatize_tx(s1, n, PICOTM_TM_PRIVATIZE_LOAD);
    privatize_tx(s2, n, PICOTM_TM_PRIVATIZE_LOAD);
    return memcmp_tm(s1, s2, n);
}
#endif

#if defined(PICOTM_LIBC_HAVE_MEMCPY) && PICOTM_LIBC_HAVE_MEMCPY
PICOTM_EXPORT
void*
memcpy_tx(void* restrict dest, const void* restrict src, size_t n)
{
    privatize_tx(dest, n, PICOTM_TM_PRIVATIZE_STORE);
    privatize_tx(src, n, PICOTM_TM_PRIVATIZE_LOAD);
    return memcpy_tm(dest, src, n);
}
#endif

#if defined(PICOTM_LIBC_HAVE_MEMMOVE) && PICOTM_LIBC_HAVE_MEMMOVE
PICOTM_EXPORT
void*
memmove_tx(void* dest, const void* src, size_t n)
{
    uint8_t* dest8 = dest;
    const uint8_t* src8 = src;

    while (n) {

        uint8_t buf[32];

        size_t siz = n > sizeof(buf) ? sizeof(buf) : n;

        load_tx(src8, buf, siz);
        store_tx(dest8, buf, siz);

        dest8 += siz;
        src8 += siz;
        n -= siz;
    }
    return dest;
}
#endif

#if defined(PICOTM_LIBC_HAVE_MEMSET) && PICOTM_LIBC_HAVE_MEMSET
PICOTM_EXPORT
void*
memset_tx(void* s, int c, size_t n)
{
    uint8_t buf[32];
    memset(buf, c, sizeof(buf));

    uint8_t* s8 = s;

    while (n) {
        size_t siz = n > sizeof(buf) ? sizeof(buf) : n;
        store_tx(s8, buf, siz);

        s8 += siz;
        n -= siz;
    }
    return s;
}
#endif

#if defined(PICOTM_LIBC_HAVE_MEMRCHR) && PICOTM_LIBC_HAVE_MEMRCHR
PICOTM_EXPORT
void*
memrchr_tx(const void* s, int c, size_t n)
{
    privatize_tx(s, n, PICOTM_TM_PRIVATIZE_LOAD);
    return memrchr_tm(s, c, n);
}
#endif

#if defined(PICOTM_LIBC_HAVE_RAWMEMCHR) && PICOTM_LIBC_HAVE_RAWMEMCHR
PICOTM_EXPORT
void*
rawmemchr_tx(const void* s, int c)
{
    privatize_c_tx(s, c, PICOTM_TM_PRIVATIZE_LOAD);
    return rawmemchr_tm(s, c);
}
#endif

/*
 * String functions
 */

#if defined(PICOTM_LIBC_HAVE_STPCPY) && PICOTM_LIBC_HAVE_STPCPY
PICOTM_EXPORT
char*
stpcpy_tx(char* restrict dest, const char* restrict src)
{
    privatize_c_tx(src, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    store_tx(dest, src, strlen(src));
    return dest;
}
#endif

#if defined(PICOTM_LIBC_HAVE_STPNCPY) && PICOTM_LIBC_HAVE_STPNCPY
PICOTM_EXPORT
char*
stpncpy_tx(char* restrict dest, const char* restrict src, size_t n)
{
    privatize_c_tx(src, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    size_t len = strlen(src) + 1;
    if (len < n) {
        store_tx(dest, src, len);
        memset_tx(dest + len, 0, n - len);
    } else {
        store_tx(dest, src, n);
    }
    return dest;
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRCAT) && PICOTM_LIBC_HAVE_STRCAT
PICOTM_EXPORT
char*
strcat_tx(char* restrict dest, const char* restrict src)
{
    privatize_c_tx(dest, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_c_tx(src, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    size_t dlen = strlen(dest);
    size_t slen = strlen(src) + 1;
    store_tx(dest + dlen, src, slen);
    return dest;
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRCHR) && PICOTM_LIBC_HAVE_STRCHR
PICOTM_EXPORT
char*
strchr_tx(const char* s, int c)
{
    privatize_c_tx(s, c, PICOTM_TM_PRIVATIZE_LOAD);
    return strchr_tm(s, c);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRCMP) && PICOTM_LIBC_HAVE_STRCMP
PICOTM_EXPORT
int
strcmp_tx(const char* s1, const char* s2)
{
    privatize_c_tx(s1, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_c_tx(s2, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return strcmp_tm(s1, s2);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRCOLL_L) && PICOTM_LIBC_HAVE_STRCOLL_L
PICOTM_EXPORT
int
strcoll_l_tx(const char* s1, const char* s2, locale_t locale)
{
    privatize_c_tx(s1, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_c_tx(s2, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return strcoll_l_tm(s1, s2, locale);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRCPY) && PICOTM_LIBC_HAVE_STRCPY
PICOTM_EXPORT
char*
strcpy_tx(char* restrict dest, const char* restrict src)
{
    privatize_c_tx(src, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    store_tx(dest, src, strlen(src) + 1);
    return dest;
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRCSPN) && PICOTM_LIBC_HAVE_STRCSPN
PICOTM_EXPORT
size_t
strcspn_tx(const char* s, const char* reject)
{
    privatize_c_tx(s, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return strcspn(s, reject);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRDUP) && PICOTM_LIBC_HAVE_STRDUP
PICOTM_EXPORT
char*
strdup_tx(const char* s)
{
    privatize_c_tx(s, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return strdup_tm(s);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRERROR_R) && PICOTM_LIBC_HAVE_STRERROR_R
PICOTM_EXPORT
char*
__strerror_r_gnu_tx(int errnum, char* buf, size_t buflen)
{
    privatize_tx(buf, buflen, PICOTM_TM_PRIVATIZE_STORE);
    return __strerror_r_gnu_tm(errnum, buf, buflen);
}

PICOTM_EXPORT
int
__strerror_r_posix_tx(int errnum, char* buf, size_t buflen)
{
    privatize_tx(buf, buflen, PICOTM_TM_PRIVATIZE_STORE);
    return __strerror_r_posix_tm(errnum, buf, buflen);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRLEN) && PICOTM_LIBC_HAVE_STRLEN
PICOTM_EXPORT
size_t
strlen_tx(const char* s)
{
    privatize_c_tx(s, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return strlen(s);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRNCAT) && PICOTM_LIBC_HAVE_STRNCAT
PICOTM_EXPORT
char*
strncat_tx(char* restrict dest, const char* restrict src, size_t n)
{
    privatize_c_tx(dest, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_c_tx(src, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    size_t dlen = strlen(dest);
    size_t slen = strlen(src) + 1;
    if (n < slen) {
        store_tx(dest + dlen, src, n);
        memset_tx(dest + dlen + n, '\0', 1);
    } else {
        store_tx(dest + dlen, src, slen);
    }
    return dest;
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRNCMP) && PICOTM_LIBC_HAVE_STRNCMP
PICOTM_EXPORT
int
strncmp_tx(const char* s1, const char* s2, size_t n)
{
    privatize_c_tx(s1, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_c_tx(s2, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return strncmp(s1, s2, n);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRNCPY) && PICOTM_LIBC_HAVE_STRNCPY
PICOTM_EXPORT
char*
strncpy_tx(char* restrict dest, const char* restrict src, size_t n)
{
    privatize_c_tx(src, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    size_t len = strlen(src) + 1;
    if (len < n) {
        store_tx(dest, src, len);
    } else {
        store_tx(dest, src, n);
    }
    return dest;
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRNDUP) && PICOTM_LIBC_HAVE_STRNDUP
PICOTM_EXPORT
char*
strndup_tx(const char* s, size_t n)
{
    privatize_c_tx(s, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return strndup_tm(s, n);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRNLEN) && PICOTM_LIBC_HAVE_STRNLEN
PICOTM_EXPORT
size_t
strnlen_tx(const char* s, size_t maxlen)
{
    privatize_c_tx(s, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return strnlen(s, maxlen);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRPBRK) && PICOTM_LIBC_HAVE_STRPBRK
PICOTM_EXPORT
char*
strpbrk_tx(const char* s, const char* accept)
{
    privatize_c_tx(s, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_c_tx(accept, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return strpbrk(s, accept);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRRCHR) && PICOTM_LIBC_HAVE_STRRCHR
PICOTM_EXPORT
char*
strrchr_tx(const char* s, int c)
{
    privatize_c_tx(s, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return strrchr(s, c);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRSPN) && PICOTM_LIBC_HAVE_STRSPN
PICOTM_EXPORT
size_t
strspn_tx(const char* s, const char* accept)
{
    privatize_c_tx(s, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_c_tx(accept, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return strspn(s, accept);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRSTR) && PICOTM_LIBC_HAVE_STRSTR
PICOTM_EXPORT
char*
strstr_tx(const char* haystack, const char* needle)
{
    privatize_c_tx(haystack, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_c_tx(needle, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return strstr(haystack, needle);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRTOK_R) && PICOTM_LIBC_HAVE_STRTOK_R
PICOTM_EXPORT
char*
strtok_r_tx(char* restrict str, const char* restrict delim,
            char** restrict saveptr)
{
    privatize_c_tx(str, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_c_tx(delim, '\0', PICOTM_TM_PRIVATIZE_LOAD);

    char* ptr = load_ptr_tx(saveptr);
    char* tok = strtok_r(str, delim, &ptr);
    store_ptr_tx(saveptr, &ptr);

    return tok;
}
#endif
