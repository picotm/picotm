/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systx/string-tm.h"
#include "alloc/comalloctx.h"

/*
 * Memory functions
 */

SYSTX_EXPORT
void*
memccpy_tm(void* restrict dest, const void* restrict src, int c, size_t n)
{
    return memccpy(dest, src, c, n);
}

SYSTX_EXPORT
void*
memchr_tm(const void* s, int c, size_t n)
{
    return memchr(s, c, n);
}

SYSTX_EXPORT
int
memcmp_tm(const void* s1, const void* s2, size_t n)
{
    return memcmp(s1, s2, n);
}

SYSTX_EXPORT
void*
memcpy_tm(void* restrict dest, const void* restrict src, size_t n)
{
    return memcpy(dest, src, n);
}

SYSTX_EXPORT
void*
memmove_tm(void* dest, const void* src, size_t n)
{
    return memmove(dest, src, n);
}

SYSTX_EXPORT
void*
memset_tm(void* s, int c, size_t n)
{
    return memset(s, c, n);
}

SYSTX_EXPORT
void*
memrchr_tm(const void* s, int c, size_t n)
{
    return memrchr(s, c, n);
}

SYSTX_EXPORT
void*
rawmemchr_tm(const void* s, int c)
{
    return rawmemchr(s, c);
}

/*
 * String functions
 */

SYSTX_EXPORT
char*
stpcpy_tm(char* restrict dest, const char* restrict src)
{
    return stpcpy(dest, src);
}

SYSTX_EXPORT
char*
stpncpy_tm(char* restrict dest, const char* restrict src, size_t n)
{
    return stpncpy(dest, src, n);
}

SYSTX_EXPORT
char*
strcat_tm(char* restrict dest, const char* restrict src)
{
    return strcat(dest, src);
}

SYSTX_EXPORT
char*
strchr_tm(const char* s, int c)
{
    return strchr(s, c);
}

SYSTX_EXPORT
int
strcmp_tm(const char* s1, const char* s2)
{
    return strcmp(s1, s2);
}

SYSTX_EXPORT
int
strcoll_l_tm(const char* s1, const char* s2, locale_t locale)
{
    return strcoll_l(s1, s2, locale);
}

SYSTX_EXPORT
char*
strcpy_tm(char* restrict dest, const char* restrict src)
{
    return strcpy(dest, src);
}

SYSTX_EXPORT
size_t
strcspn_tm(const char* s, const char* reject)
{
    return strcspn(s, reject);
}

SYSTX_EXPORT
char*
strdup_tm(const char* s)
{
    size_t len = strlen(s) + 1;
    char* mem = com_alloc_tx_malloc(len);
    if (!mem) {
        return NULL;
    }
    return memcpy(mem, s, len);
}

SYSTX_EXPORT
int
strerror_r_tm(int errnum, char* buf, size_t buflen)
{
#if (_POSIX_C_SOURCE >= 200112L) && !_GNU_SOURCE
    return strerror_r(errnum, buf, buflen);
#else
    strerror_r(errnum, buf, buflen);
    return 0;
#endif
}

SYSTX_EXPORT
size_t
strlen_tm(const char* s)
{
    return strlen(s);
}

SYSTX_EXPORT
char*
strncat_tm(char* restrict dest, const char* restrict src, size_t n)
{
    return strncat(dest, src, n);
}

SYSTX_EXPORT
int
strncmp_tm(const char* s1, const char* s2, size_t n)
{
    return strncmp(s1, s2, n);
}

SYSTX_EXPORT
char*
strncpy_tm(char* restrict dest, const char* restrict src, size_t n)
{
    return strncpy(dest, src, n);
}

SYSTX_EXPORT
char*
strndup_tm(const char* s, size_t n)
{
    size_t len = strlen(s) + 1;
    if (n < len) {
        len = n + 1;
    }
    char* mem = com_alloc_tx_malloc(len);
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
strnlen_tm(const char* s, size_t maxlen)
{
    return strnlen(s, maxlen);
}

SYSTX_EXPORT
char*
strpbrk_tm(const char* s, const char* accept)
{
    return strpbrk(s, accept);
}

SYSTX_EXPORT
char*
strrchr_tm(const char* s, int c)
{
    return strrchr(s, c);
}

SYSTX_EXPORT
size_t
strspn_tm(const char* s, const char* accept)
{
    return strspn(s, accept);
}

SYSTX_EXPORT
char*
strstr_tm(const char* haystack, const char* needle)
{
    return strstr(haystack, needle);
}

SYSTX_EXPORT
char*
strtok_r_tm(char* restrict str, const char* restrict delim,
            char** restrict saveptr)
{
    return strtok_r(str, delim, saveptr);
}
