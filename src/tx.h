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

#include <setjmp.h>
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
struct tx_shared;

enum tx_mode {
    TX_MODE_REVOCABLE,
    TX_MODE_IRREVOCABLE
};

struct tx {
    jmp_buf*          env;
    struct picotm_log log;
    struct tx_shared *shared;
    enum tx_mode      mode;
    unsigned long     nretries;

    /** The transaction's lock-owner instance. */
    struct picotm_lock_owner lo;

    unsigned long nmodules; /**< \brief Number allocated modules */
    struct picotm_module module[MAX_NMODULES]; /** \brief Registered modules */

    bool is_initialized;
};

void
tx_init(struct tx* self, struct tx_shared* tx_shared,
        struct picotm_error* error);

void
tx_release(struct tx* self);

bool
tx_is_irrevocable(const struct tx* self);

unsigned long
tx_register_module(struct tx* self,
                   const struct picotm_module_ops* ops, void* data,
                   struct picotm_error* error);

void
tx_append_event(struct tx* self, unsigned long module, uint16_t head,
                uintptr_t tail, struct picotm_error* error);

void
tx_begin(struct tx* self, enum tx_mode mode, bool is_retry, jmp_buf* env,
         struct picotm_error* error);

void
tx_commit(struct tx* self, struct picotm_error* error);

void
tx_rollback(struct tx* self, struct picotm_error* error);

bool
tx_is_valid(struct tx* self);
