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

#include "fildes_tx.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <picotm/picotm-lib-tab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "allocator/module.h"
#include "chrdev.h"
#include "chrdevtab.h"
#include "cwd/module.h"
#include "dir.h"
#include "dirtab.h"
#include "fd.h"
#include "fifo.h"
#include "fifotab.h"
#include "openop.h"
#include "openoptab.h"
#include "pipeop.h"
#include "pipeoptab.h"
#include "range.h"
#include "regfile.h"
#include "regfiletab.h"
#include "socket.h"
#include "sockettab.h"

enum fildes_tx_cmd {
    CMD_ACCEPT,
    CMD_BIND,
    CMD_CLOSE,
    CMD_CONNECT,
    CMD_DUP,
    CMD_FCHMOD,
    CMD_FCNTL,
    CMD_FSTAT,
    CMD_FSYNC,
    CMD_LISTEN,
    CMD_LSEEK,
    CMD_MKSTEMP,
    CMD_OPEN,
    CMD_PIPE,
    CMD_PREAD,
    CMD_PWRITE,
    CMD_READ,
    CMD_RECV,
    CMD_SEND,
    CMD_SHUTDOWN,
    CMD_SOCKET,
    CMD_SYNC,
    CMD_WRITE
};

void
fildes_tx_init(struct fildes_tx* self, unsigned long module)
{
    self->module = module;

    fdtab_tx_init(&self->fdtab_tx);

    self->fd_tx_max_fildes = 0;
    SLIST_INIT(&self->fd_tx_active_list);

    self->chrdev_tx_max_index = 0;
    SLIST_INIT(&self->chrdev_tx_active_list);

    self->fifo_tx_max_index = 0;
    SLIST_INIT(&self->fifo_tx_active_list);

    self->regfile_tx_max_index = 0;
    SLIST_INIT(&self->regfile_tx_active_list);

    self->dir_tx_max_index = 0;
    SLIST_INIT(&self->dir_tx_active_list);

    self->socket_tx_max_index = 0;
    SLIST_INIT(&self->socket_tx_active_list);

    self->eventtab = NULL;
    self->eventtablen = 0;
    self->eventtabsiz = 0;

    self->openoptab = NULL;
    self->openoptablen = 0;

    self->pipeoptab = NULL;
    self->pipeoptablen = 0;
}

void
fildes_tx_uninit(struct fildes_tx* self)
{
    /* Uninit chrdev_txs */

    for (struct chrdev_tx* chrdev_tx = self->chrdev_tx;
                           chrdev_tx < self->chrdev_tx + self->chrdev_tx_max_index;
                         ++chrdev_tx) {
        chrdev_tx_uninit(chrdev_tx);
    }

    /* Uninit fifo_txs */

    for (struct fifo_tx* fifo_tx = self->fifo_tx;
                         fifo_tx < self->fifo_tx + self->fifo_tx_max_index;
                       ++fifo_tx) {
        fifo_tx_uninit(fifo_tx);
    }

    /* Uninit regfile_txs */

    for (struct regfile_tx* regfile_tx = self->regfile_tx;
                            regfile_tx < self->regfile_tx + self->regfile_tx_max_index;
                          ++regfile_tx) {
        regfile_tx_uninit(regfile_tx);
    }

    /* Uninit dir_txs */

    for (struct dir_tx* dir_tx = self->dir_tx;
                        dir_tx < self->dir_tx + self->dir_tx_max_index;
                      ++dir_tx) {
        dir_tx_uninit(dir_tx);
    }

    /* Uninit socket_txs */

    for (struct socket_tx* socket_tx = self->socket_tx;
                           socket_tx < self->socket_tx + self->socket_tx_max_index;
                         ++socket_tx) {
        socket_tx_uninit(socket_tx);
    }

    /* Uninit fd_txs */

    for (struct fd_tx* fd_tx = self->fd_tx;
                       fd_tx < self->fd_tx + self->fd_tx_max_fildes;
                     ++fd_tx) {
        fd_tx_uninit(fd_tx);
    }

    fdtab_tx_uninit(&self->fdtab_tx);

    pipeoptab_clear(&self->pipeoptab, &self->pipeoptablen);
    openoptab_clear(&self->openoptab, &self->openoptablen);

    free(self->eventtab);
}

void
fildes_tx_set_validation_mode(struct fildes_tx* self,
                              enum picotm_libc_validation_mode val_mode)
{
    picotm_libc_set_validation_mode(val_mode);
}

enum picotm_libc_validation_mode
fildes_tx_get_validation_mode(const struct fildes_tx* self)
{
    return picotm_libc_get_validation_mode();
}

static struct chrdev_tx*
get_chrdev_tx(struct fildes_tx* self, int index)
{
    for (struct chrdev_tx* chrdev_tx = self->chrdev_tx + self->chrdev_tx_max_index;
                           chrdev_tx < self->chrdev_tx + index + 1;
                         ++chrdev_tx) {
        chrdev_tx_init(chrdev_tx);
    }

    self->chrdev_tx_max_index = lmax(index + 1, self->chrdev_tx_max_index);

    return self->chrdev_tx + index;
}

static struct chrdev_tx*
get_chrdev_tx_with_ref(struct fildes_tx* self, int fildes, unsigned long flags,
                       struct picotm_error* error)
{
    struct chrdev* chrdev = chrdevtab_ref_fildes(fildes,
                                                 !!(flags & CHRDEV_FL_WANTNEW),
                                                 error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct chrdev_tx* chrdev_tx = get_chrdev_tx(self,
                                                chrdevtab_index(chrdev));

    /* In |struct fildes_tx| we hold at most one reference to the
     * transaction state of each character device. This reference is
     * released in fildes_tx_finish().
     */
    if (chrdev_tx_holds_ref(chrdev_tx)) {
        goto unref;
    }

    chrdev_tx_ref_or_set_up(chrdev_tx, chrdev, fildes, flags, error);
    if (picotm_error_is_set(error)) {
        goto err_chrdev_tx_ref_or_set_up;
    }

    SLIST_INSERT_HEAD(&self->chrdev_tx_active_list, chrdev_tx, active_list);

unref:
    chrdev_unref(chrdev);

    return chrdev_tx;

err_chrdev_tx_ref_or_set_up:
    chrdev_unref(chrdev);
    return NULL;
}

static struct fifo_tx*
get_fifo_tx(struct fildes_tx* self, int index)
{
    for (struct fifo_tx* fifo_tx = self->fifo_tx + self->fifo_tx_max_index;
                         fifo_tx < self->fifo_tx + index + 1;
                       ++fifo_tx) {
        fifo_tx_init(fifo_tx);
    }

    self->fifo_tx_max_index = lmax(index + 1, self->fifo_tx_max_index);

    return self->fifo_tx + index;
}

static struct fifo_tx*
get_fifo_tx_with_ref(struct fildes_tx* self, int fildes, unsigned long flags,
                     struct picotm_error* error)
{
    struct fifo* fifo = fifotab_ref_fildes(fildes,
                                           !!(flags & FIFO_FL_WANTNEW),
                                           error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct fifo_tx* fifo_tx = get_fifo_tx(self, fifotab_index(fifo));

    /* In |struct fildes_tx| we hold at most one reference to the
     * transaction state of each FIFO. This reference is released
     * in fildes_tx_finish().
     */
    if (fifo_tx_holds_ref(fifo_tx)) {
        goto unref;
    }

    fifo_tx_ref_or_set_up(fifo_tx, fifo, fildes, flags, error);
    if (picotm_error_is_set(error)) {
        goto err_fifo_tx_ref_or_set_up;
    }

    SLIST_INSERT_HEAD(&self->fifo_tx_active_list, fifo_tx, active_list);

unref:
    fifo_unref(fifo);

    return fifo_tx;

err_fifo_tx_ref_or_set_up:
    fifo_unref(fifo);
    return NULL;
}

static struct regfile_tx*
get_regfile_tx(struct fildes_tx* self, int index)
{
    for (struct regfile_tx* regfile_tx = self->regfile_tx + self->regfile_tx_max_index;
                            regfile_tx < self->regfile_tx + index + 1;
                          ++regfile_tx) {
        regfile_tx_init(regfile_tx);
    }

    self->regfile_tx_max_index = lmax(index + 1, self->regfile_tx_max_index);

    return self->regfile_tx + index;
}

static struct regfile_tx*
get_regfile_tx_with_ref(struct fildes_tx* self, int fildes, unsigned long flags,
                        struct picotm_error* error)
{
    struct regfile* regfile = regfiletab_ref_fildes(
        fildes,
        !!(flags & REGFILE_FL_WANTNEW),
        error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct regfile_tx* regfile_tx = get_regfile_tx(self,
                                                   regfiletab_index(regfile));

    /* In |struct fildes_tx| we hold at most one reference to the
     * transaction state of each regular file. This reference is
     * released in fildes_tx_finish().
     */
    if (regfile_tx_holds_ref(regfile_tx)) {
        goto unref;
    }

    regfile_tx_ref_or_set_up(regfile_tx, regfile, fildes, flags, error);
    if (picotm_error_is_set(error)) {
        goto err_regfile_tx_ref_or_set_up;
    }

    SLIST_INSERT_HEAD(&self->regfile_tx_active_list, regfile_tx, active_list);

unref:
    regfile_unref(regfile);

    return regfile_tx;

err_regfile_tx_ref_or_set_up:
    regfile_unref(regfile);
    return NULL;
}

static struct dir_tx*
get_dir_tx(struct fildes_tx* self, int index)
{
    for (struct dir_tx* dir_tx = self->dir_tx + self->dir_tx_max_index;
                        dir_tx < self->dir_tx + index + 1;
                      ++dir_tx) {
        dir_tx_init(dir_tx);
    }

    self->dir_tx_max_index = lmax(index + 1, self->dir_tx_max_index);

    return self->dir_tx + index;
}

static struct dir_tx*
get_dir_tx_with_ref(struct fildes_tx* self, int fildes, unsigned long flags,
                    struct picotm_error* error)
{
    struct dir* dir = dirtab_ref_fildes(fildes,
                                        !!(flags & DIR_FL_WANTNEW),
                                        error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct dir_tx* dir_tx = get_dir_tx(self, dirtab_index(dir));

    /* In |struct fildes_tx| we hold at most one reference to the
     * transaction state of each directory. This reference is
     * released in fildes_tx_finish().
     */
    if (dir_tx_holds_ref(dir_tx)) {
        goto unref;
    }

    dir_tx_ref_or_set_up(dir_tx, dir, fildes, flags, error);
    if (picotm_error_is_set(error)) {
        goto err_dir_tx_ref_or_set_up;
    }

    SLIST_INSERT_HEAD(&self->dir_tx_active_list, dir_tx, active_list);

unref:
    dir_unref(dir);

    return dir_tx;

err_dir_tx_ref_or_set_up:
    dir_unref(dir);
    return NULL;
}

static struct socket_tx*
get_socket_tx(struct fildes_tx* self, int index)
{
    for (struct socket_tx* socket_tx = self->socket_tx + self->socket_tx_max_index;
                           socket_tx < self->socket_tx + index + 1;
                         ++socket_tx) {
        socket_tx_init(socket_tx);
    }

    self->socket_tx_max_index = lmax(index + 1, self->socket_tx_max_index);

    return self->socket_tx + index;
}

static struct socket_tx*
get_socket_tx_with_ref(struct fildes_tx* self, int fildes, unsigned long flags,
                       struct picotm_error* error)
{
    struct socket* socket = sockettab_ref_fildes(fildes,
                                                 !!(flags & SOCKET_FL_WANTNEW),
                                                 error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct socket_tx* socket_tx = get_socket_tx(self,
                                                sockettab_index(socket));

    /* In |struct fildes_tx| we hold at most one reference to the
     * transaction state of each socket. This reference is released
     * in fildes_tx_finish().
     */
    if (socket_tx_holds_ref(socket_tx)) {
        goto unref;
    }

    socket_tx_ref_or_set_up(socket_tx, socket, fildes, flags, error);
    if (picotm_error_is_set(error)) {
        goto err_socket_tx_ref_or_set_up;
    }

    SLIST_INSERT_HEAD(&self->socket_tx_active_list, socket_tx, active_list);

unref:
    socket_unref(socket);

    return socket_tx;

err_socket_tx_ref_or_set_up:
    socket_unref(socket);
    return NULL;
}

static struct ofd_tx*
get_ofd_tx_with_ref(struct fildes_tx* self, int fildes, unsigned long flags,
                    struct picotm_error* error)
{
    struct stat buf;
    int res = fstat(fildes, &buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return NULL;
    }

    switch (buf.st_mode & S_IFMT) {
        case S_IFCHR: {
            struct chrdev_tx* chrdev_tx = get_chrdev_tx_with_ref(self,
                                                                 fildes,
                                                                 flags,
                                                                 error);
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return &chrdev_tx->base;
        }
        case S_IFIFO: {
            struct fifo_tx* fifo_tx = get_fifo_tx_with_ref(self,
                                                           fildes,
                                                           flags,
                                                           error);
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return &fifo_tx->base;
        }
        case S_IFREG: {
            struct regfile_tx* regfile_tx = get_regfile_tx_with_ref(self,
                                                                    fildes,
                                                                    flags,
                                                                    error);
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return &regfile_tx->base;
        }
        case S_IFDIR: {
            struct dir_tx* dir_tx = get_dir_tx_with_ref(self,
                                                        fildes,
                                                        flags,
                                                        error);
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return &dir_tx->base;
        }
        case S_IFSOCK: {
            struct socket_tx* socket_tx = get_socket_tx_with_ref(self,
                                                                 fildes,
                                                                 flags,
                                                                 error);
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return &socket_tx->base;
        }
        default:
            picotm_error_set_errno(error, EINVAL); /* unsupported file type */
            return NULL;
    }
}

static struct fd_tx*
get_fd_tx(struct fildes_tx* self, int fildes)
{
    for (struct fd_tx* fd_tx = self->fd_tx + self->fd_tx_max_fildes;
                       fd_tx < self->fd_tx + fildes + 1;
                     ++fd_tx) {

        fd_tx_init(fd_tx);
    }

    self->fd_tx_max_fildes = lmax(fildes + 1, self->fd_tx_max_fildes);

    return self->fd_tx + fildes;
}

static struct fd_tx*
get_fd_tx_with_ref(struct fildes_tx* self, int fildes, unsigned long flags,
                   struct picotm_error* error)
{
    struct fd_tx* fd_tx = get_fd_tx(self, fildes);

    /* In |struct fildes_tx| we hold at most one reference to the
     * transaction state of each file descriptor. This reference
     * is released in fildes_tx_finish().
     */
    if (fd_tx_holds_ref(fd_tx)) {

        /* Validate reference or return error if fd has been closed */
        fd_tx_lock(fd_tx);
        fd_tx_validate(fd_tx, error);
        if (picotm_error_is_set(error)) {
            fd_tx_unlock(fd_tx);
            return NULL;
        }
        fd_tx_unlock(fd_tx);

        return fd_tx;
    }

    struct fd* fd = fdtab_tx_ref_fildes(&self->fdtab_tx, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct ofd_tx* ofd_tx = get_ofd_tx_with_ref(self, fildes, flags, error);
    if (picotm_error_is_set(error)) {
        goto err_get_ofd_tx_with_ref;
    }

    fd_tx_ref_or_set_up(fd_tx, fd, ofd_tx, flags, error);
    if (picotm_error_is_set(error)) {
        goto err_fd_tx_ref;
    }

    SLIST_INSERT_HEAD(&self->fd_tx_active_list, fd_tx, active_list);

    fd_unref(fd);

    return fd_tx;

err_fd_tx_ref:
    ofd_tx_unref(ofd_tx);
err_get_ofd_tx_with_ref:
    fd_unref(fd);
    return NULL;
}

static int
append_cmd(struct fildes_tx* self, enum fildes_tx_cmd cmd, int fildes,
           int cookie, struct picotm_error* error)
{
    if (__builtin_expect(self->eventtablen >= self->eventtabsiz, 0)) {

        void* tmp = picotm_tabresize(self->eventtab,
                                     self->eventtabsiz,
                                     self->eventtabsiz + 1,
                                     sizeof(self->eventtab[0]), error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
        self->eventtab = tmp;

        ++self->eventtabsiz;
    }

    struct fd_event* event = self->eventtab + self->eventtablen;

    event->fildes = fildes;
    event->cookie = cookie;

    picotm_append_event(self->module, cmd, self->eventtablen, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    return (int)self->eventtablen++;
}

/* For each file descriptor, Linux puts a symlink in /proc/self/fd/. The
 * link refers to the actual file. This function isn't portable, as other
 * Unix systems might use different techniques.
 */
static char*
fildes_path(int fildes, struct picotm_error* error)
{
    char symlink[40];
    int res = snprintf(symlink, sizeof(symlink), "/proc/self/fd/%d", fildes);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return NULL;
    } else if ((size_t)res >= sizeof(symlink)) {
        picotm_error_set_error_code(error, PICOTM_OUT_OF_MEMORY);
        return NULL;
    }

    char* path = realpath(symlink, NULL);
    if (!path) {
        picotm_error_set_errno(error, errno);
        return NULL;
    }

    return path;
}

static bool
is_absolute_path(const char* path)
{
    return path[0] == '/';
}

static char*
absolute_path(const char* path, struct picotm_error* error)
{
    assert(path);

    if (is_absolute_path(path)) {
        char* abs_path = strdup(path);
        if (!abs_path) {
            picotm_error_set_errno(error, errno);
            return NULL;
        }
        return abs_path;
    }

    /* Construct absolute pathname */

    char* cwd = cwd_module_getcwd(NULL, 0, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    size_t pathlen = strlen(path);
    size_t cwdlen = strlen(cwd);

    char* abs_path = malloc(pathlen + cwdlen + 2 * sizeof(*abs_path));
    if (!abs_path) {
        picotm_error_set_errno(error, errno);
        goto err_malloc;
    }

    memcpy(abs_path, path, pathlen);
    abs_path[pathlen] = '/';
    memcpy(abs_path + pathlen + 1, cwd, cwdlen);
    abs_path[pathlen + 1 + cwdlen] = '\0';

    allocator_module_free(cwd, strlen(cwd));

    return abs_path;

err_malloc:
    allocator_module_free(cwd, strlen(cwd));
    return NULL;
}

/* Unlink a file descriptor's file in a quite reliable, but Linux-only,
 * way. The function does some additional checks to ensure that the file
 * we're unlinking is the file we created in the first place. Note that
 * this is not fully atomic. The file could be replaced while this function
 * is between stat() and unlink(). We would remove a file which we don't
 * own.
 */
static void
unlink_fildes(int fildes, struct picotm_error* error)
{
    struct stat buf[2];

    char* path = fildes_path(fildes, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    /* Get file status of descriptor and path */

    int res = fstat(fildes, buf + 0);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        goto err_fstat;
    }

    res = stat(path, buf + 1);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        goto err_stat;
    }

    /* Check if file descriptor and path refer to the same file */

    if (buf[0].st_dev != buf[1].st_dev) {
        goto out;
    }
    if (buf[0].st_ino != buf[1].st_ino) {
        goto out;
    }

    /* Unlink file */

    res = unlink(path);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        goto err_unlink;
    }

out:
    free(path);

    return;

err_unlink:
err_stat:
err_fstat:
    free(path);
}

/*
 * accept()
 */

int
fildes_tx_exec_accept(struct fildes_tx* self, int sockfd,
                      struct sockaddr* address, socklen_t* address_len,
                      int isnoundo, struct picotm_error* error)
{
    /* Write-lock file-descriptor table to preserve file-descriptor order */
    fdtab_tx_try_wrlock(&self->fdtab_tx, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Accept connection */

    int cookie = -1;

    int connfd = fd_tx_accept_exec(fd_tx, sockfd, address, address_len,
                                   isnoundo, &cookie, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Reference fd_tx */

    get_fd_tx_with_ref(self, connfd, 0, error);
    if (picotm_error_is_set(error)) {
        goto err_get_fd_tx_with_ref;
    }

    /* Inject event */
    append_cmd(self, CMD_ACCEPT, connfd, cookie, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    return connfd;

err_get_fd_tx_with_ref:
    if (TEMP_FAILURE_RETRY(close(connfd)) < 0) {
        perror("close");
    }
    return -1;
}

static void
apply_accept(struct fildes_tx* self, int fildes, int cookie,
             struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    if (cookie != -1) {
        fd_tx_accept_apply(fd_tx, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
}

static void
undo_accept(struct fildes_tx* self, int fildes, int cookie,
            struct picotm_error* error)
{
    assert(self);

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    if (cookie != -1) {
        fd_tx_accept_undo(fd_tx, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Mark file descriptor to be closed */
    fd_tx_signal_close(fd_tx);
}

/*
 * bind()
 */

int
fildes_tx_exec_bind(struct fildes_tx* self, int socket,
                    const struct sockaddr* address, socklen_t addresslen,
                    int isnoundo, struct picotm_error* error)
{
    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, socket, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Bind */

    int cookie = -1;

    int res = fd_tx_bind_exec(fd_tx, socket, address, addresslen,
                              isnoundo, &cookie, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_BIND, socket, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return res;
}

static void
apply_bind(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_bind_apply(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_bind(struct fildes_tx* self, int fildes, int cookie,
          struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_bind_undo(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * chmod()
 */

int
fildes_tx_exec_chmod(struct fildes_tx* self, const char* path, mode_t mode,
                     struct picotm_error* error)
{
    char* real_path = cwd_module_realpath(path, NULL, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int res = chmod(real_path, mode);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        goto err_chmod;
    }

    allocator_module_free(real_path, strlen(real_path));

    return res;

err_chmod:
    allocator_module_free(real_path, strlen(real_path));
    return -1;
}

/*
 * close()
 */

int
fildes_tx_exec_close(struct fildes_tx* self, int fildes, int isnoundo,
                     struct picotm_error* error)
{
    /* Write-lock file-descriptor table to preserve file-descriptor order */
    fdtab_tx_try_wrlock(&self->fdtab_tx, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Close */

    int cookie = -1;

    int res = fd_tx_close_exec(fd_tx, fildes, isnoundo, &cookie, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_CLOSE, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return res;
}

static void
apply_close(struct fildes_tx* self, int fildes, int cookie,
            struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_close_apply(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_close(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_close_undo(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * connect()
 */

int
fildes_tx_exec_connect(struct fildes_tx* self, int sockfd,
                       const struct sockaddr* serv_addr, socklen_t addrlen,
                       int isnoundo, struct picotm_error* error)
{
    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Connect */

    int cookie = -1;

    int res = fd_tx_connect_exec(fd_tx, sockfd, serv_addr, addrlen,
                                 isnoundo, &cookie, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_CONNECT, sockfd, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return res;
}

static void
apply_connect(struct fildes_tx* self, int fildes, int cookie,
              struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_connect_apply(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_connect(struct fildes_tx* self, int fildes, int cookie,
             struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_connect_undo(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * dup()
 */

int
fildes_tx_exec_dup(struct fildes_tx* self, int fildes, int cloexec,
                   int isnoundo, struct picotm_error* error)
{
    assert(self);

    /* Write-lock file-descriptor table to preserve file-descriptor order */
    fdtab_tx_try_wrlock(&self->fdtab_tx, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Execute dup() */

    int cookie = -1;

    int res = fd_tx_dup_exec(fd_tx, fildes, cloexec, isnoundo, &cookie,
                             error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    int new_fildes = res;

    /* Reference fd_tx for new_fildes */

    get_fd_tx_with_ref(self, new_fildes, 0, error);
    if (picotm_error_is_set(error)) {
        goto err_get_fd_tx_with_ref;
    }

    /* Inject event */
    append_cmd(self, CMD_DUP, new_fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        goto err_append_cmd;
    }

    return new_fildes;

err_append_cmd:
err_get_fd_tx_with_ref:
    TEMP_FAILURE_RETRY(close(new_fildes));
    return -1;
}

static void
apply_dup(struct fildes_tx* self, int fildes, int cookie,
          struct picotm_error* error)
{
    assert(self);

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    if (cookie >= 0) {
        fd_tx_dup_apply(fd_tx, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
}

static void
undo_dup(struct fildes_tx* self, int fildes, int cookie,
         struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < MAXNUMFD);

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    if (cookie >= 0) {
        fd_tx_dup_undo(fd_tx, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Mark file descriptor to be closed. This works, because dup() occured
       inside transaction. So no other transaction should have access to it. */
    fd_tx_signal_close(fd_tx);
}

/*
 * fchdir()
 */

int
fildes_tx_exec_fchdir(struct fildes_tx* self, int fildes,
                      struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */
    get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Check file descriptor */

    struct stat buf;
    int res = fstat(fildes, &buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }

    if (!S_ISDIR(buf.st_mode)) {
        picotm_error_set_errno(error, ENOTDIR);
        return -1;
    }

    char* path = fildes_path(fildes, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    res = cwd_module_chdir(path, error);
    if (picotm_error_is_set(error)) {
        goto err_cwd_module_chdir;
    }

    free(path);

    return res;

err_cwd_module_chdir:
    free(path);
    return -1;
}

/*
 * fchmod()
 */

int
fildes_tx_exec_fchmod(struct fildes_tx* self, int fildes, mode_t mode,
                      int isnoundo, struct picotm_error* error)
{
    /* Update/create fd_tx */
    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int cookie = -1;

    int res = fd_tx_fchmod_exec(fd_tx, fildes, mode, isnoundo, &cookie,
                                error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_FCHMOD, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return res;
}

static void
apply_fchmod(struct fildes_tx* self, int fildes, int cookie,
             struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_fchmod_apply(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_fchmod(struct fildes_tx* self, int fildes, int cookie,
            struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_fchmod_undo(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * fcntl()
 */

int
fildes_tx_exec_fcntl(struct fildes_tx* self, int fildes, int cmd,
                     union fcntl_arg* arg, int isnoundo,
                     struct picotm_error* error)
{
    assert(self);

    if ((cmd == F_DUPFD) || (cmd == F_DUPFD_CLOEXEC)) {
        /* Write-lock file-descriptor table to preserve file-descriptor order */
        fdtab_tx_try_wrlock(&self->fdtab_tx, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    /* Update/create fd_tx */
    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int cookie = -1;

    int res = fd_tx_fcntl_exec(fd_tx, fildes, cmd, arg, isnoundo, &cookie,
                               error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_FCNTL, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return res;
}

static void
apply_fcntl(struct fildes_tx* self, int fildes, int cookie,
            struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_fcntl_apply(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_fcntl(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_fcntl_undo(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * fstat()
 */

int
fildes_tx_exec_fstat(struct fildes_tx* self, int fildes, struct stat* buf,
                     int isnoundo, struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* fstat() */

    int cookie = -1;

    int res = fd_tx_fstat_exec(fd_tx, fildes, buf, isnoundo, &cookie,
                               error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_FSTAT, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return res;
}

static void
apply_fstat(struct fildes_tx* self, int fildes, int cookie,
            struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_fstat_apply(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_fstat(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_fstat_undo(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * fsync()
 */

int
fildes_tx_exec_fsync(struct fildes_tx* self, int fildes, int isnoundo,
                     struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Fsync */

    int cookie = -1;

    int res = fd_tx_fsync_exec(fd_tx, fildes, isnoundo, &cookie,
                               error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_FSYNC, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return res;
}

static void
apply_fsync(struct fildes_tx* self, int fildes, int cookie,
            struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_fsync_apply(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_fsync(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_fsync_undo(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * link()
 */

int
fildes_tx_exec_link(struct fildes_tx* self, const char* path1, const char* path2,
                    struct picotm_error* error)
{
    char* real_path1 = cwd_module_realpath(path1, NULL, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    char* abs_path2 = absolute_path(path2, error);
    if (picotm_error_is_set(error)) {
        goto err_absolute_path;
    }

    int res = link(real_path1, abs_path2);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        goto err_link;
    }

    free(abs_path2);
    allocator_module_free(real_path1, strlen(real_path1));

    return res;

err_link:
    free(abs_path2);
err_absolute_path:
    allocator_module_free(real_path1, strlen(real_path1));
    return -1;
}

/*
 * listen()
 */

int
fildes_tx_exec_listen(struct fildes_tx* self, int sockfd, int backlog,
                      int isnoundo, struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Connect */

    int cookie = -1;

    int res = fd_tx_listen_exec(fd_tx, sockfd, backlog, isnoundo, &cookie,
                                error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_LISTEN, sockfd, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return res;
}

static void
apply_listen(struct fildes_tx* self, int fildes, int cookie,
             struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_listen_apply(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_listen(struct fildes_tx* self, int fildes, int cookie,
            struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_listen_undo(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * lseek()
 */

off_t
fildes_tx_exec_lseek(struct fildes_tx* self, int fildes, off_t offset,
                     int whence, int isnoundo, struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return (off_t)-1;
    }

    /* Seek */

    int cookie = -1;

    off_t pos = fd_tx_lseek_exec(fd_tx, fildes, offset, whence,
                                 isnoundo, &cookie, error);
    if (picotm_error_is_set(error)) {
        return (off_t)-1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_LSEEK, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return (off_t)-1;
        }
    }

    return pos;
}

static void
apply_lseek(struct fildes_tx* self, int fildes, int cookie,
            struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_lseek_apply(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_lseek(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_lseek_undo(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * lstat()
 */

int
fildes_tx_exec_lstat(struct fildes_tx* self, const char* path, struct stat* buf,
                     struct picotm_error* error)
{
    char* real_path = cwd_module_realpath(path, NULL, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int res = lstat(real_path, buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        goto err_lstat;
    }

    allocator_module_free(real_path, strlen(real_path));

    return res;

err_lstat:
    allocator_module_free(real_path, strlen(real_path));
    return -1;
}

/*
 * mkdir()
 */

int
fildes_tx_exec_mkdir(struct fildes_tx* self, const char* path, mode_t mode,
                     struct picotm_error* error)
{
    char* abs_path = absolute_path(path, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int res = mkdir(abs_path, mode);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        goto err_mkdir;
    }

    free(abs_path);

    return res;

err_mkdir:
    free(abs_path);
    return -1;
}

/*
 * mkfifo()
 */

int
fildes_tx_exec_mkfifo(struct fildes_tx* self, const char* path, mode_t mode,
                      struct picotm_error* error)
{
    char* abs_path = absolute_path(path, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int res = mkfifo(abs_path, mode);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        goto err_mkfifo;
    }

    free(abs_path);

    return res;

err_mkfifo:
    free(abs_path);
    return -1;
}

/*
 * mknod()
 */

int
fildes_tx_exec_mknod(struct fildes_tx* self, const char* path, mode_t mode,
                     dev_t dev, struct picotm_error* error)
{
    char* abs_path = absolute_path(path, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int res = mknod(abs_path, mode, dev);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        goto err_mknod;
    }

    free(abs_path);

    return res;

err_mknod:
    free(abs_path);
    return -1;
}

/*
 * mkstemp()
 */

int
fildes_tx_exec_mkstemp(struct fildes_tx* self, char* pathname,
                       struct picotm_error* error)
{
    assert(self);
    assert(pathname);

    /* Write-lock file-descriptor table to preserve file-descriptor order */
    fdtab_tx_try_wrlock(&self->fdtab_tx, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Construct absolute pathname */

    char* abs_path = absolute_path(pathname, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Make call */

    int res = mkstemp(abs_path);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        goto err_mkstemp;
    }
    int fildes = res;

    /* Copy trailing filled XXXXXX back to pathname */
    memcpy(pathname + strlen(pathname) - 6,
           abs_path + strlen(abs_path) - 6, 6);

    append_cmd(self, CMD_MKSTEMP, fildes, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_append_cmd;
    }

    free(abs_path);

    return fildes;

err_append_cmd:
    unlink(abs_path);
err_mkstemp:
    free(abs_path);
    return -1;
}

static void
apply_mkstemp(struct fildes_tx* self, int fildes, int cookie,
              struct picotm_error* error)
{ }

/* Removing the temporary file is only possible because it is certain
 * that the transaction created that file initially.
 */
static void
undo_mkstemp(struct fildes_tx* self, int fildes, int cookie,
             struct picotm_error* error)
{
    /* don't care about errors */
    struct picotm_error ignored_error = PICOTM_ERROR_INITIALIZER;
    unlink_fildes(fildes, &ignored_error);

    int res = TEMP_FAILURE_RETRY(close(fildes));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

/*
 * open()
 */

#define DO_UNLINK(mode_) \
    ( ( (mode_)&(O_CREAT|O_EXCL) ) == (O_CREAT|O_EXCL) )

int
fildes_tx_exec_open(struct fildes_tx* self, const char* path, int oflag,
                    mode_t mode, int isnoundo, struct picotm_error* error)
{
    /* O_TRUNC needs irrevocability */

    if ((mode&O_TRUNC) && !isnoundo) {
        picotm_error_set_revocable(error);
        return -1;
    }

    /* Write-lock file-descriptor table to preserve file-descriptor order */
    fdtab_tx_try_wrlock(&self->fdtab_tx, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Open file */

    char* abs_path = absolute_path(path, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int fildes = TEMP_FAILURE_RETRY(open(abs_path, oflag, mode));
    if (fildes < 0) {
        picotm_error_set_errno(error, errno);
        goto err_open;
    }

    /* FIXME: Distinguish between open calls. Ofd only knows one file
              description, but each open creates a new open file description;
              File position might be wrong
              Ideas: Maybe introduce open index (0: outside of Tx,
              n>0 inside Tx), or maybe reset file position on commiting open */

    /* Update/create fd_tx */

    get_fd_tx_with_ref(self, fildes, FD_FL_WANTNEW, error);
    if (picotm_error_is_set(error)) {
        goto err_get_fd_tx_with_ref;
    }

    int cookie = openoptab_append(&self->openoptab,
                                  &self->openoptablen, DO_UNLINK(mode),
                                  error);
    if (picotm_error_is_set(error)) {
        goto err_openoptab_append;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_OPEN, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            goto err_append_cmd;
        }
    }

    free(abs_path);

    return fildes;

err_append_cmd:
err_openoptab_append:
err_get_fd_tx_with_ref:
    TEMP_FAILURE_RETRY(close(fildes));
err_open:
    free(abs_path);
    return -1;
}

static void
apply_open(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
}

static void
undo_open(struct fildes_tx* self, int fildes, int cookie,
          struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < MAXNUMFD);
    assert(cookie < (ssize_t)self->openoptablen);

    if (self->openoptab[cookie].unlink) {
        /* don't care about errors */
        struct picotm_error ignored_error = PICOTM_ERROR_INITIALIZER;
        unlink_fildes(fildes, &ignored_error);
    }

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    /* Mark file descriptor to be closed */
    fd_tx_signal_close(fd_tx);
}

/*
 * pipe()
 */

int
fildes_tx_exec_pipe(struct fildes_tx* self, int pipefd[2],
                    struct picotm_error* error)
{
    assert(self);

    /* Write-lock file-descriptor table to preserve file-descriptor order */
    fdtab_tx_try_wrlock(&self->fdtab_tx, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Create pipe */

    int res = TEMP_FAILURE_RETRY(pipe(pipefd));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }

    /* Update/create fd_tx */

    get_fd_tx_with_ref(self, pipefd[0], 0, error);
    if (picotm_error_is_set(error)) {
        if (TEMP_FAILURE_RETRY(close(pipefd[0])) < 0) {
            perror("close");
        }
        if (TEMP_FAILURE_RETRY(close(pipefd[1])) < 0) {
            perror("close");
        }
        return -1;
    }

    get_fd_tx_with_ref(self, pipefd[1], 0, error);
    if (picotm_error_is_set(error)) {
        if (TEMP_FAILURE_RETRY(close(pipefd[0])) < 0) {
            perror("close");
        }
        if (TEMP_FAILURE_RETRY(close(pipefd[1])) < 0) {
            perror("close");
        }
        return -1;
    }

    int cookie = pipeoptab_append(&self->pipeoptab,
                                  &self->pipeoptablen, pipefd,
                                  error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_PIPE, 0, cookie, error);
        if (picotm_error_is_set(error)) {
            if (TEMP_FAILURE_RETRY(close(pipefd[0])) < 0) {
                perror("close");
            }
            if (TEMP_FAILURE_RETRY(close(pipefd[1])) < 0) {
                perror("close");
            }
            return -1;
        }
    }

    return 0;
}

static void
apply_pipe(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
}

static void
undo_pipe(struct fildes_tx* self, int fildes, int cookie,
          struct picotm_error* error)
{
    assert(self);

    const struct pipeop* pipeop = self->pipeoptab+cookie;

    if (TEMP_FAILURE_RETRY(close(pipeop->pipefd[0])) < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
    if (TEMP_FAILURE_RETRY(close(pipeop->pipefd[1])) < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

/*
 * pread()
 */

ssize_t
fildes_tx_exec_pread(struct fildes_tx* self, int fildes, void* buf,
                     size_t nbyte, off_t off, int isnoundo,
                     struct picotm_error* error)
{
    assert(self);

    /* update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* pread */

    enum picotm_libc_validation_mode val_mode =
        fildes_tx_get_validation_mode(self);

    int cookie = -1;

    ssize_t len = fd_tx_pread_exec(fd_tx, fildes, buf, nbyte, off,
                                   isnoundo, val_mode, &cookie, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_PREAD, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return len;
}

static void
apply_pread(struct fildes_tx* self, int fildes, int cookie,
            struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_pread_apply(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_pread(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_pread_undo(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * pwrite()
 */

ssize_t
fildes_tx_exec_pwrite(struct fildes_tx* self, int fildes, const void* buf,
                      size_t nbyte, off_t off, int isnoundo,
                      struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Pwrite */

    int cookie = -1;

    ssize_t len = fd_tx_pwrite_exec(fd_tx, fildes, buf, nbyte, off,
                                    isnoundo, &cookie, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_PWRITE, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return len;
}

static void
apply_pwrite(struct fildes_tx* self, int fildes, int cookie,
             struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_pwrite_apply(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_pwrite(struct fildes_tx* self, int fildes, int cookie,
            struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_pwrite_undo(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * read()
 */

ssize_t
fildes_tx_exec_read(struct fildes_tx* self, int fildes, void* buf,
                    size_t nbyte, int isnoundo, struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Read */

    enum picotm_libc_validation_mode val_mode =
        fildes_tx_get_validation_mode(self);

    int cookie = -1;

    ssize_t len = fd_tx_read_exec(fd_tx, fildes, buf, nbyte,
                                  isnoundo, val_mode, &cookie, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_READ, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return len;
}

static void
apply_read(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_read_apply(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_read(struct fildes_tx* self, int fildes, int cookie,
          struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_read_undo(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * recv()
 */

ssize_t
fildes_tx_exec_recv(struct fildes_tx* self, int sockfd, void* buffer,
                    size_t length, int flags, int isnoundo,
                    struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Receive */

    int cookie = -1;

    ssize_t len = fd_tx_recv_exec(fd_tx, sockfd, buffer, length, flags,
                                  isnoundo, &cookie, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_RECV, sockfd, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return len;
}

static void
apply_recv(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_recv_apply(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_recv(struct fildes_tx* self, int fildes, int cookie,
          struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_recv_undo(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * select()
 */

static void
ref_fdset(struct fildes_tx* self, int nfds, const fd_set* fdset,
          struct picotm_error* error)
{
    assert(nfds > 0);
    assert(!nfds || fdset);

    int fildes;

    for (fildes = 0; fildes < nfds; ++fildes) {
        if (FD_ISSET(fildes, fdset)) {

            /* Update/create fd_tx */

            get_fd_tx_with_ref(self, fildes, 0, error);
            if (picotm_error_is_set(error)) {
                return;
            }
        }
    }
}

int
fildes_tx_exec_select(struct fildes_tx* self, int nfds, fd_set* readfds,
                      fd_set* writefds, fd_set* errorfds,
                      struct timeval* timeout, int isnoundo,
                      struct picotm_error* error)
{
    assert(self);

    /* Ref all selected file descriptors */

    if (readfds) {
        ref_fdset(self, nfds, readfds, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }
    if (writefds) {
        ref_fdset(self, nfds, writefds, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }
    if (errorfds) {
        ref_fdset(self, nfds, errorfds, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    int res;

    if (!timeout && !isnoundo) {

        /* Arbitrarily choosen default timeout of 5 sec */
        struct timeval def_timeout = {5, 0};

        res = TEMP_FAILURE_RETRY(select(nfds, readfds, writefds, errorfds,
                                        &def_timeout));
        if (res < 0) {
            picotm_error_set_errno(error, errno);
            return -1;
        }
    } else {
        res = TEMP_FAILURE_RETRY(select(nfds, readfds, writefds, errorfds,
                                        timeout));
        if (res < 0) {
            picotm_error_set_errno(error, errno);
            return -1;
        }
    }

    return res;
}

/*
 * send()
 */

ssize_t
fildes_tx_exec_send(struct fildes_tx* self, int sockfd, const void* buffer,
                    size_t length, int flags, int isnoundo,
                    struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Send */

    int cookie = -1;

    ssize_t len = fd_tx_send_exec(fd_tx, sockfd, buffer, length, flags,
                                  isnoundo, &cookie, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_SEND, sockfd, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return len;
}

static void
apply_send(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_send_apply(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_send(struct fildes_tx* self, int fildes, int cookie,
          struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_send_undo(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * shutdown()
 */

int
fildes_tx_exec_shutdown(struct fildes_tx* self, int sockfd, int how,
                        int isnoundo, struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Shutdown */

    int cookie = -1;

    int len = fd_tx_shutdown_exec(fd_tx, sockfd, how, isnoundo, &cookie,
                                  error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_SHUTDOWN, sockfd, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return len;
}

static void
apply_shutdown(struct fildes_tx* self, int fildes, int cookie,
               struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_shutdown_apply(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_shutdown(struct fildes_tx* self, int fildes, int cookie,
              struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_shutdown_undo(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * socket()
 */

int
fildes_tx_exec_socket(struct fildes_tx* self, int domain, int type,
                      int protocol, struct picotm_error* error)
{
    assert(self);

    /* Write-lock file-descriptor table to preserve file-descriptor order */
    fdtab_tx_try_wrlock(&self->fdtab_tx, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Create socket */

    int sockfd = TEMP_FAILURE_RETRY(socket(domain, type, protocol));
    if (sockfd < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }

    /* Update/create fd_tx */

    get_fd_tx_with_ref(self, sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        if (TEMP_FAILURE_RETRY(close(sockfd)) < 0) {
            perror("close");
        }
        return -1;
    }

    /* Inject event */
    append_cmd(self, CMD_SOCKET, sockfd, -1, error);
    if (picotm_error_is_set(error)) {
        if (TEMP_FAILURE_RETRY(close(sockfd)) < 0) {
            perror("close");
        }
        return -1;
    }

    return sockfd;
}

static void
apply_socket(struct fildes_tx* self, int fildes, int cookie,
             struct picotm_error* error)
{
    assert(self);
}

static void
undo_socket(struct fildes_tx* self, int fildes, int cookie,
            struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < MAXNUMFD);

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    /* Mark file descriptor to be closed. This works, because dup() occured
     * inside the transaction. So no other transaction should have access to
     * it. */
    fd_tx_signal_close(fd_tx);
}

/*
 * stat()
 */

int
fildes_tx_exec_stat(struct fildes_tx* self, const char* path,
                    struct stat* buf, struct picotm_error* error)
{
    char* real_path = cwd_module_realpath(path, NULL, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int res = stat(real_path, buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        goto err_stat;
    }

    allocator_module_free(real_path, strlen(real_path));

    return res;

err_stat:
    allocator_module_free(real_path, strlen(real_path));
    return -1;
}

/*
 * sync()
 */

void
fildes_tx_exec_sync(struct fildes_tx* self, struct picotm_error* error)
{
    assert(self);

    /* Sync */
    sync();

    /* Inject event */
    append_cmd(self, CMD_SYNC, -1, -1, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
apply_sync(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);

    sync();
}

static void
undo_sync(struct fildes_tx* self, int fildes, int cookie,
          struct picotm_error* error)
{ }

/*
 * unlink()
 */

int
fildes_tx_exec_unlink(struct fildes_tx* self, const char* path,
                      struct picotm_error* error)
{
    char* real_path = cwd_module_realpath(path, NULL, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int res = unlink(real_path);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        goto err_unlink;
    }

    allocator_module_free(real_path, strlen(real_path));

    return res;

err_unlink:
    allocator_module_free(real_path, strlen(real_path));
    return -1;
}

/*
 * write()
 */

ssize_t
fildes_tx_exec_write(struct fildes_tx* self, int fildes, const void* buf,
                     size_t nbyte, int isnoundo, struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Write */

    int cookie = -1;

    ssize_t len = fd_tx_write_exec(fd_tx, fildes, buf, nbyte,
                                   isnoundo, &cookie, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_WRITE, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return len;
}

static void
apply_write(struct fildes_tx* self, int fildes, int cookie,
            struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_write_apply(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_write(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_write_undo(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * Module interface
 */

void
fildes_tx_lock(struct fildes_tx* self, struct picotm_error* error)
{
    /* Lock fds */

    struct fd_tx* fd_tx;
    SLIST_FOREACH(fd_tx, &self->fd_tx_active_list, active_list) {
        fd_tx_lock(fd_tx);
    }

    /* Lock character devices */

    struct chrdev_tx* chrdev_tx;
    SLIST_FOREACH(chrdev_tx, &self->chrdev_tx_active_list, active_list) {
        chrdev_tx_lock(chrdev_tx);
    }

    /* Lock fifos */

    struct fifo_tx* fifo_tx;
    SLIST_FOREACH(fifo_tx, &self->fifo_tx_active_list, active_list) {
        fifo_tx_lock(fifo_tx);
    }

    /* Lock regular files */

    struct regfile_tx* regfile_tx;
    SLIST_FOREACH(regfile_tx, &self->regfile_tx_active_list, active_list) {
        regfile_tx_lock(regfile_tx);
    }

    /* Lock directories */

    struct dir_tx* dir_tx;
    SLIST_FOREACH(dir_tx, &self->dir_tx_active_list, active_list) {
        dir_tx_lock(dir_tx);
    }

    /* Lock sockets */

    struct socket_tx* socket_tx;
    SLIST_FOREACH(socket_tx, &self->socket_tx_active_list, active_list) {
        socket_tx_lock(socket_tx);
    }
}

void
fildes_tx_unlock(struct fildes_tx* self)
{
    /* Unlock sockets */

    struct socket_tx* socket_tx;
    SLIST_FOREACH(socket_tx, &self->socket_tx_active_list, active_list) {
        socket_tx_unlock(socket_tx);
    }

    /* Unlock directories */

    struct dir_tx* dir_tx;
    SLIST_FOREACH(dir_tx, &self->dir_tx_active_list, active_list) {
        dir_tx_unlock(dir_tx);
    }

    /* Unlock regular files */

    struct regfile_tx* regfile_tx;
    SLIST_FOREACH(regfile_tx, &self->regfile_tx_active_list, active_list) {
        regfile_tx_unlock(regfile_tx);
    }

    /* Unlock fifos */

    struct fifo_tx* fifo_tx;
    SLIST_FOREACH(fifo_tx, &self->fifo_tx_active_list, active_list) {
        fifo_tx_unlock(fifo_tx);
    }

    /* Unlock character devices */

    struct chrdev_tx* chrdev_tx;
    SLIST_FOREACH(chrdev_tx, &self->chrdev_tx_active_list, active_list) {
        chrdev_tx_unlock(chrdev_tx);
    }

    /* Unlock fds */

    struct fd_tx* fd_tx;
    SLIST_FOREACH(fd_tx, &self->fd_tx_active_list, active_list) {
        fd_tx_unlock(fd_tx);
    }
}

void
fildes_tx_validate(struct fildes_tx* self, int noundo,
                   struct picotm_error* error)
{
    /* Validate fd_txs */

    struct fd_tx* fd_tx;
    SLIST_FOREACH(fd_tx, &self->fd_tx_active_list, active_list) {
        fd_tx_validate(fd_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Validate chrdev_txs */

    struct chrdev_tx* chrdev_tx;
    SLIST_FOREACH(chrdev_tx, &self->chrdev_tx_active_list, active_list) {
        chrdev_tx_validate(chrdev_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Validate fifo_txs */

    struct fifo_tx* fifo_tx;
    SLIST_FOREACH(fifo_tx, &self->fifo_tx_active_list, active_list) {
        fifo_tx_validate(fifo_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Validate regfile_txs */

    struct regfile_tx* regfile_tx;
    SLIST_FOREACH(regfile_tx, &self->regfile_tx_active_list, active_list) {
        regfile_tx_validate(regfile_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Validate dir_txs */

    struct dir_tx* dir_tx;
    SLIST_FOREACH(dir_tx, &self->dir_tx_active_list, active_list) {
        dir_tx_validate(dir_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Validate socket_txs */

    struct socket_tx* socket_tx;
    SLIST_FOREACH(socket_tx, &self->socket_tx_active_list, active_list) {
        socket_tx_validate(socket_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
}

void
fildes_tx_apply_event(struct fildes_tx* self,
                      const struct picotm_event* event,
                      struct picotm_error* error)
{
    static void (* const apply[])(struct fildes_tx*,
                                  int,
                                  int,
                                  struct picotm_error*) = {
        apply_accept,
        apply_bind,
        apply_close,
        apply_connect,
        apply_dup,
        apply_fchmod,
        apply_fcntl,
        apply_fstat,
        apply_fsync,
        apply_listen,
        apply_lseek,
        apply_mkstemp,
        apply_open,
        apply_pipe,
        apply_pread,
        apply_pwrite,
        apply_read,
        apply_recv,
        apply_send,
        apply_shutdown,
        apply_socket,
        apply_sync,
        apply_write
    };

    apply[event->call](self,
                       self->eventtab[event->cookie].fildes,
                       self->eventtab[event->cookie].cookie,
                       error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fildes_tx_undo_event(struct fildes_tx* self,
                     const struct picotm_event* event,
                     struct picotm_error* error)
{
    static void (* const undo[])(struct fildes_tx*,
                                 int,
                                 int,
                                 struct picotm_error*) = {
        undo_accept,
        undo_bind,
        undo_close,
        undo_connect,
        undo_dup,
        undo_fchmod,
        undo_fcntl,
        undo_fstat,
        undo_fsync,
        undo_listen,
        undo_lseek,
        undo_mkstemp,
        undo_open,
        undo_pipe,
        undo_pread,
        undo_pwrite,
        undo_read,
        undo_recv,
        undo_send,
        undo_shutdown,
        undo_socket,
        undo_sync,
        undo_write
    };

    undo[event->call](self,
                      self->eventtab[event->cookie].fildes,
                      self->eventtab[event->cookie].cookie,
                      error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fildes_tx_update_cc(struct fildes_tx* self, int noundo,
                    struct picotm_error* error)
{
    /* Update fd_txs */

    struct fd_tx* fd_tx;
    SLIST_FOREACH(fd_tx, &self->fd_tx_active_list, active_list) {
        fd_tx_update_cc(fd_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Update chrdev_txs */

    struct chrdev_tx* chrdev_tx;
    SLIST_FOREACH(chrdev_tx, &self->chrdev_tx_active_list, active_list) {
        chrdev_tx_update_cc(chrdev_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Update fifo_txs */

    struct fifo_tx* fifo_tx;
    SLIST_FOREACH(fifo_tx, &self->fifo_tx_active_list, active_list) {
        fifo_tx_update_cc(fifo_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Update regfile_txs */

    struct regfile_tx* regfile_tx;
    SLIST_FOREACH(regfile_tx, &self->regfile_tx_active_list, active_list) {
        regfile_tx_update_cc(regfile_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Update dir_txs */

    struct dir_tx* dir_tx;
    SLIST_FOREACH(dir_tx, &self->dir_tx_active_list, active_list) {
        dir_tx_update_cc(dir_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Update socket_txs */

    struct socket_tx* socket_tx;
    SLIST_FOREACH(socket_tx, &self->socket_tx_active_list, active_list) {
        socket_tx_update_cc(socket_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Update concurrency control on file-descriptor table */
    fdtab_tx_update_cc(&self->fdtab_tx, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fildes_tx_clear_cc(struct fildes_tx* self, int noundo,
                   struct picotm_error* error)
{
    /* Clear fd_txs' CC */

    struct fd_tx* fd_tx;
    SLIST_FOREACH(fd_tx, &self->fd_tx_active_list, active_list) {
        fd_tx_clear_cc(fd_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Clear chrdev_txs' CC */

    struct chrdev_tx* chrdev_tx;
    SLIST_FOREACH(chrdev_tx, &self->chrdev_tx_active_list, active_list) {
        chrdev_tx_clear_cc(chrdev_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Clear fifo_txs' CC */

    struct fifo_tx* fifo_tx;
    SLIST_FOREACH(fifo_tx, &self->fifo_tx_active_list, active_list) {
        fifo_tx_clear_cc(fifo_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Clear regfile_txs' CC */

    struct regfile_tx* regfile_tx;
    SLIST_FOREACH(regfile_tx, &self->regfile_tx_active_list, active_list) {
        regfile_tx_clear_cc(regfile_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Clear dir_txs' CC */

    struct dir_tx* dir_tx;
    SLIST_FOREACH(dir_tx, &self->dir_tx_active_list, active_list) {
        dir_tx_clear_cc(dir_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Clear socket_txs' CC */

    struct socket_tx* socket_tx;
    SLIST_FOREACH(socket_tx, &self->socket_tx_active_list, active_list) {
        socket_tx_clear_cc(socket_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Clear concurrency control on file-descriptor table */
    fdtab_tx_update_cc(&self->fdtab_tx, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fildes_tx_finish(struct fildes_tx* self, struct picotm_error* error)
{
    /* Unref socket_txs */

    while (!SLIST_EMPTY(&self->socket_tx_active_list)) {

        struct socket_tx* socket_tx = SLIST_FIRST(&self->socket_tx_active_list);
        SLIST_REMOVE_HEAD(&self->socket_tx_active_list, active_list);

        socket_tx_unref(socket_tx);
    }

    /* Unref dir_txs */

    while (!SLIST_EMPTY(&self->dir_tx_active_list)) {

        struct dir_tx* dir_tx = SLIST_FIRST(&self->dir_tx_active_list);
        SLIST_REMOVE_HEAD(&self->dir_tx_active_list, active_list);

        dir_tx_unref(dir_tx);
    }

    /* Unref regfile_txs */

    while (!SLIST_EMPTY(&self->regfile_tx_active_list)) {

        struct regfile_tx* regfile_tx = SLIST_FIRST(&self->regfile_tx_active_list);
        SLIST_REMOVE_HEAD(&self->regfile_tx_active_list, active_list);

        regfile_tx_unref(regfile_tx);
    }

    /* Unref fifo_txs */

    while (!SLIST_EMPTY(&self->fifo_tx_active_list)) {

        struct fifo_tx* fifo_tx = SLIST_FIRST(&self->fifo_tx_active_list);
        SLIST_REMOVE_HEAD(&self->fifo_tx_active_list, active_list);

        fifo_tx_unref(fifo_tx);
    }

    /* Unref chrdev_txs */

    while (!SLIST_EMPTY(&self->chrdev_tx_active_list)) {

        struct chrdev_tx* chrdev_tx = SLIST_FIRST(&self->chrdev_tx_active_list);
        SLIST_REMOVE_HEAD(&self->chrdev_tx_active_list, active_list);

        chrdev_tx_unref(chrdev_tx);
    }

    /* Unref fd_txs */

    while (!SLIST_EMPTY(&self->fd_tx_active_list)) {

        struct fd_tx* fd_tx = SLIST_FIRST(&self->fd_tx_active_list);
        SLIST_REMOVE_HEAD(&self->fd_tx_active_list, active_list);

        fd_tx_unref(fd_tx);
    }
}
