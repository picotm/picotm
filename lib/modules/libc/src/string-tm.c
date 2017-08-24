/* Permission is hereby granted, free of charge, to any person obtaining a
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
 */

#include "picotm/string-tm.h"
#include <errno.h>
#include <picotm/picotm-module.h>
#include "allocator/module.h"
#include "error/module.h"

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
    error_module_save_errno();

    size_t len = strlen(s) + sizeof(*s);

    void* mem;
    allocator_module_posix_memalign(&mem, 2 * sizeof(void*), len);

    return memcpy(mem, s, len);
}

#define STRERROR_MAXLEN 64 /* must be large enough to hold any error string */

PICOTM_EXPORT
char*
__strerror_r_gnu_tm(int errnum, char* buf, size_t buflen)
{
#if (_POSIX_C_SOURCE >= 200112L) && !_GNU_SOURCE
    error_module_save_errno();

    char tmpbuf[STRERROR_MAXLEN];
    int res;
    do {
        res = strerror_r(errnum, tmpbuf, sizeof(tmplen));
        if (res < 0) {
            /* glibc 2.12 and earlier return the error code in errno. */
            picotm_recover_from_errno(errno);
        } else if (res > 0) {
            /* glibc 2.13 and later return the error code as result. */
            picotm_recover_from_errno(res);
        }
    } while (res); // error if res != 0 handled older and newer glibc
    if (buflen) {
        size_t tlen = strlen(tmpbuf);
        size_t blen = buflen - 1;
        size_t slen = blen < tlen ? blen : tlen;
        memcpy(buf, tmpbuf, slen);
        buf[slen] = '\0';
    }
    char* str = buf;
#else
    char* str;
    do {
        str = strerror_r(errnum, buf, buflen);
        if (!str) {
            picotm_recover_from_errno(errno);
        }
    } while (!str);
#endif

    return str;
}

PICOTM_EXPORT
int
__strerror_r_posix_tm(int errnum, char* buf, size_t buflen)
{
    error_module_save_errno();

    int res = 0;

#if (_POSIX_C_SOURCE >= 200112L) && !_GNU_SOURCE
    do {
        res = strerror_r(errnum, buf, buflen);
        if (res < 0) {
            /* glibc 2.12 and earlier return the error code in errno. */
            picotm_recover_from_errno(errno);
            return
        } else if (res > 0) {
            /* glibc 2.13 and later return the error code as result. */
            picotm_recover_from_errno(res);
        }
    } while (res); // error if res != 0 handled older and newer glibc
    char* str = buf;
#else
    char* str;
    do {
        str = strerror_r(errnum, buf, buflen);
        if (!str) {
            picotm_recover_from_errno(errno);
        }
    } while (!str);
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
    error_module_save_errno();

    size_t len = strlen(s) + sizeof(*s);
    if (n < len) {
        len = n + sizeof(*s);
    }

    void* mem;
    allocator_module_posix_memalign(&mem, 2 * sizeof(void*), len);

    char* s2 = memcpy(mem, s, len);
    if (n < len) {
        s2[n + 1] = '\0';
    }
    return s2;
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
