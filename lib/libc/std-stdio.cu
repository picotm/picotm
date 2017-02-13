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

ceuta_hdrl(#ifndef TANGER_STM_STD_STDIO_H);
ceuta_hdrl(#define TANGER_STM_STD_STDIO_H);
ceuta_hdrl(#include <stdio.h>);

#include <stdio.h>

ceuta_pure(void, perror, perror, const char *str);
ceuta_excl(int,  rename, rename, const char *new, const char *old);

ceuta_hdrl(static int tanger_wrapperpure_tanger_stm_std_fprintf(FILE*, const char*, ...) __attribute__ ((weakref("fprintf"))););

ceuta_hdrl(#endif);

