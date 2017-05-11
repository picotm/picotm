/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stddef.h>

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
 * fchdir()
 */

int
vfs_tx_exec_fchdir(struct vfs_tx* self, int fildes,
                   struct picotm_error* error);

/*
 * mkstemp()
 */

int
vfs_tx_exec_mkstemp(struct vfs_tx* self, char* pathname,
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
                   const struct picotm_event* event, size_t n,
                   struct picotm_error* error);

void
vfs_tx_undo_event(struct vfs_tx* self,
                  const struct picotm_event* event, size_t n,
                  struct picotm_error* error);

void
vfs_tx_finish(struct vfs_tx* self, struct picotm_error* error);
