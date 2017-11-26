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

#if defined(PICOTM_LIBC_HAVE_MEMCCPY) && PICOTM_LIBC_HAVE_MEMCCPY
PICOTM_EXPORT
void*
memccpy_tm(void* restrict dest, const void* restrict src, int c, size_t n)
{
    return memccpy(dest, src, c, n);
}
#endif

#if defined(PICOTM_LIBC_HAVE_MEMCHR) && PICOTM_LIBC_HAVE_MEMCHR
PICOTM_EXPORT
void*
memchr_tm(const void* s, int c, size_t n)
{
    return memchr(s, c, n);
}
#endif

#if defined(PICOTM_LIBC_HAVE_MEMCMP) && PICOTM_LIBC_HAVE_MEMCMP
PICOTM_EXPORT
int
memcmp_tm(const void* s1, const void* s2, size_t n)
{
    return memcmp(s1, s2, n);
}
#endif

#if defined(PICOTM_LIBC_HAVE_MEMCPY) && PICOTM_LIBC_HAVE_MEMCPY
PICOTM_EXPORT
void*
memcpy_tm(void* restrict dest, const void* restrict src, size_t n)
{
    return memcpy(dest, src, n);
}
#endif

#if defined(PICOTM_LIBC_HAVE_MEMMOVE) && PICOTM_LIBC_HAVE_MEMMOVE
PICOTM_EXPORT
void*
memmove_tm(void* dest, const void* src, size_t n)
{
    return memmove(dest, src, n);
}
#endif

#if defined(PICOTM_LIBC_HAVE_MEMSET) && PICOTM_LIBC_HAVE_MEMSET
PICOTM_EXPORT
void*
memset_tm(void* s, int c, size_t n)
{
    return memset(s, c, n);
}
#endif

#if defined(PICOTM_LIBC_HAVE_MEMRCHR) && PICOTM_LIBC_HAVE_MEMRCHR
PICOTM_EXPORT
void*
memrchr_tm(const void* s, int c, size_t n)
{
    return memrchr(s, c, n);
}
#endif

#if defined(PICOTM_LIBC_HAVE_RAWMEMCHR) && PICOTM_LIBC_HAVE_RAWMEMCHR
PICOTM_EXPORT
void*
rawmemchr_tm(const void* s, int c)
{
    return rawmemchr(s, c);
}
#endif

/*
 * String functions
 */

#if defined(PICOTM_LIBC_HAVE_STPCPY) && PICOTM_LIBC_HAVE_STPCPY
PICOTM_EXPORT
char*
stpcpy_tm(char* restrict dest, const char* restrict src)
{
    return stpcpy(dest, src);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STPNCPY) && PICOTM_LIBC_HAVE_STPNCPY
PICOTM_EXPORT
char*
stpncpy_tm(char* restrict dest, const char* restrict src, size_t n)
{
    return stpncpy(dest, src, n);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRCAT) && PICOTM_LIBC_HAVE_STRCAT
PICOTM_EXPORT
char*
strcat_tm(char* restrict dest, const char* restrict src)
{
    return strcat(dest, src);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRCHR) && PICOTM_LIBC_HAVE_STRCHR
PICOTM_EXPORT
char*
strchr_tm(const char* s, int c)
{
    return strchr(s, c);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRCMP) && PICOTM_LIBC_HAVE_STRCMP
PICOTM_EXPORT
int
strcmp_tm(const char* s1, const char* s2)
{
    return strcmp(s1, s2);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRCOLL_L) && PICOTM_LIBC_HAVE_STRCOLL_L
PICOTM_EXPORT
int
strcoll_l_tm(const char* s1, const char* s2, locale_t locale)
{
    return strcoll_l(s1, s2, locale);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRCPY) && PICOTM_LIBC_HAVE_STRCPY
PICOTM_EXPORT
char*
strcpy_tm(char* restrict dest, const char* restrict src)
{
    return strcpy(dest, src);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRCSPN) && PICOTM_LIBC_HAVE_STRCSPN
PICOTM_EXPORT
size_t
strcspn_tm(const char* s, const char* reject)
{
    return strcspn(s, reject);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRDUP) && PICOTM_LIBC_HAVE_STRDUP
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
#endif

#if defined(PICOTM_LIBC_HAVE_STRERROR_R) && PICOTM_LIBC_HAVE_STRERROR_R

#define STRERROR_MAXLEN 64 /* must be large enough to hold any error string */

PICOTM_EXPORT
char*
__strerror_r_gnu_tm(int errnum, char* buf, size_t buflen)
{
#if defined(__GNU_LIBRARY__) && \
    (defined(_GNU_SOURCE) || (_POSIX_C_SOURCE < 200112L))
    char* str;
    do {
        str = strerror_r(errnum, buf, buflen);
        if (!str) {
            picotm_recover_from_errno(errno);
        }
    } while (!str);
#else
    error_module_save_errno();

    char tmpbuf[STRERROR_MAXLEN];
    int res;
    do {
        res = strerror_r(errnum, tmpbuf, sizeof(tmpbuf));
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
#endif

    return str;
}

PICOTM_EXPORT
int
__strerror_r_posix_tm(int errnum, char* buf, size_t buflen)
{
    /* ERANGE is not an error, but signals a too-small buffer. We
     * return it as status code to the transaction's execution phase.
     */

    int res = 0;

#if defined(__GNU_LIBRARY__) && \
    (defined(_GNU_SOURCE) || (_POSIX_C_SOURCE < 200112L))
    char tmpbuf[STRERROR_MAXLEN];
    char* str;
    do {
        str = strerror_r(errnum, tmpbuf, sizeof(tmpbuf));
        if (!str) {
            picotm_recover_from_errno(errno);
        }
        if (str == tmpbuf) {
            /* GNU's strerror_l only uses the buffer if an unknown
             * error code was given. */
            picotm_recover_from_errno(EINVAL);
        }
    } while (str == buf);
    size_t slen = strlen(str);
    if (slen >= buflen) {
        return ERANGE; /* output buffer is too small */
    }
    memcpy(buf, str, slen);
    buf[slen] = '\0';
#else
    error_module_save_errno();

    do {
        res = strerror_r(errnum, buf, buflen);
        if (res < 0) {
            /* glibc 2.12 and earlier return the error code in errno. */
            if (errno == ERANGE) {
                return res;
            }
            picotm_recover_from_errno(errno);
        } else if (res > 0) {
            /* glibc 2.13 and later return the error code as result. */
            if (res == ERANGE) {
                return res;
            }
            picotm_recover_from_errno(res);
        }
    } while (res); // error if res != 0 handled older and newer glibc
    char* str = buf;
#endif

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRLEN) && PICOTM_LIBC_HAVE_STRLEN
PICOTM_EXPORT
size_t
strlen_tm(const char* s)
{
    return strlen(s);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRNCAT) && PICOTM_LIBC_HAVE_STRNCAT
PICOTM_EXPORT
char*
strncat_tm(char* restrict dest, const char* restrict src, size_t n)
{
    return strncat(dest, src, n);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRNCMP) && PICOTM_LIBC_HAVE_STRNCMP
PICOTM_EXPORT
int
strncmp_tm(const char* s1, const char* s2, size_t n)
{
    return strncmp(s1, s2, n);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRNCPY) && PICOTM_LIBC_HAVE_STRNCPY
PICOTM_EXPORT
char*
strncpy_tm(char* restrict dest, const char* restrict src, size_t n)
{
    return strncpy(dest, src, n);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRNDUP) && PICOTM_LIBC_HAVE_STRNDUP
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
#endif

#if defined(PICOTM_LIBC_HAVE_STRNLEN) && PICOTM_LIBC_HAVE_STRNLEN
PICOTM_EXPORT
size_t
strnlen_tm(const char* s, size_t maxlen)
{
    return strnlen(s, maxlen);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRPBRK) && PICOTM_LIBC_HAVE_STRPBRK
PICOTM_EXPORT
char*
strpbrk_tm(const char* s, const char* accept)
{
    return strpbrk(s, accept);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRRCHR) && PICOTM_LIBC_HAVE_STRRCHR
PICOTM_EXPORT
char*
strrchr_tm(const char* s, int c)
{
    return strrchr(s, c);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRSPN) && PICOTM_LIBC_HAVE_STRSPN
PICOTM_EXPORT
size_t
strspn_tm(const char* s, const char* accept)
{
    return strspn(s, accept);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRSTR) && PICOTM_LIBC_HAVE_STRSTR
PICOTM_EXPORT
char*
strstr_tm(const char* haystack, const char* needle)
{
    return strstr(haystack, needle);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRTOK_R) && PICOTM_LIBC_HAVE_STRTOK_R
PICOTM_EXPORT
char*
strtok_r_tm(char* restrict str, const char* restrict delim,
            char** restrict saveptr)
{
    return strtok_r(str, delim, saveptr);
}
#endif
