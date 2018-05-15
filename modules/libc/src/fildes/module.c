/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
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
 *
 * SPDX-License-Identifier: MIT
 */

#include "module.h"
#include "picotm/picotm.h"
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

struct fildes_shared_state {
    struct picotm_shared_ref16_obj ref_obj;
    struct fildes fildes;
};

#define FILDES_SHARED_STATE_INITIALIZER             \
{                                                   \
    .ref_obj = PICOTM_SHARED_REF16_OBJ_INITIALIZER  \
}

static void
init_fildes_shared_state_fields(struct fildes_shared_state* shared,
                                struct picotm_error* error)
{
    fildes_init(&shared->fildes, error);
}

static void
uninit_fildes_shared_state_fields(struct fildes_shared_state* shared)
{
    fildes_uninit(&shared->fildes);
}

static void
first_ref_fildes_shared_state_cb(struct picotm_shared_ref16_obj* ref_obj,
                                 void* data, struct picotm_error* error)
{
    struct fildes_shared_state* shared =
        picotm_containerof(ref_obj, struct fildes_shared_state, ref_obj);
    init_fildes_shared_state_fields(shared, error);
}

static void
fildes_shared_state_ref(struct fildes_shared_state* self,
                        struct picotm_error* error)
{
    picotm_shared_ref16_obj_up(&self->ref_obj, NULL, NULL,
                               first_ref_fildes_shared_state_cb,
                               error);
    if (picotm_error_is_set(error)) {
        return;
    }
};

static void
final_ref_fildes_shared_state_cb(struct picotm_shared_ref16_obj* ref_obj,
                                 void* data, struct picotm_error* error)
{
    struct fildes_shared_state* shared =
        picotm_containerof(ref_obj, struct fildes_shared_state, ref_obj);
    uninit_fildes_shared_state_fields(shared);
}

static void
fildes_shared_state_unref(struct fildes_shared_state* self)
{
    picotm_shared_ref16_obj_down(&self->ref_obj, NULL, NULL,
                                 final_ref_fildes_shared_state_cb);
}

/*
 * Global data
 */

/* Returns the statically allocated global state. Callers *must* already
 * hold a reference. */
static struct fildes_shared_state*
get_fildes_global_state(void)
{
    static struct fildes_shared_state s_global =
        FILDES_SHARED_STATE_INITIALIZER;
    return &s_global;
}

static struct fildes_shared_state*
ref_fildes_global_state(struct picotm_error* error)
{
    struct fildes_shared_state* global = get_fildes_global_state();

    fildes_shared_state_ref(global, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return global;
};

static void
unref_fildes_global_state(void)
{
    struct fildes_shared_state* global = get_fildes_global_state();
    fildes_shared_state_unref(global);
}

/*
 * Module interface
 */

struct fildes_module {
    bool          is_initialized;
    struct fildes_log log;
    struct fildes_tx tx;
};

/*
 * Thread-local data
 */

static void
prepare_commit_cb(void* data, int noundo, struct picotm_error* error)
{
    struct fildes_module* module = data;

    fildes_tx_prepare_commit(&module->tx, noundo, error);
}

static void
apply_event_cb(uint16_t head, uintptr_t tail, void* data,
               struct picotm_error* error)
{
    struct fildes_module* module = data;

    struct fildes_event* event = fildes_log_at(&module->log, tail);

    fildes_tx_apply_event(&module->tx, head, event->fildes, event->cookie,
                          error);
}

static void
undo_event_cb(uint16_t head, uintptr_t tail, void *data,
              struct picotm_error* error)
{
    struct fildes_module* module = data;

    struct fildes_event* event = fildes_log_at(&module->log, tail);

    fildes_tx_undo_event(&module->tx, head, event->fildes, event->cookie,
                         error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    struct fildes_module* module = data;

    fildes_tx_finish(&module->tx, error);
    fildes_log_clear(&module->log);
}

static void
uninit_cb(void* data)
{
    struct fildes_module* module = data;

    fildes_tx_uninit(&module->tx);
    fildes_log_uninit(&module->log);

    module->is_initialized = false;

    unref_fildes_global_state();
}

static struct fildes_tx*
get_fildes_tx(bool initialize, struct picotm_error* error)
{
    static const struct picotm_module_ops g_ops = {
        .prepare_commit = prepare_commit_cb,
        .apply_event = apply_event_cb,
        .undo_event = undo_event_cb,
        .finish = finish_cb,
        .uninit = uninit_cb
    };
    static __thread struct fildes_module t_module;

    if (t_module.is_initialized) {
        return &t_module.tx;
    } else if (!initialize) {
        return NULL;
    }

    struct fildes_shared_state* global = ref_fildes_global_state(error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    unsigned long module = picotm_register_module(&g_ops, &t_module, error);
    if (picotm_error_is_set(error)) {
        goto err_picotm_register_module;
    }

    fildes_log_init(&t_module.log, module);
    fildes_tx_init(&t_module.tx, &global->fildes, &t_module.log);

    t_module.is_initialized = true;

    return &t_module.tx;

err_picotm_register_module:
    unref_fildes_global_state();
    return NULL;
}

static struct fildes_tx*
get_non_null_fildes_tx(void)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        struct fildes_tx* fildes_tx = get_fildes_tx(true, &error);

        if (!picotm_error_is_set(&error)) {
            assert(fildes_tx);
            return fildes_tx;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

/*
 * Public interface
 */

int
fildes_module_accept(int sockfd, struct sockaddr* address,
                     socklen_t* address_len)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_accept(fildes_tx, sockfd, address,
                                        address_len, picotm_is_irrevocable(),
                                        &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_bind(int sockfd, const struct sockaddr* address,
                   socklen_t address_len)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_bind(fildes_tx, sockfd, address, address_len,
                                      picotm_is_irrevocable(), &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_chmod(const char* path, mode_t mode)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_chmod(fildes_tx, path, mode, &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_close(int fildes)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_close(fildes_tx, fildes,
                                       picotm_is_irrevocable(),
                                       &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_connect(int sockfd, const struct sockaddr* serv_addr,
                      socklen_t addr_len)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_connect(fildes_tx, sockfd, serv_addr,
                                         addr_len, picotm_is_irrevocable(),
                                         &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_dup_internal(int fildes, int cloexec)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_dup(fildes_tx, fildes, cloexec,
                                     picotm_is_irrevocable(), &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_dup(int fildes)
{
    return fildes_module_dup_internal(fildes, 0);
}

int
fildes_module_fchdir(int fildes)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_fchdir(fildes_tx, fildes, &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_fchmod(int fildes, mode_t mode)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_fchmod(fildes_tx, fildes, mode,
                                        picotm_is_irrevocable(),
                                        &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_fcntl(int fildes, int cmd, union fcntl_arg* arg)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_fcntl(fildes_tx, fildes, cmd, arg,
                                       picotm_is_irrevocable(), &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_fstat(int fildes, struct stat* buf)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_fstat(fildes_tx, fildes, buf,
                                       picotm_is_irrevocable(), &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_fsync(int fildes)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_fsync(fildes_tx, fildes,
                                       picotm_is_irrevocable(), &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_link(const char* path1, const char* path2)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_link(fildes_tx, path1, path2, &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_listen(int sockfd, int backlog)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_listen(fildes_tx, sockfd, backlog,
                                        picotm_is_irrevocable(), &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

off_t
fildes_module_lseek(int fildes, off_t offset, int whence)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        off_t res = fildes_tx_exec_lseek(fildes_tx, fildes, offset, whence,
                                         picotm_is_irrevocable(), &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_lstat(const char* path, struct stat* buf)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_lstat(fildes_tx, path, buf, &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_mkdir(const char* path, mode_t mode)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_mkdir(fildes_tx, path, mode, &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_mkfifo(const char* path, mode_t mode)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_mkfifo(fildes_tx, path, mode, &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_mknod(const char* path, mode_t mode, dev_t dev)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_mknod(fildes_tx, path, mode, dev, &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_mkstemp(char* template)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int fildes = fildes_tx_exec_mkstemp(fildes_tx, template, &error);

        if (!picotm_error_is_set(&error)) {
            return fildes;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_open(const char* path, int oflag, mode_t mode)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_open(fildes_tx, path, oflag, mode,
                                      picotm_is_irrevocable(), &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_pipe(int pipefd[2])
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_pipe(fildes_tx, pipefd, &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

ssize_t
fildes_module_pread(int fildes, void* buf, size_t nbyte, off_t off)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        ssize_t res = fildes_tx_exec_pread(fildes_tx, fildes, buf, nbyte, off,
                                           picotm_is_irrevocable(), &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

ssize_t
fildes_module_pwrite(int fildes, const void* buf, size_t nbyte, off_t off)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        ssize_t res = fildes_tx_exec_pwrite(fildes_tx, fildes, buf, nbyte,
                                            off, picotm_is_irrevocable(),
                                            &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

ssize_t
fildes_module_read(int fildes, void* buf, size_t nbyte)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        ssize_t res = fildes_tx_exec_read(fildes_tx, fildes, buf, nbyte,
                                          picotm_is_irrevocable(), &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

ssize_t
fildes_module_recv(int sockfd, void* buffer, size_t length, int flags)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        ssize_t res = fildes_tx_exec_recv(fildes_tx, sockfd, buffer, length,
                                          flags, picotm_is_irrevocable(),
                                          &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_select(int nfds, fd_set* readfds, fd_set* writefds,
                     fd_set* errorfds, struct timeval* timeout)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_select(fildes_tx, nfds, readfds,
                                        writefds, errorfds, timeout,
                                        picotm_is_irrevocable(), &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

ssize_t
fildes_module_send(int fildes, const void* buffer, size_t length, int flags)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        ssize_t res = fildes_tx_exec_send(fildes_tx, fildes, buffer, length,
                                          flags, picotm_is_irrevocable(),
                                          &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_shutdown(int sockfd, int how)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_shutdown(fildes_tx, sockfd, how,
                                          picotm_is_irrevocable(),
                                          &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_socket(int domain, int type, int protocol)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = fildes_tx_exec_socket(fildes_tx, domain, type, protocol,
                                        &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_stat(const char* path, struct stat* buf)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int fildes = fildes_tx_exec_stat(fildes_tx, path, buf, &error);

        if (!picotm_error_is_set(&error)) {
            return fildes;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

void
fildes_module_sync()
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        fildes_tx_exec_sync(fildes_tx, &error);

        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
fildes_module_unlink(const char* path)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int fildes = fildes_tx_exec_unlink(fildes_tx, path, &error);

        if (!picotm_error_is_set(&error)) {
            return fildes;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

ssize_t
fildes_module_write(int fildes, const void* buf, size_t nbyte)
{
    struct fildes_tx* fildes_tx = get_non_null_fildes_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        ssize_t res = fildes_tx_exec_write(fildes_tx, fildes, buf, nbyte,
                                           picotm_is_irrevocable(), &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}
