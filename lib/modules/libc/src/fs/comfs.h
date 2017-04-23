/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMFS_H
#define COMFS_H

#include <picotm/picotm-module.h>

enum com_fs_action
{
    ACTION_FCHDIR = 0,
    ACTION_MKSTEMP
};

struct com_fs_event
{
    int cookie;
};

struct com_fs
{
    unsigned long module;

    struct com_fs_event *eventtab; /**< \brief Event table */
    size_t               eventtablen; /**< \brief Length of event table */

    int inicwd; /**< \brief File descriptor of initial working dir */
    int newcwd; /**< \brief File descriptor of current working dir */
};

int
com_fs_init(struct com_fs *data, unsigned long module);

void
com_fs_uninit(struct com_fs *data);

int
com_fs_inject(struct com_fs *data, enum com_fs_action action, int cookie);

int
com_fs_get_cwd(struct com_fs *data);

char *
com_fs_get_cwd_path(struct com_fs *data);

char *
com_fs_absolute_path(struct com_fs *data, const char *path);

char *
com_fs_canonical_path(struct com_fs *data, const char *path);

int
com_fs_lock(struct com_fs *data);

void
com_fs_unlock(struct com_fs *data);

int
com_fs_validate(struct com_fs *data);

int
com_fs_apply_event(struct com_fs *data, const struct event *event, size_t n);

int
com_fs_undo_event(struct com_fs *data, const struct event *event, size_t n);

int
com_fs_finish(struct com_fs *data);

#endif

