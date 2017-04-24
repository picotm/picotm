/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOG_H
#define LOG_H

#include <stddef.h>
#include <stdint.h>

struct module;

/**
 * \brief The log is the core part of the framework. It holds the events of a
 *        transaction.
 */
struct log {
    struct event *eventtab; /** \brief Table of transction's events */
    size_t        eventtablen; /** \brief Number of events */
    size_t        eventtabsiz; /** \brief Maximum number of events */
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
 * \brief Inject an event into log
 */
int
log_inject_event(struct log *log, unsigned long module, unsigned long call,
                 uintptr_t cookie);

/**
 * \brief Apply events in the log
 */
int
log_apply_events(struct log *log, struct module* module, int noundo);

/**
 * \brief Undo events in the log
 */
int
log_undo_events(struct log *log, struct module* module, int noundo);

#endif
