/* Copyright (C) 2009  Thomas Zimmermann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef COMFS_H
#define COMFS_H

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
    struct com_fs_event *eventtab; /**< \brief Event table */
    size_t               eventtablen; /**< \brief Length of event table */

    int inicwd; /**< \brief File descriptor of initial working dir */
    int newcwd; /**< \brief File descriptor of current working dir */
};

int
com_fs_init(struct com_fs *data);

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

