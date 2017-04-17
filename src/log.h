/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOG_H
#define LOG_H

#include <tanger-stm-ext-actions.h>
#include "component.h"

#define MAX_NCOMPONENTS (256)

/**
 * \brief The log is the core part of the framework. It holds the events of a
 *        transaction.
 */
struct log {
    struct event *eventtab; /** \brief Table of transction's events */
    size_t        eventtablen; /** \brief Number of events */
    size_t        eventtabsiz; /** \brief Maximum number of events */

    unsigned long    nmodules; /**< \brief Number allocated modules */
    struct component com[MAX_NCOMPONENTS]; /** \brief Registered components */
};

/**
 * \brief Init log
 */
int
log_init(struct log *log);

/**
 * \brief Uninit log
 */
int
log_uninit(struct log *log);

/**
 * \brief Allocate a new module
 */
long
log_alloc_module(struct log* log);

/**
 * \brief Retrieve component structure by 'name enumerator'
 */
struct component *
log_get_component_by_name(struct log *log, enum component_name component);

/**
 * \brief Inject an event into log
 */
int
log_inject_event(struct log *log, unsigned long component, unsigned long call,
                 uintptr_t cookie);

/**
 * \brief Clear log
 */
void
log_clear_events(struct log *log);

/**
 * \brief Lock transaction's domains
 */
int
log_lock(struct log *log);

/**
 * \brief Unlock transaction's domains
 */
int
log_unlock(struct log *log);

/**
 * \brief Validate transaction's domains
 */
int
log_validate(struct log *log, int eotx, int noundo);

/**
 * \brief Apply events in the log
 */
int
log_apply_events(struct log *log, int noundo);

/**
 * \brief Undo events in the log
 */
int
log_undo_events(struct log *log, int noundo);

/**
 * \brief Update CC data of transaction's domains; called at the end of
 *        commit
 */
int
log_updatecc(struct log *log, int noundo);

/**
 * \brief Clear CC data of transaction's domains; called during abort
 */
int
log_clearcc(struct log *log, int noundo);

/**
 * \brief Cleanup after transaction
 */
int
log_finish(struct log *log);

#endif

