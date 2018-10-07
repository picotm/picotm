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
