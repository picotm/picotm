/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systx/string.h"
#include <errno.h>
#include <stdbool.h>
#include <systx/systx.h>
#include <systx/systx-tm.h>
#include "systx/stdlib.h"
#include "systx/string-tm.h"

/*
 * Memory functions
 */

SYSTX_EXPORT
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

SYSTX_EXPORT
void*
memchr_tx(const void* s, int c, size_t n)
{
    privatize_tx(s, n, SYSTX_TM_PRIVATIZE_LOAD);
    return memchr_tm(s, c, n);
}

SYSTX_EXPORT
int
memcmp_tx(const void* s1, const void* s2, size_t n)
{
    privatize_tx(s1, n, SYSTX_TM_PRIVATIZE_LOAD);
    privatize_tx(s2, n, SYSTX_TM_PRIVATIZE_LOAD);
    return memcmp_tm(s1, s2, n);
}

SYSTX_EXPORT
void*
memcpy_tx(void* restrict dest, const void* restrict src, size_t n)
{
    privatize_tx(dest, n, SYSTX_TM_PRIVATIZE_STORE);
    privatize_tx(src, n, SYSTX_TM_PRIVATIZE_LOAD);
    return memcpy_tm(dest, src, n);
}

SYSTX_EXPORT
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

SYSTX_EXPORT
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

SYSTX_EXPORT
void*
memrchr_tx(const void* s, int c, size_t n)
{
    privatize_tx(s, n, SYSTX_TM_PRIVATIZE_LOAD);
    return memrchr_tm(s, c, n);
}

SYSTX_EXPORT
void*
rawmemchr_tx(const void* s, int c)
{
    privatize_c_tx(s, c, SYSTX_TM_PRIVATIZE_LOAD);
    return rawmemchr_tm(s, c);
}

/*
 * String functions
 */

SYSTX_EXPORT
char*
stpcpy_tx(char* restrict dest, const char* restrict src)
{
    privatize_c_tx(src, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    store_tx(dest, src, strlen(src));
    return dest;
}

SYSTX_EXPORT
char*
stpncpy_tx(char* restrict dest, const char* restrict src, size_t n)
{
    privatize_c_tx(src, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    size_t len = strlen(src) + 1;
    if (len < n) {
        store_tx(dest, src, len);
        memset_tx(dest + len, 0, n - len);
    } else {
        store_tx(dest, src, n);
    }
    return dest;
}

SYSTX_EXPORT
char*
strcat_tx(char* restrict dest, const char* restrict src)
{
    privatize_c_tx(dest, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    privatize_c_tx(src, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    size_t dlen = strlen(dest);
    size_t slen = strlen(src) + 1;
    store_tx(dest + dlen, src, slen);
    return dest;
}

SYSTX_EXPORT
char*
strchr_tx(const char* s, int c)
{
    privatize_c_tx(s, c, SYSTX_TM_PRIVATIZE_LOAD);
    return strchr_tm(s, c);
}

SYSTX_EXPORT
int
strcmp_tx(const char* s1, const char* s2)
{
    privatize_c_tx(s1, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    privatize_c_tx(s2, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    return strcmp_tm(s1, s2);
}

SYSTX_EXPORT
int
strcoll_l_tx(const char* s1, const char* s2, locale_t locale)
{
    privatize_c_tx(s1, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    privatize_c_tx(s2, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    return strcoll_l_tm(s1, s2, locale);
}

SYSTX_EXPORT
char*
strcpy_tx(char* restrict dest, const char* restrict src)
{
    privatize_c_tx(src, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    store_tx(dest, src, strlen(src) + 1);
    return dest;
}

SYSTX_EXPORT
size_t
strcspn_tx(const char* s, const char* reject)
{
    privatize_c_tx(s, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    return strcspn(s, reject);
}

SYSTX_EXPORT
char*
strdup_tx(const char* s)
{
    privatize_c_tx(s, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    size_t len = strlen(s) + 1;
    char* mem = malloc_tx(len);
    if (!mem) {
        return NULL;
    }
    return memcpy(mem, s, len);
}

SYSTX_EXPORT
int
strerror_r_tx(int errnum, char* buf, size_t buflen)
{
    /* We cannot easily allocate memory dynamically here. Use a
     * fixed-size buffer instead. Errno strings are not that long,
     * so a small buffer should be sufficient. */
    char tmpbuf[256];

#if (_POSIX_C_SOURCE >= 200112L) && !_GNU_SOURCE
    int res = strerror_r(errnum, tmpbuf, sizeof(tmpbuf));
#else
    strerror_r(errnum, tmpbuf, sizeof(tmpbuf));
    int res = 0;
#endif
    if (res) {
        return res;
    }
    size_t len = strlen(tmpbuf) + 1;

    if (len > buflen) {
        len = buflen;
        if (!len) {
            return ERANGE;
        }
        tmpbuf[len - 1] = '\0';
    }
    store_tx(buf, tmpbuf, len);

    return 0;
}

SYSTX_EXPORT
size_t
strlen_tx(const char* s)
{
    privatize_c_tx(s, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    return strlen(s);
}

SYSTX_EXPORT
char*
strncat_tx(char* restrict dest, const char* restrict src, size_t n)
{
    privatize_c_tx(dest, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    privatize_c_tx(src, '\0', SYSTX_TM_PRIVATIZE_LOAD);
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

SYSTX_EXPORT
int
strncmp_tx(const char* s1, const char* s2, size_t n)
{
    privatize_c_tx(s1, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    privatize_c_tx(s2, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    return strncmp(s1, s2, n);
}

SYSTX_EXPORT
char*
strncpy_tx(char* restrict dest, const char* restrict src, size_t n)
{
    privatize_c_tx(src, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    size_t len = strlen(src) + 1;
    if (len < n) {
        store_tx(dest, src, len);
    } else {
        store_tx(dest, src, n);
    }
    return dest;
}

SYSTX_EXPORT
char*
strndup_tx(const char* s, size_t n)
{
    privatize_c_tx(s, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    size_t len = strlen(s) + 1;
    if (n < len) {
        len = n + 1;
    }
    char* mem = malloc_tx(len);
    if (!mem) {
        return NULL;
    }
    memcpy(mem, s, len);
    if (n < len) {
        mem[n + 1] = '\0';
    }
    return mem;
}

SYSTX_EXPORT
size_t
strnlen_tx(const char* s, size_t maxlen)
{
    privatize_c_tx(s, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    return strnlen(s, maxlen);
}

SYSTX_EXPORT
char*
strpbrk_tx(const char* s, const char* accept)
{
    privatize_c_tx(s, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    privatize_c_tx(accept, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    return strpbrk(s, accept);
}

SYSTX_EXPORT
char*
strrchr_tx(const char* s, int c)
{
    privatize_c_tx(s, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    return strrchr(s, c);
}

SYSTX_EXPORT
size_t
strspn_tx(const char* s, const char* accept)
{
    privatize_c_tx(s, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    privatize_c_tx(accept, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    return strspn(s, accept);
}

SYSTX_EXPORT
char*
strstr_tx(const char* haystack, const char* needle)
{
    privatize_c_tx(haystack, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    privatize_c_tx(needle, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    return strstr(haystack, needle);
}

SYSTX_EXPORT
char*
strtok_r_tx(char* restrict str, const char* restrict delim,
            char** restrict saveptr)
{
    privatize_c_tx(str, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    privatize_c_tx(delim, '\0', SYSTX_TM_PRIVATIZE_LOAD);

    char* ptr = load_ptr_tx(saveptr);
    char* tok = strtok_r(str, delim, &ptr);
    store_ptr_tx(saveptr, &ptr);

    return tok;
}
