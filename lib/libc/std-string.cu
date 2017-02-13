/* Copyright (C) 2009  Thomas Zimmermann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

ceuta_hdrl(#ifndef TANGER_STM_STD_STRING_H);
ceuta_hdrl(#define TANGER_STM_STD_STRING_H);
ceuta_hdrl(#include <string.h>);

#include <string.h>

ceuta_wrap([use=s1] void*, memccpy, memccpy, [siz=n|out]       void *s1, [siz=n|in] const void *s2, int c, size_t n);
ceuta_wrap([use=s1] void*, memchr,  memchr,  [siz=n|in]  const void *s1,                            int c, size_t n);
ceuta_wrap(         int,   memcmp,  memcmp,  [siz=n|in]  const void *s1, [siz=n|in] const void *s2, size_t n);
ceuta_wrap([use=s1] void*, memcpy,  memcpy,  [siz=n|out]       void *s1, [siz=n|in] const void *s2, size_t n);
ceuta_wrap([use=s1] void*, memmove, memmove, [siz=n|out]       void *s1, [siz=n|in] const void *s2, size_t n);
ceuta_wrap([use=s]  void*, memset,  memset,  [siz=n|out]       void *s,                             int c, size_t n);

ceuta_wrap(         int,    strcasecmp, strcasecmp, [cstring|in]  const char *s1, [cstring|in] const char *s2);
ceuta_wrap([use=s1] char*,  strcpy,     strcpy,     [sizof=s2|out]      char *s1, [cstring|in] const char *s2);
ceuta_wrap(         size_t, strlen,     strlen,     [cstring|in]  const char *s);

ceuta_hdrl(#endif);

