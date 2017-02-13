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

ceuta_hdrl(#ifndef TANGER_STM_STD_FCNTL_H);
ceuta_hdrl(#define TANGER_STM_STD_FCNTL_H);
ceuta_hdrl(#include <fcntl.h>);

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "types.h"
#include "fd/fcntlop.h"

ceuta_hdrl(static int tanger_wrapper_tanger_stm_std_open(const char *path, int oflag, ...) __attribute__ ((weakref("open"))););

extern int
tanger_stm_std_open(const char *path, int oflag, ...)
{
    extern void* com_alloc_tx_malloc(size_t);
    extern void  com_alloc_tx_free(void*);
    extern int   com_fd_tx_open(const char*, int, mode_t);

    /* Load path */

    char *path2;

    if (path) {
        tanger_stm_tx_t *tx = tanger_stm_get_tx();
        assert(tx);

        unsigned int len;
        void *tmp = tanger_stm_loadregionstring(tx, (char*)path, &len);
        assert(tmp);
        ++len; /* Workaround length bug in tanger_stm_loadregionstring. */
        path2 = com_alloc_tx_malloc(len);
        if (!path2) {
            perror("malloc");
            abort();
        }
        memcpy(path2, tmp, len);
        tanger_stm_loadregionpost(tx, (uint8_t*)path, len);
    } else {
        path2 = NULL;
    }

    /* Lookup mode */

    mode_t mode = 0;

    if (oflag&O_CREAT) {
        va_list arg;
        va_start(arg, oflag);
        mode = va_arg(arg, int);
        va_end(arg);
    }

    int fildes = com_fd_tx_open(path2, oflag, mode);

    if (path2) {
        com_alloc_tx_free(path2);
    }

    return fildes;
}

ceuta_decl(int, creat, creat, const char *path, mode_t mode);

extern int
tanger_stm_std_creat(const char *path, mode_t mode)
{
    return tanger_stm_std_open(path, O_WRONLY|O_CREAT|O_TRUNC, mode);
}

ceuta_hdrl(static int tanger_wrapper_tanger_stm_std_fcntl(int fildes, int cmd, ...) __attribute__ ((weakref("fcntl"))););

extern int
tanger_stm_std_fcntl(int fildes, int cmd, ...)
{
    extern int com_fd_tx_fcntl(int, int, void*);
    extern int com_fd_tx_dup_internal(int, int);

    int res;

    switch (cmd) {
        case F_DUPFD:
            /* Handle like dup */
            res = com_fd_tx_dup_internal(fildes, 0);
            break;
        case F_DUPFD_CLOEXEC:
            /* Handle like dup+CLOEXEC */
            res = com_fd_tx_dup_internal(fildes, 1);
            break;
        case F_SETFD:
        case F_SETFL:
        case F_SETOWN:
            {
                union com_fd_fcntl_arg val;
                va_list arg;
                va_start(arg, cmd);
                val.arg0 = va_arg(arg, int);
                va_end(arg);

                res = com_fd_tx_fcntl(fildes, cmd, &val);
            }
            break;
        case F_GETLK:
        case F_SETLK:
        case F_SETLKW:
            {
                union com_fd_fcntl_arg val;
                struct flock *f;
                va_list arg;
                va_start(arg, cmd);
                f = va_arg(arg, struct flock*);
                va_end(arg);

                memcpy(&val.arg1, f, sizeof(val.arg1));

                res = com_fd_tx_fcntl(fildes, cmd, &val);
            }
            break;
        default:
            res = com_fd_tx_fcntl(fildes, cmd, NULL);
            break;
    }

    return res;
}

ceuta_hdrl(#endif);

