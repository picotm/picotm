/*
 * picotm - A system-level transaction manager
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "module.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-global-state.h"
#include "picotm/picotm-lib-shared-state.h"
#include "picotm/picotm-lib-state.h"
#include "picotm/picotm-lib-thread-state.h"
#include "picotm/picotm-module.h"
#include <assert.h>
#include <stdlib.h>
#include "fildes.h"
#include "fildes_event.h"
#include "fildes_log.h"
#include "fildes_tx.h"

/*
 * Shared state
 */

static void
init_fildes_shared_state_fields(struct fildes* fildes,
                                struct picotm_error* error)
{
    fildes_init(fildes, error);
}

static void
uninit_fildes_shared_state_fields(struct fildes* fildes)
{
    fildes_uninit(fildes);
}

PICOTM_SHARED_STATE(fildes, struct fildes);
PICOTM_SHARED_STATE_STATIC_IMPL(fildes, struct fildes,
                                init_fildes_shared_state_fields,
                                uninit_fildes_shared_state_fields)

/*
 * Global state
 */

PICOTM_GLOBAL_STATE_STATIC_IMPL(fildes)

/*
 * Module interface
 */

struct fildes_module {
    struct fildes_log log;
    struct fildes_tx tx;
};

static void
fildes_module_init(struct fildes_module* self, struct fildes* fildes,
                   unsigned long module_id)
{
    assert(self);

    fildes_log_init(&self->log, module_id);
    fildes_tx_init(&self->tx, fildes, &self->log);
}

static void
fildes_module_uninit(struct fildes_module* self)
{
    assert(self);

    fildes_tx_uninit(&self->tx);
    fildes_log_uninit(&self->log);
}

static void
fildes_module_prepare_commit(struct fildes_module* self, int noundo,
                             struct picotm_error* error)
{
    assert(self);

    fildes_tx_prepare_commit(&self->tx, noundo, error);
}

static void
fildes_module_apply_event(struct fildes_module* self, uint16_t head,
                          uintptr_t tail, struct picotm_error* error)
{
    assert(self);

    struct fildes_event* event = fildes_log_at(&self->log, tail);
    assert(event);

    fildes_tx_apply_event(&self->tx, head, event->fildes, event->cookie,
                          error);
}

static void
fildes_module_undo_event(struct fildes_module* self, uint16_t head,
                         uintptr_t tail, struct picotm_error* error)
{
    assert(self);

    struct fildes_event* event = fildes_log_at(&self->log, tail);
    assert(event);

    fildes_tx_undo_event(&self->tx, head, event->fildes, event->cookie,
                         error);
}

static void
fildes_module_finish(struct fildes_module* self, struct picotm_error* error)
{
    assert(self);

    fildes_tx_finish(&self->tx, error);
    fildes_log_clear(&self->log);
}

/*
 * Thread-local state
 */

PICOTM_STATE(fildes_module, struct fildes_module);
PICOTM_STATE_STATIC_DECL(fildes_module, struct fildes_module)
PICOTM_THREAD_STATE_STATIC_DECL(fildes_module)

static void
prepare_commit_cb(void* data, int noundo, struct picotm_error* error)
{
    struct fildes_module* module = data;
    fildes_module_prepare_commit(module, noundo, error);
}

static void
apply_event_cb(uint16_t head, uintptr_t tail, void* data,
               struct picotm_error* error)
{
    struct fildes_module* module = data;
    fildes_module_apply_event(module, head, tail, error);
}

static void
undo_event_cb(uint16_t head, uintptr_t tail, void *data,
              struct picotm_error* error)
{
    struct fildes_module* module = data;
    fildes_module_undo_event(module, head, tail, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    struct fildes_module* module = data;
    fildes_module_finish(module, error);
}

static void
release_cb(void* data)
{
    PICOTM_THREAD_STATE_RELEASE(fildes_module);
}

static void
init_fildes_module(struct fildes_module* module, struct picotm_error* error)
{
    static const struct picotm_module_ops s_ops = {
        .prepare_commit = prepare_commit_cb,
        .apply_event = apply_event_cb,
        .undo_event = undo_event_cb,
        .finish = finish_cb,
        .release = release_cb
    };

    struct fildes* fildes = PICOTM_GLOBAL_STATE_REF(fildes, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    unsigned long module_id = picotm_register_module(&s_ops, module, error);
    if (picotm_error_is_set(error)) {
        goto err_picotm_register_module;
    }

    fildes_module_init(module, fildes, module_id);

    return;

err_picotm_register_module:
    PICOTM_GLOBAL_STATE_UNREF(fildes);
}

static void
uninit_fildes_module(struct fildes_module* module)
{
    fildes_module_uninit(module);
    PICOTM_GLOBAL_STATE_UNREF(fildes);
}

PICOTM_STATE_STATIC_IMPL(fildes_module, struct fildes_module,
                         init_fildes_module,
                         uninit_fildes_module)
PICOTM_THREAD_STATE_STATIC_IMPL(fildes_module)

static struct fildes_tx*
get_fildes_tx(struct picotm_error* error)
{
    struct fildes_module* module = PICOTM_THREAD_STATE_ACQUIRE(fildes_module,
                                                               true, error);
    if (picotm_error_is_set(error)) {
        return nullptr;
    }
    return &module->tx;
}

/*
 * Public interface
 */

int
fildes_module_accept(int sockfd, struct sockaddr* address,
                     socklen_t* address_len, struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_accept(fildes_tx, sockfd, address, address_len,
                                 picotm_is_irrevocable(), error);
}

int
fildes_module_bind(int sockfd, const struct sockaddr* address,
                   socklen_t address_len, struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_bind(fildes_tx, sockfd, address, address_len,
                               picotm_is_irrevocable(), error);
}

int
fildes_module_chmod(const char* path, mode_t mode, struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_chmod(fildes_tx, path, mode, error);
}

int
fildes_module_close(int fildes, struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_close(fildes_tx, fildes, picotm_is_irrevocable(),
                                error);
}

int
fildes_module_connect(int sockfd, const struct sockaddr* serv_addr,
                      socklen_t addr_len, struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_connect(fildes_tx, sockfd, serv_addr, addr_len,
                                  picotm_is_irrevocable(), error);
}

int
fildes_module_dup_internal(int fildes, int cloexec, struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_dup(fildes_tx, fildes, cloexec,
                              picotm_is_irrevocable(),
                              error);
}

int
fildes_module_dup(int fildes, struct picotm_error* error)
{
    return fildes_module_dup_internal(fildes, 0, error);
}

int
fildes_module_fchdir(int fildes, struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_fchdir(fildes_tx, fildes, error);
}

int
fildes_module_fchmod(int fildes, mode_t mode, struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_fchmod(fildes_tx, fildes, mode,
                                 picotm_is_irrevocable(),
                                 error);
}

int
fildes_module_fcntl(int fildes, int cmd, union fcntl_arg* arg,
                    struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_fcntl(fildes_tx, fildes, cmd, arg,
                                picotm_is_irrevocable(), error);
}

int
fildes_module_fstat(int fildes, struct stat* buf, struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_fstat(fildes_tx, fildes, buf,
                                picotm_is_irrevocable(), error);
}

int
fildes_module_fsync(int fildes, struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_fsync(fildes_tx, fildes,
                                picotm_is_irrevocable(),
                                error);
}

int
fildes_module_link(const char* path1, const char* path2,
                   struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_link(fildes_tx, path1, path2, error);
}

int
fildes_module_listen(int sockfd, int backlog, struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_listen(fildes_tx, sockfd, backlog,
                                 picotm_is_irrevocable(), error);
}

off_t
fildes_module_lseek(int fildes, off_t offset, int whence,
                    struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_lseek(fildes_tx, fildes, offset, whence,
                                picotm_is_irrevocable(), error);
}

int
fildes_module_lstat(const char* path, struct stat* buf,
                    struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_lstat(fildes_tx, path, buf, error);
}

int
fildes_module_mkdir(const char* path, mode_t mode, struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_mkdir(fildes_tx, path, mode, error);
}

int
fildes_module_mkfifo(const char* path, mode_t mode,
                     struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_mkfifo(fildes_tx, path, mode, error);
}

int
fildes_module_mknod(const char* path, mode_t mode, dev_t dev,
                    struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_mknod(fildes_tx, path, mode, dev, error);
}

int
fildes_module_mkstemp(char* template, struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_mkstemp(fildes_tx, template, error);
}

int
fildes_module_open(const char* path, int oflag, mode_t mode,
                   struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_open(fildes_tx, path, oflag, mode,
                               picotm_is_irrevocable(), error);
}

int
fildes_module_pipe(int pipefd[2], struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_pipe(fildes_tx, pipefd, error);
}

ssize_t
fildes_module_pread(int fildes, void* buf, size_t nbyte, off_t off,
                    struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_pread(fildes_tx, fildes, buf, nbyte, off,
                                picotm_is_irrevocable(), error);
}

ssize_t
fildes_module_pwrite(int fildes, const void* buf, size_t nbyte, off_t off,
                     struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_pwrite(fildes_tx, fildes, buf, nbyte, off,
                                 picotm_is_irrevocable(), error);
}

ssize_t
fildes_module_read(int fildes, void* buf, size_t nbyte,
                   struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_read(fildes_tx, fildes, buf, nbyte,
                               picotm_is_irrevocable(), error);
}

ssize_t
fildes_module_recv(int sockfd, void* buffer, size_t length, int flags,
                   struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_recv(fildes_tx, sockfd, buffer, length, flags,
                               picotm_is_irrevocable(), error);
}

int
fildes_module_select(int nfds, fd_set* readfds, fd_set* writefds,
                     fd_set* errorfds, struct timeval* timeout,
                     struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_select(fildes_tx, nfds, readfds, writefds, errorfds,
                                 timeout, picotm_is_irrevocable(), error);
}

ssize_t
fildes_module_send(int fildes, const void* buffer, size_t length, int flags,
                   struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_send(fildes_tx, fildes, buffer, length, flags,
                               picotm_is_irrevocable(), error);
}

int
fildes_module_shutdown(int sockfd, int how, struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_shutdown(fildes_tx, sockfd, how,
                                   picotm_is_irrevocable(),
                                   error);
}

int
fildes_module_socket(int domain, int type, int protocol,
                     struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_socket(fildes_tx, domain, type, protocol, error);
}

int
fildes_module_stat(const char* path, struct stat* buf,
                   struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_stat(fildes_tx, path, buf, error);
}

void
fildes_module_sync(struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return;
    }
    fildes_tx_exec_sync(fildes_tx, error);
}

int
fildes_module_unlink(const char* path, struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_unlink(fildes_tx, path, error);
}

ssize_t
fildes_module_write(int fildes, const void* buf, size_t nbyte,
                    struct picotm_error* error)
{
    struct fildes_tx* fildes_tx = get_fildes_tx(error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return fildes_tx_exec_write(fildes_tx, fildes, buf, nbyte,
                                picotm_is_irrevocable(), error);
}
