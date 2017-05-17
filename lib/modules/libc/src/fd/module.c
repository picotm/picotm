/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "module.h"
#include <assert.h>
#include <picotm/picotm.h>
#include <stdlib.h>
#include "errcode.h"
#include "fildes_tx.h"

struct fd_module {
    bool          is_initialized;
    struct fildes_tx tx;
};

static void
lock_cb(void* data, struct picotm_error* error)
{
    struct fd_module* module = data;

    int res = fildes_tx_lock(&module->tx, error);
    if (res < 0) {
        if (res == ERR_SYSTEM) {
            picotm_error_set_error_code(error, PICOTM_GENERAL_ERROR);
            return;
        } else if (res == ERR_CONFLICT) {
            picotm_error_set_conflicting(error, NULL);
            return;
        } else {
            /* Unsupported error code. */
            abort();
        }
    }
}

static void
unlock_cb(void* data, struct picotm_error* error)
{
    struct fd_module* module = data;

    fildes_tx_unlock(&module->tx);
}

static bool
is_valid_cb(void* data, int noundo, struct picotm_error* error)
{
    struct fd_module* module = data;

    int res = fildes_tx_validate(&module->tx, noundo, error);
    if (res < 0) {
        if (res == ERR_SYSTEM) {
            picotm_error_set_error_code(error, PICOTM_GENERAL_ERROR);
        } else if (res == ERR_CONFLICT) {
            picotm_error_set_conflicting(error, NULL);
        } else {
            /* Unsupported error code. */
            abort();
        }
        return false;
    }
    return true;
}

static void
apply_event_cb(const struct event* event, size_t n, void* data,
               struct picotm_error* error)
{
    struct fd_module* module = data;

    int res = fildes_tx_apply_event(&module->tx, event, n, error);
    if (res < 0) {
        if (res == ERR_SYSTEM) {
            picotm_error_set_error_code(error, PICOTM_GENERAL_ERROR);
            return;
        } else if (res == ERR_CONFLICT) {
            picotm_error_set_conflicting(error, NULL);
            return;
        } else {
            /* Unsupported error code. */
            abort();
        }
    }
}

static void
undo_event_cb(const struct event* event, size_t n, void *data,
              struct picotm_error* error)
{
    struct fd_module* module = data;

    int res = fildes_tx_undo_event(&module->tx, event, n, error);
    if (res < 0) {
        if (res == ERR_SYSTEM) {
            picotm_error_set_error_code(error, PICOTM_GENERAL_ERROR);
            return;
        } else if (res == ERR_CONFLICT) {
            picotm_error_set_conflicting(error, NULL);
            return;
        } else {
            /* Unsupported error code. */
            abort();
        }
    }
}

static void
update_cc_cb(void* data, int noundo, struct picotm_error* error)
{
    struct fd_module* module = data;

    int res = fildes_tx_update_cc(&module->tx, noundo, error);
    if (res < 0) {
        if (res == ERR_SYSTEM) {
            picotm_error_set_error_code(error, PICOTM_GENERAL_ERROR);
            return;
        } else if (res == ERR_CONFLICT) {
            picotm_error_set_conflicting(error, NULL);
            return;
        } else {
            /* Unsupported error code. */
            abort();
        }
    }
}

static void
clear_cc_cb(void* data, int noundo, struct picotm_error* error)
{
    struct fd_module* module = data;

    int res = fildes_tx_clear_cc(&module->tx, noundo, error);
    if (res < 0) {
        if (res == ERR_SYSTEM) {
            picotm_error_set_error_code(error, PICOTM_GENERAL_ERROR);
            return;
        } else if (res == ERR_CONFLICT) {
            picotm_error_set_conflicting(error, NULL);
            return;
        } else {
            /* Unsupported error code. */
            abort();
        }
    }
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    struct fd_module* module = data;

    fildes_tx_finish(&module->tx);
}

static void
uninit_cb(void* data)
{
    struct fd_module* module = data;

    fildes_tx_uninit(&module->tx);
    module->is_initialized = false;
}

static struct fildes_tx*
get_fildes_tx(bool initialize, struct picotm_error* error)
{
    static __thread struct fd_module t_module;

    if (t_module.is_initialized) {
        return &t_module.tx;
    } else if (!initialize) {
        return NULL;
    }

    unsigned long module = picotm_register_module(lock_cb,
                                                  unlock_cb,
                                                  is_valid_cb,
                                                  apply_event_cb,
                                                  undo_event_cb,
                                                  update_cc_cb,
                                                  clear_cc_cb,
                                                  finish_cb,
                                                  uninit_cb,
                                                  &t_module,
                                                  error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    int res = fildes_tx_init(&t_module.tx, module);
    if (res < 0) {
        return NULL;
    }

    t_module.is_initialized = true;

    return &t_module.tx;
}

static struct fildes_tx*
get_non_null_fildes_tx(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    struct fildes_tx* fildes_tx = get_fildes_tx(true, &error);
    if (picotm_error_is_set(&error)) {
        picotm_recover_from_error(&error);
    }
    assert(fildes_tx);
    return fildes_tx;
}

int
fd_module_accept(int sockfd, struct sockaddr* address, socklen_t* address_len)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    int res;

    do {
        res = fildes_tx_exec_accept(fildes_tx, sockfd, address, address_len);

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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

    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        res = fildes_tx_exec_bind(fildes_tx, sockfd, address, address_len,
                               picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    int res;

    do {
        res = fildes_tx_exec_close(fildes_tx, fildes, picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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

    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        res = fildes_tx_exec_connect(fildes_tx, sockfd, serv_addr, addr_len,
                                  picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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

    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        res = fildes_tx_exec_dup(fildes_tx, fildes, cloexec);

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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
fd_module_fcntl(int fildes, int cmd, union fcntl_arg* arg)
{
    int res;

    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        res = fildes_tx_exec_fcntl(fildes_tx, fildes, cmd, arg, picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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

    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        res = fildes_tx_exec_fsync(fildes_tx, fildes, picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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

    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        res = fildes_tx_exec_listen(fildes_tx, sockfd, backlog, picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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

    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        res = fildes_tx_exec_lseek(fildes_tx, fildes, offset, whence, picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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

    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        res = fildes_tx_exec_open(fildes_tx, path, oflag, mode, picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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

    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        res = fildes_tx_exec_pipe(fildes_tx, pipefd);

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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

    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        res = fildes_tx_exec_pread(fildes_tx, fildes, buf, nbyte, off,
                                picotm_is_irrevocable());

        /* possibly validate all optimistic domains */
        if ((fildes_tx_get_validation_mode(fildes_tx) == PICOTM_LIBC_VALIDATE_FULL)
            && fildes_tx_get_optcc(fildes_tx)
            && !picotm_is_valid()) {
            res = ERR_CONFLICT;
        }

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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

    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        res = fildes_tx_exec_pwrite(fildes_tx, fildes, buf, nbyte, off,
                                 picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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

    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        res = fildes_tx_exec_read(fildes_tx, fildes, buf, nbyte,
                               picotm_is_irrevocable());

        /* possibly validate all optimistic domains */
        if ((fildes_tx_get_validation_mode(fildes_tx) == PICOTM_LIBC_VALIDATE_FULL)
            && fildes_tx_get_optcc(fildes_tx)
            && !picotm_is_valid()) {
            res = ERR_CONFLICT;
        }

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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

    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        res = fildes_tx_exec_recv(fildes_tx, sockfd, buffer, length, flags,
                               picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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

    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        res = fildes_tx_exec_select(fildes_tx, nfds, readfds,
                                              writefds,
                                              errorfds,
                                              timeout,
                                              picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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

    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        res = fildes_tx_exec_send(fildes_tx, fildes, buffer, length, flags,
                               picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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

    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        res = fildes_tx_exec_shutdown(fildes_tx, sockfd, how, picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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

    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        res = fildes_tx_exec_socket(fildes_tx, domain, type, protocol);

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    fildes_tx_exec_sync(fildes_tx);
}

ssize_t
fd_module_write(int fildes, const void* buf, size_t nbyte)
{
    ssize_t res;

    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        res = fildes_tx_exec_write(fildes_tx, fildes, buf, nbyte,
                                picotm_is_irrevocable());

        switch (res) {
            case ERR_CONFLICT:
                picotm_restart();
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
