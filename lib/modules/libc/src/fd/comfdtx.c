/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <picotm/picotm.h>
#include "errcode.h"
#include "types.h"
#include "counter.h"
#include "pgtree.h"
#include "pgtreess.h"
#include "cmap.h"
#include "cmapss.h"
#include "rwlock.h"
#include "rwlockmap.h"
#include "rwstatemap.h"
#include "fcntlop.h"
#include "ofdtx.h"
#include "fdtx.h"
#include "comfd.h"
#include "comfdtx.h"

static int
com_fd_tx_lock(void *data)
{
    return com_fd_lock(data);
}

static int
com_fd_tx_unlock(void *data)
{
    com_fd_unlock(data);

    return 0;
}

static int
com_fd_tx_validate(void *data, int noundo)
{
    return com_fd_validate(data, noundo);
}

static int
com_fd_tx_apply_event(const struct event *event, size_t n, void *data)
{
    return com_fd_apply_event(data, event, n);
}

static int
com_fd_tx_undo_event(const struct event *event, size_t n, void *data)
{
    return com_fd_undo_event(data, event, n);
}

static int
com_fd_tx_updatecc(void *data, int noundo)
{
    return com_fd_updatecc(data, noundo);
}

static int
com_fd_tx_clearcc(void *data, int noundo)
{
    return com_fd_clearcc(data, noundo);
}

static int
com_fd_tx_finish(void *data)
{
    com_fd_finish(data);

    return 0;
}

static int
com_fd_tx_uninit(void *data)
{
    com_fd_uninit(data);

    return 0;
}

struct com_fd *
com_fd_tx_aquire_data()
{
    static __thread struct {
        bool          is_initialized;
        struct com_fd instance;
    } t_com_fd;

    if (t_com_fd.is_initialized) {
        return &t_com_fd.instance;
    }

    long res = picotm_register_module(com_fd_tx_lock,
                                     com_fd_tx_unlock,
                                     com_fd_tx_validate,
                                     com_fd_tx_apply_event,
                                     com_fd_tx_undo_event,
                                     com_fd_tx_updatecc,
                                     com_fd_tx_clearcc,
                                     com_fd_tx_finish,
                                     com_fd_tx_uninit,
                                     &t_com_fd.instance);
    if (res < 0) {
        return NULL;
    }
    unsigned long module = res;

    res = com_fd_init(&t_com_fd.instance, module);
    if (res < 0) {
        return NULL;
    }

    t_com_fd.is_initialized = true;

    return &t_com_fd.instance;
}

int
com_fd_tx_accept(int sockfd, struct sockaddr *address, socklen_t *address_len)
{
    extern int com_fd_exec_accept(struct com_fd*, int, struct sockaddr*, socklen_t*);

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    int res;

    do {
        res = com_fd_exec_accept(comfd, sockfd, address, address_len);

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

int
com_fd_tx_bind(int sockfd, const struct sockaddr *address,
                                         socklen_t address_len)
{
    extern int com_fd_exec_bind(struct com_fd*, int,
                                                const struct sockaddr*,
                                                socklen_t, int);

    int res;

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    do {
        res = com_fd_exec_bind(comfd, sockfd, address, address_len,
                               picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

int
com_fd_tx_close(int fildes)
{
    extern int com_fd_exec_close(struct com_fd*, int, int);

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    int res;

    do {
        res = com_fd_exec_close(comfd, fildes, picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

int
com_fd_tx_connect(int sockfd, const struct sockaddr *serv_addr,
                                               socklen_t addr_len)
{
    extern int com_fd_exec_connect(struct com_fd*, int,
                                                   const struct sockaddr*,
                                                   socklen_t, int);

    int res;

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    do {
        res = com_fd_exec_connect(comfd, sockfd, serv_addr, addr_len,
                                  picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

int
com_fd_tx_dup_internal(int fildes, int cloexec)
{
    extern int com_fd_exec_dup(struct com_fd*, int, int);

    int res;

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    do {
        res = com_fd_exec_dup(comfd, fildes, cloexec);

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

int
com_fd_tx_dup(int fildes)
{
    return com_fd_tx_dup_internal(fildes, 0);
}

int
com_fd_tx_fcntl(int fildes, int cmd, union com_fd_fcntl_arg *arg)
{
    extern int com_fd_exec_fcntl(struct com_fd*, int, int, union com_fd_fcntl_arg*, int);

    int res;

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    do {
        res = com_fd_exec_fcntl(comfd, fildes, cmd, arg, picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

int
com_fd_tx_fsync(int fildes)
{
    extern int com_fd_exec_fsync(struct com_fd*, int, int);

    int res;

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    do {
        res = com_fd_exec_fsync(comfd, fildes, picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

int
com_fd_tx_listen(int sockfd, int backlog)
{
    extern int com_fd_exec_listen(struct com_fd*, int, int, int);

    int res;

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    do {
        res = com_fd_exec_listen(comfd, sockfd, backlog, picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

off_t
com_fd_tx_lseek(int fildes, off_t offset, int whence)
{
    extern off_t com_fd_exec_lseek(struct com_fd*, int, off_t, int, int);

    off_t res;

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    do {
        res = com_fd_exec_lseek(comfd, fildes, offset, whence, picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

int
com_fd_tx_open(const char *path, int oflag, mode_t mode)
{
    extern int com_fd_exec_open(struct com_fd*, const char*, int, mode_t, int);

    int res;

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    do {
        res = com_fd_exec_open(comfd, path, oflag, mode, picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

int
com_fd_tx_pipe(int pipefd[2])
{
    extern int com_fd_exec_pipe(struct com_fd*, int[2]);

    int res;

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    do {
        res = com_fd_exec_pipe(comfd, pipefd);

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

ssize_t
com_fd_tx_pread(int fildes, void *buf, size_t nbyte, off_t off)
{
    extern ssize_t com_fd_exec_pread(struct com_fd*, int, void*, size_t, off_t, int);

    ssize_t res;

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    do {
        res = com_fd_exec_pread(comfd, fildes, buf, nbyte, off,
                                picotm_is_irrevocable());

        /* possibly validate all optimistic domains */
        if ((com_fd_get_validation_mode(comfd) == PICOTM_LIBC_VALIDATE_FULL)
            && com_fd_get_optcc(comfd)
            && !picotm_is_valid()) {
            res = ERR_CONFLICT;
        }

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

ssize_t
com_fd_tx_pwrite(int fildes, const void *buf, size_t nbyte, off_t off)
{
    extern ssize_t com_fd_exec_pwrite(struct com_fd*, int,
                                                      const void*,
                                                      size_t,
                                                      off_t, int);

    ssize_t res;

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    do {
        res = com_fd_exec_pwrite(comfd, fildes, buf, nbyte, off,
                                 picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

ssize_t
com_fd_tx_read(int fildes, void *buf, size_t nbyte)
{
    extern ssize_t com_fd_exec_read(struct com_fd*, int, void*, size_t, int);

    ssize_t res;

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    do {
        res = com_fd_exec_read(comfd, fildes, buf, nbyte,
                               picotm_is_irrevocable());

        /* possibly validate all optimistic domains */
        if ((com_fd_get_validation_mode(comfd) == PICOTM_LIBC_VALIDATE_FULL)
            && com_fd_get_optcc(comfd)
            && !picotm_is_valid()) {
            res = ERR_CONFLICT;
        }

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

ssize_t
com_fd_tx_recv(int sockfd, void *buffer, size_t length, int flags)
{
    extern ssize_t com_fd_exec_recv(struct com_fd*, int, void*, size_t, int, int);

    ssize_t res;

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    do {
        res = com_fd_exec_recv(comfd, sockfd, buffer, length, flags,
                               picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

int
com_fd_tx_select(int nfds, fd_set *readfds,
                               fd_set *writefds,
                               fd_set *errorfds, struct timeval *timeout)
{
    extern int com_fd_exec_select(struct com_fd*, int, fd_set*,
                                                       fd_set*,
                                                       fd_set*,
                                                       struct timeval*,
                                                       int);

    int res;

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    do {
        res = com_fd_exec_select(comfd, nfds, readfds,
                                              writefds,
                                              errorfds,
                                              timeout,
                                              picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

ssize_t
com_fd_tx_send(int fildes, const void *buffer, size_t length, int flags)
{
    extern ssize_t com_fd_exec_send(struct com_fd*, int, const void*, size_t, int, int);

    ssize_t res;

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    do {
        res = com_fd_exec_send(comfd, fildes, buffer, length, flags,
                               picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

int
com_fd_tx_shutdown(int sockfd, int how)
{
    extern int com_fd_exec_shutdown(struct com_fd*, int, int, int);

    int res;

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    do {
        res = com_fd_exec_shutdown(comfd, sockfd, how, picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

int
com_fd_tx_socket(int domain, int type, int protocol)
{
    extern int com_fd_exec_socket(struct com_fd*, int, int, int);

    int res;

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    do {
        res = com_fd_exec_socket(comfd, domain, type, protocol);

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

void
com_fd_tx_sync(void)
{
    extern void com_fd_exec_sync(struct com_fd*);

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    com_fd_exec_sync(comfd);
}

ssize_t
com_fd_tx_write(int fildes, const void *buf, size_t nbyte)
{
    extern ssize_t com_fd_exec_write(struct com_fd*, int, const void*, size_t, int);

    ssize_t res;

    struct com_fd *comfd = com_fd_tx_aquire_data();
    assert(comfd);

    do {
        res = com_fd_exec_write(comfd, fildes, buf, nbyte,
                                picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

