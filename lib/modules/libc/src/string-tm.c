/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/string-tm.h"
#include <errno.h>
#include <picotm/picotm-module.h>
#include "alloc/comalloctx.h"
#include "picotm/picotm-libc.h"

/*
 * Memory functions
 */

PICOTM_EXPORT
void*
memccpy_tm(void* restrict dest, const void* restrict src, int c, size_t n)
{
    return memccpy(dest, src, c, n);
}

PICOTM_EXPORT
void*
memchr_tm(const void* s, int c, size_t n)
{
    return memchr(s, c, n);
}

PICOTM_EXPORT
int
memcmp_tm(const void* s1, const void* s2, size_t n)
{
    return memcmp(s1, s2, n);
}

PICOTM_EXPORT
void*
memcpy_tm(void* restrict dest, const void* restrict src, size_t n)
{
    return memcpy(dest, src, n);
}

PICOTM_EXPORT
void*
memmove_tm(void* dest, const void* src, size_t n)
{
    return memmove(dest, src, n);
}

PICOTM_EXPORT
void*
memset_tm(void* s, int c, size_t n)
{
    return memset(s, c, n);
}

PICOTM_EXPORT
void*
memrchr_tm(const void* s, int c, size_t n)
{
    return memrchr(s, c, n);
}

PICOTM_EXPORT
void*
rawmemchr_tm(const void* s, int c)
{
    return rawmemchr(s, c);
}

/*
 * String functions
 */

PICOTM_EXPORT
char*
stpcpy_tm(char* restrict dest, const char* restrict src)
{
    return stpcpy(dest, src);
}

PICOTM_EXPORT
char*
stpncpy_tm(char* restrict dest, const char* restrict src, size_t n)
{
    return stpncpy(dest, src, n);
}

PICOTM_EXPORT
char*
strcat_tm(char* restrict dest, const char* restrict src)
{
    return strcat(dest, src);
}

PICOTM_EXPORT
char*
strchr_tm(const char* s, int c)
{
    return strchr(s, c);
}

PICOTM_EXPORT
int
strcmp_tm(const char* s1, const char* s2)
{
    return strcmp(s1, s2);
}

PICOTM_EXPORT
int
strcoll_l_tm(const char* s1, const char* s2, locale_t locale)
{
    return strcoll_l(s1, s2, locale);
}

PICOTM_EXPORT
char*
strcpy_tm(char* restrict dest, const char* restrict src)
{
    return strcpy(dest, src);
}

PICOTM_EXPORT
size_t
strcspn_tm(const char* s, const char* reject)
{
    return strcspn(s, reject);
}

PICOTM_EXPORT
char*
strdup_tm(const char* s)
{
    picotm_libc_save_errno();

    size_t len = strlen(s) + sizeof(*s);

    char* mem;

    do {
        mem = com_alloc_tx_malloc(len);
        if (!mem) {
            picotm_recover_from_errno(errno);
        }
    } while (!mem);

    return memcpy(mem, s, len);
}

PICOTM_EXPORT
int
strerror_r_tm(int errnum, char* buf, size_t buflen)
{
    picotm_libc_save_errno();

    int res;

#if (_POSIX_C_SOURCE >= 200112L) && !_GNU_SOURCE
    do {
        res = strerror_r(errnum, buf, buflen);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);
#else
    const char* str;
    do {
        str = strerror_r(errnum, buf, buflen);
        if (!str) {
            picotm_recover_from_errno(errno);
        }
    } while (!str);
    res = 0;
#endif

    return res;
}

PICOTM_EXPORT
size_t
strlen_tm(const char* s)
{
    return strlen(s);
}

PICOTM_EXPORT
char*
strncat_tm(char* restrict dest, const char* restrict src, size_t n)
{
    return strncat(dest, src, n);
}

PICOTM_EXPORT
int
strncmp_tm(const char* s1, const char* s2, size_t n)
{
    return strncmp(s1, s2, n);
}

PICOTM_EXPORT
char*
strncpy_tm(char* restrict dest, const char* restrict src, size_t n)
{
    return strncpy(dest, src, n);
}

PICOTM_EXPORT
char*
strndup_tm(const char* s, size_t n)
{
    picotm_libc_save_errno();

    size_t len = strlen(s) + sizeof(*s);
    if (n < len) {
        len = n + sizeof(*s);
    }

    char* mem;

    do {
        mem = com_alloc_tx_malloc(len);
        if (!mem) {
            picotm_recover_from_errno(errno);
        }
    } while (!mem);

    memcpy(mem, s, len);
    if (n < len) {
        mem[n + 1] = '\0';
    }
    return mem;
}

PICOTM_EXPORT
size_t
strnlen_tm(const char* s, size_t maxlen)
{
    return strnlen(s, maxlen);
}

PICOTM_EXPORT
char*
strpbrk_tm(const char* s, const char* accept)
{
    return strpbrk(s, accept);
}

PICOTM_EXPORT
char*
strrchr_tm(const char* s, int c)
{
    return strrchr(s, c);
}

PICOTM_EXPORT
size_t
strspn_tm(const char* s, const char* accept)
{
    return strspn(s, accept);
}

PICOTM_EXPORT
char*
strstr_tm(const char* haystack, const char* needle)
{
    return strstr(haystack, needle);
}

PICOTM_EXPORT
char*
strtok_r_tm(char* restrict str, const char* restrict delim,
            char** restrict saveptr)
{
    return strtok_r(str, delim, saveptr);
}
