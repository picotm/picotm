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

#pragma once

#include <picotm/picotm-lib-rwstate.h>
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
cwd_tx_update_cc(struct cwd_tx* self, struct picotm_error* error);

void
cwd_tx_clear_cc(struct cwd_tx* self, struct picotm_error* error);

void
cwd_tx_finish(struct cwd_tx* self);
