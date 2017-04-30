/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct event;
struct picotm_error;
struct module;

/**
 * \brief The log holds the events of a transaction.
 */
struct log {

    /** Table of transaction's events. */
    struct event* eventtab;

    /** Number of events in the log. */
    size_t eventtablen;

    /** Maximum number of events. */
    size_t eventtabsiz;
};

/**
 * Init log.
 */
int
log_init(struct log* self);

/**
 * Uninit log.
 */
void
log_uninit(struct log* self);

/**
 * Inject an event into log.
 */
int
log_inject_event(struct log* self, unsigned long module, unsigned long call,
                 uintptr_t cookie);

/**
 * Apply events in the log.
 *
 * \param self          The log.
 * \param module        The table of registered modules.
 * \param noundo        True if the transaction is irrevokable, or false otherwise.
 * \param[out] error    An error returned by an apply operation.
 * \returns             0 on success, or a negative value otherwise.
 */
int
log_apply_events(struct log* self, const struct module* module, bool noundo,
                 struct picotm_error* error);

/**
 * Undo events in the log.
 *
 * \param self          The log.
 * \param module        The table of registered modules.
 * \param noundo        True if the transaction is irrevokable, or false otherwise.
 * \param[out] error    An error returned by an undo operation.
 * \returns             0 on success, or a negative otherwise.
 */
int
log_undo_events(struct log* self, const struct module* module, bool noundo,
                struct picotm_error* error);
