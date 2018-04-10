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

#include "picotm/picotm.h" /* for __picotm_jmp_buf */
#include <stdbool.h>
#include "picotm_module.h"
#include "picotm_lock_owner.h"
#include "picotm_log.h"

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

#define MAX_NMODULES    (256)

struct picotm_error;
struct picotm_lock_manager;

enum picotm_tx_mode {
    TX_MODE_REVOCABLE,
    TX_MODE_IRREVOCABLE
};

struct picotm_tx {
    __picotm_jmp_buf*   env;
    struct picotm_log   log;
    enum picotm_tx_mode mode;
    unsigned long       nretries;

    /** The global lock manager for all transactions. */
    struct picotm_lock_manager* lm;

    /** The transaction's lock-owner instance. */
    struct picotm_lock_owner lo;

    unsigned long nmodules; /**< \brief Number allocated modules */
    struct picotm_module module[MAX_NMODULES]; /** \brief Registered modules */
};

void
picotm_tx_init(struct picotm_tx* self, struct picotm_lock_manager* lm,
               struct picotm_error* error);

void
picotm_tx_release(struct picotm_tx* self);

bool
picotm_tx_is_irrevocable(const struct picotm_tx* self);

unsigned long
picotm_tx_register_module(struct picotm_tx* self,
                          const struct picotm_module_ops* ops, void* data,
                          struct picotm_error* error);

void
picotm_tx_append_event(struct picotm_tx* self, unsigned long module,
                       uint16_t head, uintptr_t tail,
                       struct picotm_error* error);

void
picotm_tx_begin(struct picotm_tx* self, enum picotm_tx_mode mode,
                bool is_retry, __picotm_jmp_buf* env,
                struct picotm_error* error);

void
picotm_tx_commit(struct picotm_tx* self, struct picotm_error* error);

void
picotm_tx_rollback(struct picotm_tx* self, struct picotm_error* error);

bool
picotm_tx_is_valid(struct picotm_tx* self);
