/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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

#include "fildes_tx.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "allocator/module.h"
#include "chrdev.h"
#include "chrdev_tx.h"
#include "compat/temp_failure_retry.h"
#include "cwd/module.h"
#include "dir.h"
#include "dir_tx.h"
#include "fifo.h"
#include "fifo_tx.h"
#include "fildes.h"
#include "fildes_log.h"
#include "ofdtab.h"
#include "openop.h"
#include "openoptab.h"
#include "pipeop.h"
#include "pipeoptab.h"
#include "range.h"
#include "regfile.h"
#include "regfile_tx.h"
#include "socket.h"
#include "socket_tx.h"

struct file_tx_mem {
    union {
        struct chrdev_tx  chrdev_tx;
        struct dir_tx     dir_tx;
        struct fifo_tx    fifo_tx;
        struct regfile_tx regfile_tx;
        struct socket_tx  socket_tx;
    } entry;
};

static struct file_tx_mem*
file_tx_mem_of_entry_base(struct file_tx* base)
{
    switch (file_tx_file_type(base)) {
        case PICOTM_LIBC_FILE_TYPE_CHRDEV:
            return picotm_containerof(base, struct file_tx_mem,
                                      entry.chrdev_tx.base);
        case PICOTM_LIBC_FILE_TYPE_DIR:
            return picotm_containerof(base, struct file_tx_mem,
                                      entry.dir_tx.base);
        case PICOTM_LIBC_FILE_TYPE_FIFO:
            return picotm_containerof(base, struct file_tx_mem,
                                      entry.fifo_tx.base);
        case PICOTM_LIBC_FILE_TYPE_REGULAR:
            return picotm_containerof(base, struct file_tx_mem,
                                      entry.regfile_tx.base);
        case PICOTM_LIBC_FILE_TYPE_SOCKET:
            return picotm_containerof(base, struct file_tx_mem,
                                      entry.socket_tx.base);
    }
    return NULL;
}

static struct file_tx*
file_tx_create(enum picotm_libc_file_type type, struct picotm_error* error)
{
    struct file_tx_mem* mem = malloc(sizeof(*mem));
    if (!mem) {
        picotm_error_set_errno(error, errno);
        return NULL;
    }
    switch (type) {
        case PICOTM_LIBC_FILE_TYPE_CHRDEV:
            chrdev_tx_init(&mem->entry.chrdev_tx);
            return &mem->entry.chrdev_tx.base;
        case PICOTM_LIBC_FILE_TYPE_DIR:
            dir_tx_init(&mem->entry.dir_tx);
            return &mem->entry.dir_tx.base;
        case PICOTM_LIBC_FILE_TYPE_FIFO:
            fifo_tx_init(&mem->entry.fifo_tx);
            return &mem->entry.fifo_tx.base;
        case PICOTM_LIBC_FILE_TYPE_REGULAR:
            regfile_tx_init(&mem->entry.regfile_tx);
            return &mem->entry.regfile_tx.base;
        case PICOTM_LIBC_FILE_TYPE_SOCKET:
            socket_tx_init(&mem->entry.socket_tx);
            return &mem->entry.socket_tx.base;
    }
    return NULL;
}

static void
file_tx_destroy(struct file_tx* self)
{
    struct file_tx_mem* mem = file_tx_mem_of_entry_base(self);
    switch (file_tx_file_type(self)) {
        case PICOTM_LIBC_FILE_TYPE_CHRDEV:
            chrdev_tx_uninit(&mem->entry.chrdev_tx);
            break;
        case PICOTM_LIBC_FILE_TYPE_DIR:
            dir_tx_uninit(&mem->entry.dir_tx);
            break;
        case PICOTM_LIBC_FILE_TYPE_FIFO:
            fifo_tx_uninit(&mem->entry.fifo_tx);
            break;
        case PICOTM_LIBC_FILE_TYPE_REGULAR:
            regfile_tx_uninit(&mem->entry.regfile_tx);
            break;
        case PICOTM_LIBC_FILE_TYPE_SOCKET:
            socket_tx_uninit(&mem->entry.socket_tx);
            break;
    }
    free(mem);
}

static void
file_tx_retype(struct file_tx* self, enum picotm_libc_file_type type)
{
    enum picotm_libc_file_type cur_type = file_tx_file_type(self);
    if (cur_type == type) {
        return;
    }

    struct file_tx_mem* mem = file_tx_mem_of_entry_base(self);

    switch (file_tx_file_type(self)) {
        case PICOTM_LIBC_FILE_TYPE_CHRDEV:
            chrdev_tx_uninit(&mem->entry.chrdev_tx);
            break;
        case PICOTM_LIBC_FILE_TYPE_DIR:
            dir_tx_uninit(&mem->entry.dir_tx);
            break;
        case PICOTM_LIBC_FILE_TYPE_FIFO:
            fifo_tx_uninit(&mem->entry.fifo_tx);
            break;
        case PICOTM_LIBC_FILE_TYPE_REGULAR:
            regfile_tx_uninit(&mem->entry.regfile_tx);
            break;
        case PICOTM_LIBC_FILE_TYPE_SOCKET:
            socket_tx_uninit(&mem->entry.socket_tx);
            break;
    }
    switch (type) {
        case PICOTM_LIBC_FILE_TYPE_CHRDEV:
            chrdev_tx_init(&mem->entry.chrdev_tx);
            break;
        case PICOTM_LIBC_FILE_TYPE_DIR:
            dir_tx_init(&mem->entry.dir_tx);
            break;
        case PICOTM_LIBC_FILE_TYPE_FIFO:
            fifo_tx_init(&mem->entry.fifo_tx);
            break;
        case PICOTM_LIBC_FILE_TYPE_REGULAR:
            regfile_tx_init(&mem->entry.regfile_tx);
            break;
        case PICOTM_LIBC_FILE_TYPE_SOCKET:
            socket_tx_init(&mem->entry.socket_tx);
            break;
    }
}

void
fildes_tx_init(struct fildes_tx* self, struct fildes* fildes,
               struct fildes_log* log)
{
    assert(self);

    self->fildes = fildes;
    self->log = log;

    fdtab_tx_init(&self->fdtab_tx, fildes);

    self->fd_tx_max_fildes = 0;

    picotm_slist_init_head(&self->fd_tx_active_list);

    picotm_slist_init_head(&self->file_tx_alloced_list);
    picotm_slist_init_head(&self->file_tx_active_list);

    self->ofd_tx_max_index = 0;

    picotm_slist_init_head(&self->ofd_tx_active_list);

    self->openoptab = NULL;
    self->openoptablen = 0;

    self->pipeoptab = NULL;
    self->pipeoptablen = 0;
}

static void
cleanup_file_tx_alloced_list_cb(struct picotm_slist* item)
{
    struct file_tx* file_tx = file_tx_of_list_entry(item);
    file_tx_destroy(file_tx);
}

void
fildes_tx_uninit(struct fildes_tx* self)
{
    /* Uninit allocated instances of |struct file_tx| */
    picotm_slist_cleanup_0(&self->file_tx_alloced_list,
                           cleanup_file_tx_alloced_list_cb);
    picotm_slist_uninit_head(&self->file_tx_alloced_list);
    picotm_slist_uninit_head(&self->file_tx_active_list);

    /* Uninit ofd_txs */

    for (struct ofd_tx* ofd_tx = self->ofd_tx;
                        ofd_tx < self->ofd_tx + self->ofd_tx_max_index;
                      ++ofd_tx) {
        ofd_tx_uninit(ofd_tx);
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

    picotm_slist_uninit_head(&self->ofd_tx_active_list);
    picotm_slist_uninit_head(&self->fd_tx_active_list);
}

static bool
true_if_eq_id_cb(const struct picotm_slist* item, void* data)
{
    const struct file_tx* file_tx =
        file_tx_of_list_entry((struct picotm_slist*)item);
    assert(file_tx);

    const struct file_id* id = data;
    assert(id);

    int cmp = file_id_cmp(&file_tx->file->id, id);
    return !cmp;
}

static struct file_tx*
get_file_tx(struct fildes_tx* self, int fildes,
            enum picotm_libc_file_type type,
            struct picotm_error* error)
{
    struct file_id id;
    file_id_init_from_fildes(&id, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    /* Let's see if we already have the file in use.*/

    struct picotm_slist* pos = picotm_slist_find_1(&self->file_tx_active_list,
                                                   true_if_eq_id_cb, &id);
    if (pos != picotm_slist_end(&self->file_tx_active_list)) {
        struct file_tx* file_tx = file_tx_of_list_entry(pos);
        assert(file_tx_file_type(file_tx) == type);
        return file_tx;
    }

    /* If not, we allocate a new entry either from our LRU
     * list, or from heap memory.
     */

    struct file_tx* file_tx;

    pos = picotm_slist_front(&self->file_tx_alloced_list);
    if (pos) {
        picotm_slist_dequeue_front(&self->file_tx_alloced_list);
        file_tx = file_tx_of_list_entry(pos);
        file_tx_retype(file_tx, type);
    } else {
        file_tx = file_tx_create(type, error);
        if (picotm_error_is_set(error)) {
            return NULL;
        }
    }

    return file_tx;
}

static struct chrdev_tx*
get_chrdev_tx(struct fildes_tx* self, int fildes, struct picotm_error* error)
{
    struct file_tx* file_tx = get_file_tx(self, fildes,
                                          PICOTM_LIBC_FILE_TYPE_CHRDEV,
                                          error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return chrdev_tx_of_file_tx(file_tx);
}

static struct chrdev_tx*
get_chrdev_tx_with_ref(struct fildes_tx* self, int fildes,
                       struct picotm_error* error)
{
    struct chrdev_tx* chrdev_tx = get_chrdev_tx(self, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    } else if (file_tx_holds_ref(&chrdev_tx->base)) {
        return chrdev_tx;
    }

    /* In |struct fildes_tx| we hold at most one reference to the
     * transaction state of each character device. This reference is
     * released in fildes_tx_finish().
     */
    struct chrdev* chrdev = fildes_ref_chrdev(self->fildes, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    file_tx_ref_or_set_up(&chrdev_tx->base, &chrdev->base, error);
    if (picotm_error_is_set(error)) {
        goto err_file_tx_ref_or_set_up;
    }

    picotm_slist_enqueue_front(&self->file_tx_active_list,
                               &chrdev_tx->base.list_entry);

    file_unref(&chrdev->base);

    return chrdev_tx;

err_file_tx_ref_or_set_up:
    file_unref(&chrdev->base);
    return NULL;
}

static struct fifo_tx*
get_fifo_tx(struct fildes_tx* self, int fildes, struct picotm_error* error)
{
    struct file_tx* file_tx = get_file_tx(self, fildes,
                                          PICOTM_LIBC_FILE_TYPE_FIFO,
                                          error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return fifo_tx_of_file_tx(file_tx);
}

static struct fifo_tx*
get_fifo_tx_with_ref(struct fildes_tx* self, int fildes,
                     struct picotm_error* error)
{
    struct fifo_tx* fifo_tx = get_fifo_tx(self, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    } else if (file_tx_holds_ref(&fifo_tx->base)) {
        return fifo_tx;
    }

    /* In |struct fildes_tx| we hold at most one reference to the
     * transaction state of each FIFO. This reference is released
     * in fildes_tx_finish().
     */
    struct fifo* fifo = fildes_ref_fifo(self->fildes, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    file_tx_ref_or_set_up(&fifo_tx->base, &fifo->base, error);
    if (picotm_error_is_set(error)) {
        goto err_file_tx_ref_or_set_up;
    }

    picotm_slist_enqueue_front(&self->file_tx_active_list,
                               &fifo_tx->base.list_entry);

    file_unref(&fifo->base);

    return fifo_tx;

err_file_tx_ref_or_set_up:
    file_unref(&fifo->base);
    return NULL;
}

static struct regfile_tx*
get_regfile_tx(struct fildes_tx* self, int fildes, struct picotm_error* error)
{
    struct file_tx* file_tx = get_file_tx(self, fildes,
                                          PICOTM_LIBC_FILE_TYPE_REGULAR,
                                          error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return regfile_tx_of_file_tx(file_tx);
}

static struct regfile_tx*
get_regfile_tx_with_ref(struct fildes_tx* self, int fildes,
                        struct picotm_error* error)
{
    struct regfile_tx* regfile_tx = get_regfile_tx(self, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    } else if (file_tx_holds_ref(&regfile_tx->base)) {
        return regfile_tx;
    }

    /* In |struct fildes_tx| we hold at most one reference to the
     * transaction state of each regular file. This reference is
     * released in fildes_tx_finish().
     */
    struct regfile* regfile = fildes_ref_regfile(self->fildes, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    file_tx_ref_or_set_up(&regfile_tx->base, &regfile->base, error);
    if (picotm_error_is_set(error)) {
        goto err_file_tx_ref_or_set_up;
    }

    picotm_slist_enqueue_front(&self->file_tx_active_list,
                               &regfile_tx->base.list_entry);

    file_unref(&regfile->base);

    return regfile_tx;

err_file_tx_ref_or_set_up:
    file_unref(&regfile->base);
    return NULL;
}

static struct dir_tx*
get_dir_tx(struct fildes_tx* self, int fildes, struct picotm_error* error)
{
    struct file_tx* file_tx = get_file_tx(self, fildes,
                                          PICOTM_LIBC_FILE_TYPE_DIR,
                                          error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return dir_tx_of_file_tx(file_tx);
}

static struct dir_tx*
get_dir_tx_with_ref(struct fildes_tx* self, int fildes,
                    struct picotm_error* error)
{
    struct dir_tx* dir_tx = get_dir_tx(self, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    } else if (file_tx_holds_ref(&dir_tx->base)) {
        return dir_tx;
    }

    /* In |struct fildes_tx| we hold at most one reference to the
     * transaction state of each directory. This reference is
     * released in fildes_tx_finish().
     */
    struct dir* dir = fildes_ref_dir(self->fildes, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    file_tx_ref_or_set_up(&dir_tx->base, &dir->base, error);
    if (picotm_error_is_set(error)) {
        goto err_file_tx_ref_or_set_up;
    }

    picotm_slist_enqueue_front(&self->file_tx_active_list,
                               &dir_tx->base.list_entry);

    file_unref(&dir->base);

    return dir_tx;

err_file_tx_ref_or_set_up:
    file_unref(&dir->base);
    return NULL;
}

static struct socket_tx*
get_socket_tx(struct fildes_tx* self, int fildes, struct picotm_error* error)
{
    struct file_tx* file_tx = get_file_tx(self, fildes,
                                          PICOTM_LIBC_FILE_TYPE_SOCKET,
                                          error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return socket_tx_of_file_tx(file_tx);
}

static struct socket_tx*
get_socket_tx_with_ref(struct fildes_tx* self, int fildes,
                       struct picotm_error* error)
{
    struct socket_tx* socket_tx = get_socket_tx(self, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    } else if (file_tx_holds_ref(&socket_tx->base)) {
        return socket_tx;
    }

    /* In |struct fildes_tx| we hold at most one reference to the
     * transaction state of each socket. This reference is released
     * in fildes_tx_finish().
     */
    struct socket* socket = fildes_ref_socket(self->fildes, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    file_tx_ref_or_set_up(&socket_tx->base, &socket->base, error);
    if (picotm_error_is_set(error)) {
        goto err_file_tx_ref_or_set_up;
    }

    picotm_slist_enqueue_front(&self->file_tx_active_list,
                               &socket_tx->base.list_entry);

    file_unref(&socket->base);

    return socket_tx;

err_file_tx_ref_or_set_up:
    file_unref(&socket->base);
    return NULL;
}

static struct file_tx*
get_file_tx_with_ref(struct fildes_tx* self, int fildes,
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
                                                                 error);
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return &chrdev_tx->base;
        }
        case S_IFIFO: {
            struct fifo_tx* fifo_tx = get_fifo_tx_with_ref(self,
                                                           fildes,
                                                           error);
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return &fifo_tx->base;
        }
        case S_IFREG: {
            struct regfile_tx* regfile_tx = get_regfile_tx_with_ref(self,
                                                                    fildes,
                                                                    error);
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return &regfile_tx->base;
        }
        case S_IFDIR: {
            struct dir_tx* dir_tx = get_dir_tx_with_ref(self,
                                                        fildes,
                                                        error);
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return &dir_tx->base;
        }
        case S_IFSOCK: {
            struct socket_tx* socket_tx = get_socket_tx_with_ref(self,
                                                                 fildes,
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

static struct ofd_tx*
get_ofd_tx(struct fildes_tx* self, const struct ofd_id* id,
           struct picotm_error* error)
{
    struct ofd_tx* empty = NULL;

    struct ofd_tx* beg = self->ofd_tx;
    const struct ofd_tx* end = self->ofd_tx + self->ofd_tx_max_index;

    for (; beg < end; ++beg) {

        if (!ofd_tx_holds_ref(beg)) {
            if (!empty) {
                empty = beg;
            }
            continue;
        }

        int cmp = ofd_tx_cmp_with_id(beg, id);
        if (!cmp) {
            return beg; /* found correct entry; return */
        }
    }

    /* no entry found; return an empty element */

    if (empty) {
        return empty;
    }

    empty = self->ofd_tx + self->ofd_tx_max_index;

    if (empty == picotm_arrayend(self->ofd_tx)) {
        picotm_error_set_error_code(error, PICOTM_OUT_OF_MEMORY);
        return NULL;
    }

    ofd_tx_init(empty);
    ++self->ofd_tx_max_index;

    return empty;
}

static struct ofd_tx*
get_ofd_tx_with_ref(struct fildes_tx* self, int fildes, bool newly_created,
                    struct picotm_error* error)
{
    struct ofd_id id;
    ofd_id_init_from_fildes(&id, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct ofd_tx* ofd_tx = get_ofd_tx(self, &id, error);
    if (picotm_error_is_set(error)) {
        goto err_get_ofd_tx;
    }

    if (ofd_tx_holds_ref(ofd_tx)) {
        return ofd_tx; /* fast path: already holds a reference */
    }

    struct file_tx* file_tx = get_file_tx_with_ref(self, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_get_file_tx_with_ref;
    }

    struct ofd* ofd = fildes_ref_ofd(self->fildes, fildes, newly_created,
                                     error);
    if (picotm_error_is_set(error)) {
        goto err_ofdtab_ref_fildes;
    }

    ofd_tx_ref_or_set_up(ofd_tx, ofd, file_tx, error);
    if (picotm_error_is_set(error)) {
        goto err_ofd_tx_ref_or_set_up;
    }

    picotm_slist_enqueue_front(&self->ofd_tx_active_list,
                               &ofd_tx->active_list);

    /* In |struct fildes_tx| we hold at most one reference to the
     * transaction state of each open file description. This reference is
     * released in fildes_tx_finish().
     */
    ofd_unref(ofd);

    ofd_id_uninit(&id);

    return ofd_tx;

err_ofd_tx_ref_or_set_up:
    ofd_unref(ofd);
err_ofdtab_ref_fildes:
err_get_file_tx_with_ref:
err_get_ofd_tx:
    ofd_id_uninit(&id);
    return NULL;
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
find_fd_tx_with_ref(struct fildes_tx* self, int fildes, int ofd_fildes,
                    bool newly_created_ofd, struct picotm_error* error)
{
    struct fd_tx* fd_tx = get_fd_tx(self, fildes);

    /* In |struct fildes_tx| we hold at most one reference to the
     * transaction state of each file descriptor. This reference
     * is released in fildes_tx_finish().
     */
    if (fd_tx_holds_ref(fd_tx)) {

        /* Validate reference or return error if fd has been closed */
        fd_tx_validate(fd_tx, error);
        if (picotm_error_is_set(error)) {
            return NULL;
        }

        return fd_tx;
    }

    struct fd* fd = fdtab_tx_ref_fildes(&self->fdtab_tx, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct ofd_tx* ofd_tx = get_ofd_tx_with_ref(self, ofd_fildes,
                                                newly_created_ofd,
                                                error);
    if (picotm_error_is_set(error)) {
        goto err_get_ofd_tx_with_ref;
    }

    fd_tx_ref_or_set_up(fd_tx, fd, ofd_tx, error);
    if (picotm_error_is_set(error)) {
        goto err_fd_tx_ref;
    }

    picotm_slist_enqueue_front(&self->fd_tx_active_list, &fd_tx->active_list);

    fd_unref(fd);

    return fd_tx;

err_fd_tx_ref:
    ofd_tx_unref(ofd_tx);
err_get_ofd_tx_with_ref:
    fd_unref(fd);
    return NULL;
}

static struct fd_tx*
get_fd_tx_with_ref(struct fildes_tx* self, int fildes,
                   struct picotm_error* error)
{
    return find_fd_tx_with_ref(self, fildes, fildes, false, error);
}

static struct fd_tx*
add_new_fd_tx_with_ref(struct fildes_tx* self, int fildes, bool newly_created_ofd,
                       struct picotm_error* error)
{
    return find_fd_tx_with_ref(self, fildes, fildes, newly_created_ofd, error);
}

static struct fd_tx*
add_dup_fd_tx_with_ref(struct fildes_tx* self, int fildes, int old_fildes,
                       struct picotm_error* error)
{
    return find_fd_tx_with_ref(self, fildes, old_fildes, false, error);
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

    size_t len = cwdlen + 1 + pathlen + 1;

    char* abs_path = malloc(len * sizeof(*abs_path));
    if (!abs_path) {
        picotm_error_set_errno(error, errno);
        goto err_malloc;
    }

    memcpy(abs_path, cwd, cwdlen);
    abs_path[cwdlen] = '/';
    memcpy(abs_path + cwdlen + 1, path, pathlen);
    abs_path[cwdlen + 1 + pathlen] = '\0';

    allocator_module_free(cwd, cwdlen, error);
    if (picotm_error_is_set(error)) {
        goto err_allocator_module_free;
    }

    return abs_path;

err_allocator_module_free:
    free(abs_path);
    picotm_error_mark_as_non_recoverable(error);
    return NULL;
err_malloc:
    {
        struct picotm_error tmp_error = PICOTM_ERROR_INITIALIZER;
        allocator_module_free(cwd, cwdlen, &tmp_error);
        if (picotm_error_is_set(&tmp_error)) {
            picotm_error_mark_as_non_recoverable(error);
        }
    }
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

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, sockfd, error);
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

    add_new_fd_tx_with_ref(self, connfd, true, error);
    if (picotm_error_is_set(error)) {
        goto err_add_new_fd_tx_with_ref;
    }

    /* Inject event */
    fildes_log_append(self->log, FILDES_OP_ACCEPT, connfd, cookie, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    return connfd;

err_add_new_fd_tx_with_ref:
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

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, socket, error);
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
        fildes_log_append(self->log, FILDES_OP_BIND, socket, cookie, error);
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

    allocator_module_free(real_path, strlen(real_path), error);
    if (picotm_error_is_set(error)) {
        goto err_allocator_module_free;
    }

    return res;

err_allocator_module_free:
    picotm_error_mark_as_non_recoverable(error);
    return -1;
err_chmod:
    {
        struct picotm_error tmp_error = PICOTM_ERROR_INITIALIZER;
        allocator_module_free(real_path, strlen(real_path), &tmp_error);
        if (picotm_error_is_set(&tmp_error)) {
            picotm_error_mark_as_non_recoverable(error);
        }
    }
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

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, error);
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
        fildes_log_append(self->log, FILDES_OP_CLOSE, fildes, cookie, error);
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

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, sockfd, error);
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
        fildes_log_append(self->log, FILDES_OP_CONNECT, sockfd, cookie, error);
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

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, error);
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

    add_dup_fd_tx_with_ref(self, new_fildes, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_add_dup_fd_tx_with_ref;
    }

    /* Inject event */
    fildes_log_append(self->log, FILDES_OP_DUP, new_fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        goto err_fildes_log_append;
    }

    return new_fildes;

err_fildes_log_append:
err_add_dup_fd_tx_with_ref:
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
    get_fd_tx_with_ref(self, fildes, error);
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
    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, error);
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
        fildes_log_append(self->log, FILDES_OP_FCHMOD, fildes, cookie, error);
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

#if defined(F_DUPFD_CLOEXEC)
    if ((cmd == F_DUPFD) || (cmd == F_DUPFD_CLOEXEC)) {
#else
    if (cmd == F_DUPFD) {
#endif
        /* Write-lock file-descriptor table to preserve file-descriptor order */
        fdtab_tx_try_wrlock(&self->fdtab_tx, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    /* Update/create fd_tx */
    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, error);
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
        fildes_log_append(self->log, FILDES_OP_FCNTL, fildes, cookie, error);
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

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, error);
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
        fildes_log_append(self->log, FILDES_OP_FSTAT, fildes, cookie, error);
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

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, error);
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
        fildes_log_append(self->log, FILDES_OP_FSYNC, fildes, cookie, error);
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

    allocator_module_free(real_path1, strlen(real_path1), error);
    if (picotm_error_is_set(error)) {
        goto err_allocator_module_free;
    }

    return res;

err_allocator_module_free:
    picotm_error_mark_as_non_recoverable(error);
    return -1;
err_link:
    free(abs_path2);
err_absolute_path:
    {
        struct picotm_error tmp_error = PICOTM_ERROR_INITIALIZER;
        allocator_module_free(real_path1, strlen(real_path1), &tmp_error);
        if (picotm_error_is_set(&tmp_error)) {
            picotm_error_mark_as_non_recoverable(error);
        }
    }
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

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, sockfd, error);
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
        fildes_log_append(self->log, FILDES_OP_LISTEN, sockfd, cookie, error);
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

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, error);
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
        fildes_log_append(self->log, FILDES_OP_LSEEK, fildes, cookie, error);
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

    allocator_module_free(real_path, strlen(real_path), error);
    if (picotm_error_is_set(error)) {
        goto err_allocator_module_free;
    }

    return res;

err_allocator_module_free:
    picotm_error_mark_as_non_recoverable(error);
    return -1;
err_lstat:
    {
        struct picotm_error tmp_error = PICOTM_ERROR_INITIALIZER;
        allocator_module_free(real_path, strlen(real_path), &tmp_error);
        if (picotm_error_is_set(&tmp_error)) {
            picotm_error_mark_as_non_recoverable(error);
        }
    }
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

    fildes_log_append(self->log, FILDES_OP_MKSTEMP, fildes, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_fildes_log_append;
    }

    free(abs_path);

    return fildes;

err_fildes_log_append:
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

    /* Update/create fd_tx */

    add_new_fd_tx_with_ref(self, fildes, true, error);
    if (picotm_error_is_set(error)) {
        goto err_add_new_fd_tx_with_ref;
    }

    int cookie = openoptab_append(&self->openoptab,
                                  &self->openoptablen, DO_UNLINK(mode),
                                  error);
    if (picotm_error_is_set(error)) {
        goto err_openoptab_append;
    }

    /* Inject event */
    if (cookie >= 0) {
        fildes_log_append(self->log, FILDES_OP_OPEN, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            goto err_fildes_log_append;
        }
    }

    free(abs_path);

    return fildes;

err_fildes_log_append:
err_openoptab_append:
err_add_new_fd_tx_with_ref:
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

    add_new_fd_tx_with_ref(self, pipefd[0], true, error);
    if (picotm_error_is_set(error)) {
        if (TEMP_FAILURE_RETRY(close(pipefd[0])) < 0) {
            perror("close");
        }
        if (TEMP_FAILURE_RETRY(close(pipefd[1])) < 0) {
            perror("close");
        }
        return -1;
    }

    add_new_fd_tx_with_ref(self, pipefd[1], true, error);
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
        fildes_log_append(self->log, FILDES_OP_PIPE, 0, cookie, error);
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

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* pread */

    int cookie = -1;

    ssize_t len = fd_tx_pread_exec(fd_tx, fildes, buf, nbyte, off,
                                   isnoundo, &cookie, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* inject event */
    if (cookie >= 0) {
        fildes_log_append(self->log, FILDES_OP_PREAD, fildes, cookie, error);
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

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, error);
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
        fildes_log_append(self->log, FILDES_OP_PWRITE, fildes, cookie, error);
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

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Read */

    int cookie = -1;

    ssize_t len = fd_tx_read_exec(fd_tx, fildes, buf, nbyte,
                                  isnoundo, &cookie, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        fildes_log_append(self->log, FILDES_OP_READ, fildes, cookie, error);
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

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, sockfd, error);
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
        fildes_log_append(self->log, FILDES_OP_RECV, sockfd, cookie, error);
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

            get_fd_tx_with_ref(self, fildes, error);
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

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, sockfd, error);
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
        fildes_log_append(self->log, FILDES_OP_SEND, sockfd, cookie, error);
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

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, sockfd, error);
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
        fildes_log_append(self->log, FILDES_OP_SHUTDOWN, sockfd, cookie, error);
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

    add_new_fd_tx_with_ref(self, sockfd, true, error);
    if (picotm_error_is_set(error)) {
        if (TEMP_FAILURE_RETRY(close(sockfd)) < 0) {
            perror("close");
        }
        return -1;
    }

    /* Inject event */
    fildes_log_append(self->log, FILDES_OP_SOCKET, sockfd, -1, error);
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

    allocator_module_free(real_path, strlen(real_path), error);
    if (picotm_error_is_set(error)) {
        goto err_allocator_module_free;
    }

    return res;

err_allocator_module_free:
    picotm_error_mark_as_non_recoverable(error);
    return -1;
err_stat:
    {
        struct picotm_error tmp_error = PICOTM_ERROR_INITIALIZER;
        allocator_module_free(real_path, strlen(real_path), &tmp_error);
        if (picotm_error_is_set(&tmp_error)) {
            picotm_error_mark_as_non_recoverable(error);
        }
    }
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
    fildes_log_append(self->log, FILDES_OP_SYNC, -1, -1, error);
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

    allocator_module_free(real_path, strlen(real_path), error);
    if (picotm_error_is_set(error)) {
        picotm_error_mark_as_non_recoverable(error);
        goto err;
    }

    return res;

err_unlink:
    {
        struct picotm_error tmp_error = PICOTM_ERROR_INITIALIZER;
        allocator_module_free(real_path, strlen(real_path), &tmp_error);
        if (picotm_error_is_set(&tmp_error)) {
            picotm_error_mark_as_non_recoverable(error);
        }
    }
err:
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

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, error);
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
        fildes_log_append(self->log, FILDES_OP_WRITE, fildes, cookie, error);
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

static size_t
prepare_commit_fd_tx(struct fd_tx* fd_tx, struct picotm_error* error)
{
    fd_tx_prepare_commit(fd_tx, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static size_t
prepare_commit_fd_tx_cb(struct picotm_slist* item, void* data)
{
    return prepare_commit_fd_tx(fd_tx_of_slist(item), data);
}

void
fildes_tx_prepare_commit(struct fildes_tx* self, int noundo,
                         struct picotm_error* error)
{
    /* Validate file-descriptor state */
    picotm_slist_walk_1(&self->fd_tx_active_list, prepare_commit_fd_tx_cb,
                        error);
}

void
fildes_tx_apply_event(struct fildes_tx* self, enum fildes_op op, int fildes,
                      int cookie, struct picotm_error* error)
{
    static void (* const apply[LAST_FILDES_OP])(struct fildes_tx*,
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

    apply[op](self, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fildes_tx_undo_event(struct fildes_tx* self, enum fildes_op op, int fildes,
                     int cookie, struct picotm_error* error)
{
    static void (* const undo[LAST_FILDES_OP])(struct fildes_tx*,
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

    undo[op](self, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
finish_file_tx(struct file_tx* file_tx, struct fildes_tx* fildes_tx)
{
    file_tx_finish(file_tx);
    file_tx_unref(file_tx);

    picotm_slist_enqueue_front(&fildes_tx->file_tx_alloced_list,
                               &file_tx->list_entry);
}

static void
finish_file_tx_cb(struct picotm_slist* item, void* data)
{
    finish_file_tx(file_tx_of_list_entry(item), data);
}

static void
finish_ofd_tx(struct ofd_tx* ofd_tx)
{
    ofd_tx_finish(ofd_tx);
    ofd_tx_unref(ofd_tx);
}

static void
finish_ofd_tx_cb(struct picotm_slist* item)
{
    finish_ofd_tx(ofd_tx_of_slist(item));
}

static void
finish_fd_tx(struct fd_tx* fd_tx)
{
    fd_tx_finish(fd_tx);
    fd_tx_unref(fd_tx);
}

static void
finish_fd_tx_cb(struct picotm_slist* item)
{
    finish_fd_tx(fd_tx_of_slist(item));
}

void
fildes_tx_finish(struct fildes_tx* self, struct picotm_error* error)
{
    /* Unref files */
    picotm_slist_cleanup_1(&self->file_tx_active_list,
                           finish_file_tx_cb, self);

    /* Unref open file descriptions */
    picotm_slist_cleanup_0(&self->ofd_tx_active_list, finish_ofd_tx_cb);

    /* Unref file descriptors */
    picotm_slist_cleanup_0(&self->fd_tx_active_list, finish_fd_tx_cb);

    /* Clear concurrency control on file-descriptor table */
    fdtab_tx_finish(&self->fdtab_tx);
}
