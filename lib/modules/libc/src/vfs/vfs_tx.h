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

#pragma once

#include <stddef.h>
#include <sys/stat.h>

/**
 * \cond impl || libc_impl || libc_impl_vfs
 * \ingroup libc_impl
 * \ingroup libc_impl_vfs
 * \file
 * \endcond
 */

struct picotm_error;
struct picotm_event;

struct vfs_tx_event {
    int cookie;
};

/**
 * A file-system transaction.
 */
struct vfs_tx {
    unsigned long module;

    /** Event table */
    struct vfs_tx_event* eventtab;

    /** Length of event table */
    size_t               eventtablen;

    /** File descriptor of initial working dir */
    int inicwd;
    /** File descriptor of current working dir */
    int newcwd;
};

void
vfs_tx_init(struct vfs_tx* self, unsigned long module);

void
vfs_tx_uninit(struct vfs_tx* self);

int
vfs_tx_get_cwd(struct vfs_tx* self, struct picotm_error* error);

char*
vfs_tx_get_cwd_path(struct vfs_tx* self, struct picotm_error* error);

char*
vfs_tx_absolute_path(struct vfs_tx* self, const char* path,
                     struct picotm_error* error);

char*
vfs_tx_canonical_path(struct vfs_tx* self, const char* path,
                      struct picotm_error* error);

/*
 * chmod()
 */

int
vfs_tx_exec_chmod(struct vfs_tx* self, const char* path, mode_t mode,
                  struct picotm_error* error);

/*
 * fchdir()
 */

int
vfs_tx_exec_fchdir(struct vfs_tx* self, int fildes,
                   struct picotm_error* error);

/*
 * fchmod()
 */

int
vfs_tx_exec_fchmod(struct vfs_tx* self, int fildes, mode_t mode,
                   struct picotm_error* error);

/*
 * fstat()
 */

int
vfs_tx_exec_fstat(struct vfs_tx* self, int fildes, struct stat* buf,
                  struct picotm_error* error);

/*
 * getcwd()
 */

char*
vfs_tx_exec_getcwd(struct vfs_tx* self, char* buf, size_t size,
                   struct picotm_error* error);

/*
 * link()
 */

int
vfs_tx_exec_link(struct vfs_tx* self, const char* path1, const char* path2,
                 struct picotm_error* error);

/*
 * lstat()
 */

int
vfs_tx_exec_lstat(struct vfs_tx* self, const char* path, struct stat* buf,
                  struct picotm_error* error);

/*
 * mkdir()
 */

int
vfs_tx_exec_mkdir(struct vfs_tx* self, const char* path, mode_t mode,
                  struct picotm_error* error);

/*
 * mkfifo()
 */

int
vfs_tx_exec_mkfifo(struct vfs_tx* self, const char* path, mode_t mode,
                   struct picotm_error* error);

/*
 * mknod()
 */

int
vfs_tx_exec_mknod(struct vfs_tx* self, const char* path, mode_t mode,
                  dev_t dev, struct picotm_error* error);

/*
 * mkstemp()
 */

int
vfs_tx_exec_mkstemp(struct vfs_tx* self, char* pathname,
                    struct picotm_error* error);

/*
 * stat()
 */

int
vfs_tx_exec_stat(struct vfs_tx* self, const char* path, struct stat* buf,
                 struct picotm_error* error);

/*
 * unlink()
 */

int
vfs_tx_exec_unlink(struct vfs_tx* self, const char* path,
                   struct picotm_error* error);

/*
 * Module interface
 */

void
vfs_tx_lock(struct vfs_tx* self, struct picotm_error* error);

void
vfs_tx_unlock(struct vfs_tx* self);

void
vfs_tx_validate(struct vfs_tx* self, struct picotm_error* error);

void
vfs_tx_apply_event(struct vfs_tx* self,
                   const struct picotm_event* event,
                   struct picotm_error* error);

void
vfs_tx_undo_event(struct vfs_tx* self,
                  const struct picotm_event* event,
                  struct picotm_error* error);

void
vfs_tx_finish(struct vfs_tx* self, struct picotm_error* error);
