/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

struct module;
struct picotm_error;
struct picotm_event;

/**
 * \brief The log holds the events of a transaction.
 */
struct log {

    /** Table of transaction's events. */
    struct picotm_event* eventtab;

    /** Number of events in the log. */
    size_t eventtablen;

    /** Maximum number of events. */
    size_t eventtabsiz;
};

/**
 * Init log.
 */
void
log_init(struct log* self);

/**
 * Uninit log.
 */
void
log_uninit(struct log* self);

/**
 * Appends an event to the transaction's event log.
 */
void
log_append_event(struct log* self, unsigned long module, unsigned long call,
                 uintptr_t cookie, struct picotm_error* error);

/**
 * Apply events in the log.
 *
 * \param self          The log.
 * \param module        The table of registered modules.
 * \param noundo        True if the transaction is irrevokable, or false otherwise.
 * \param[out] error    An error returned by an apply operation.
 */
void
log_apply_events(struct log* self, const struct module* module, bool noundo,
                 struct picotm_error* error);

/**
 * Undo events in the log.
 *
 * \param self          The log.
 * \param module        The table of registered modules.
 * \param noundo        True if the transaction is irrevokable, or false otherwise.
 * \param[out] error    An error returned by an undo operation.
 */
void
log_undo_events(struct log* self, const struct module* module, bool noundo,
                struct picotm_error* error);
