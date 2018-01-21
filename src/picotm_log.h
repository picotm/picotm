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

#include <stddef.h>

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

struct picotm_error;
struct picotm_event;

/**
 * \brief The log holds the events of a transaction.
 */
struct picotm_log {

    /** Table of transaction events. */
    struct picotm_event* eventtab;

    /** Valid data bytes in the transaction log. */
    size_t eventtablen;

    /** Allocation size of the transaction log. */
    size_t eventtabsiz;
};

/**
 * Init log.
 */
void
picotm_log_init(struct picotm_log* self);

/**
 * Uninit log.
 */
void
picotm_log_uninit(struct picotm_log* self);

/**
 * \brief Returns the first event in an event log.
 * \param   self    The event log.
 * \returns The event log's first event.
 */
struct picotm_event*
picotm_log_begin(struct picotm_log* self);

/**
 * \brief Returns the terminal event at the end of an event log.
 * \param   self    The event log.
 * \returns The event log's terminal event.
 */
const struct picotm_event*
picotm_log_end(struct picotm_log* self);

/**
 * \brief Appends an event to an event log.
 * \param       self    The event log.
 * \param       event   The event to append to the log.
 * \param[out]  error   Returns an error to the caller.
 */
void
picotm_log_append(struct picotm_log* self, const struct picotm_event* event,
           struct picotm_error* error);

/**
 * \brief Removes all events from an event log.
 * \param   self    The event log.
 */
void
picotm_log_clear(struct picotm_log* self);
