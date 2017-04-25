/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "module.h"
#include <assert.h>
#include <picotm/picotm.h>
#include "errcode.h"
#include "comfd.h"

struct fd_module {
    bool          is_initialized;
    struct com_fd instance;
};

static int
lock_cb(void* data)
{
    struct fd_module* module = data;

    return com_fd_lock(&module->instance);
}

static int
unlock_cb(void* data)
{
    struct fd_module* module = data;

    com_fd_unlock(&module->instance);

    return 0;
}

static int
validate_cb(void* data, int noundo)
{
    struct fd_module* module = data;

    return com_fd_validate(&module->instance, noundo);
}

static int
apply_event_cb(const struct event* event, size_t n, void* data)
{
    struct fd_module* module = data;

    return com_fd_apply_event(&module->instance, event, n);
}

static int
undo_event_cb(const struct event* event, size_t n, void *data)
{
    struct fd_module* module = data;

    return com_fd_undo_event(&module->instance, event, n);
}

static int
update_cc_cb(void* data, int noundo)
{
    struct fd_module* module = data;

    return com_fd_updatecc(&module->instance, noundo);
}

static int
clear_cc_cb(void* data, int noundo)
{
    struct fd_module* module = data;

    return com_fd_clearcc(&module->instance, noundo);
}

static int
finish_cb(void* data)
{
    struct fd_module* module = data;

    com_fd_finish(&module->instance);

    return 0;
}

static int
uninit_cb(void* data)
{
    struct fd_module* module = data;

    com_fd_uninit(&module->instance);

    module->is_initialized = false;

    return 0;
}

static struct com_fd*
get_com_fd(void)
{
    static __thread struct fd_module t_module;

    if (t_module.is_initialized) {
        return &t_module.instance;
    }

    long res = picotm_register_module(lock_cb,
                                      unlock_cb,
                                      validate_cb,
                                      apply_event_cb,
                                      undo_event_cb,
                                      update_cc_cb,
                                      clear_cc_cb,
                                      finish_cb,
                                      uninit_cb,
                                      &t_module);
    if (res < 0) {
        return NULL;
    }
    unsigned long module = res;

    res = com_fd_init(&t_module.instance, module);
    if (res < 0) {
        return NULL;
    }

    t_module.is_initialized = true;

    return &t_module.instance;
}

static struct com_fd*
get_non_null_com_fd(void)
{
    struct com_fd* com_fd = get_com_fd();
    assert(com_fd);

    return com_fd;
}

int
fd_module_accept(int sockfd, struct sockaddr* address, socklen_t* address_len)
{
    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_bind(int sockfd, const struct sockaddr* address,
                socklen_t address_len)
{
    int res;

    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_close(int fildes)
{
    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_connect(int sockfd, const struct sockaddr* serv_addr,
                   socklen_t addr_len)
{
    int res;

    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_dup_internal(int fildes, int cloexec)
{
    int res;

    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_dup(int fildes)
{
    return fd_module_dup_internal(fildes, 0);
}

int
fd_module_fcntl(int fildes, int cmd, union com_fd_fcntl_arg* arg)
{
    int res;

    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_fsync(int fildes)
{
    int res;

    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_listen(int sockfd, int backlog)
{
    int res;

    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_lseek(int fildes, off_t offset, int whence)
{
    off_t res;

    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_open(const char* path, int oflag, mode_t mode)
{
    int res;

    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_pipe(int pipefd[2])
{
    int res;

    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_pread(int fildes, void* buf, size_t nbyte, off_t off)
{
    ssize_t res;

    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_pwrite(int fildes, const void* buf, size_t nbyte, off_t off)
{
    ssize_t res;

    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_read(int fildes, void* buf, size_t nbyte)
{
    ssize_t res;

    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_recv(int sockfd, void* buffer, size_t length, int flags)
{
    ssize_t res;

    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_select(int nfds, fd_set* readfds, fd_set* writefds,
                  fd_set* errorfds, struct timeval* timeout)
{
    int res;

    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_send(int fildes, const void* buffer, size_t length, int flags)
{
    ssize_t res;

    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_shutdown(int sockfd, int how)
{
    int res;

    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_socket(int domain, int type, int protocol)
{
    int res;

    struct com_fd* comfd = get_non_null_com_fd();

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
fd_module_sync()
{
    struct com_fd* comfd = get_non_null_com_fd();

    com_fd_exec_sync(comfd);
}

ssize_t
fd_module_write(int fildes, const void* buf, size_t nbyte)
{
    ssize_t res;

    struct com_fd* comfd = get_non_null_com_fd();

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
