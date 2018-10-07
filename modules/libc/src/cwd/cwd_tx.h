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

#pragma once

#include "picotm/picotm-lib-rwstate.h"
#include <stddef.h>
#include <stdint.h>
#include "cwd.h"
#include "cwd_event.h"

/**
 * \cond impl || libc_impl || libc_impl_cwd
 * \ingroup libc_impl
 * \ingroup libc_impl_cwd
 * \file
 * \endcond
 */

struct cwd_log;
struct picotm_error;

struct cwd_tx {

    struct cwd_log* log;

    struct cwd* cwd;

    struct picotm_rwstate   rwstate[NUMBER_OF_CWD_FIELDS];
};

void
cwd_tx_init(struct cwd_tx* self, struct cwd_log* log, struct cwd* cwd);

void
cwd_tx_uninit(struct cwd_tx* self);

/**
 * Tries to acquire a reader lock on the current working directory.
 *
 * \param       self    The CWD structure.
 * \param       field   The reader lock's field.
 * \param[out]  error   Returns an error.
 */
void
cwd_tx_try_rdlock_field(struct cwd_tx* self, enum cwd_field field,
                        struct picotm_error* error);

/**
 * Tries to acquire a writer lock on the current working directory.
 *
 * \param       self    The CWD structure.
 * \param       field   The writer lock's field.
 * \param[out]  error   Returns an error.
 */
void
cwd_tx_try_wrlock_field(struct cwd_tx* self, enum cwd_field field,
                        struct picotm_error* error);

/*
 * chdir()
 */

int
cwd_tx_chdir_exec(struct cwd_tx* self, const char* path,
                  struct picotm_error* error);

/*
 * getcwd()
 */

char*
cwd_tx_getcwd_exec(struct cwd_tx* self, char* buf, size_t size,
                   struct picotm_error* error);

/*
 * realpath()
 */

char*
cwd_tx_realpath_exec(struct cwd_tx* self, const char* path,
                     char* resolved_path, struct picotm_error* error);

/*
 * Module interface
 */

void
cwd_tx_apply_event(struct cwd_tx* self, enum cwd_op op, char* alloced,
                   struct picotm_error* error);

void
cwd_tx_undo_event(struct cwd_tx* self, enum cwd_op op, char* alloced,
                  struct picotm_error* error);

void
cwd_tx_finish(struct cwd_tx* self);
