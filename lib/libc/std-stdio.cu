/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

ceuta_hdrl(#ifndef TANGER_STM_STD_STDIO_H);
ceuta_hdrl(#define TANGER_STM_STD_STDIO_H);
ceuta_hdrl(#include <stdio.h>);

#include <stdio.h>

ceuta_pure(void, perror, perror, const char *str);
ceuta_excl(int,  rename, rename, const char *new, const char *old);

ceuta_hdrl(static int tanger_wrapperpure_tanger_stm_std_fprintf(FILE*, const char*, ...) __attribute__ ((weakref("fprintf"))););

ceuta_hdrl(#endif);

