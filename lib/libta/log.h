/* Copyright (C) 2009  Thomas Zimmermann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef LOG_H
#define LOG_H

/**
 * \brief The log is the core part of the framework. It holds the events of a
 *        transaction.
 */
struct log
{
    stm_error_handler *errhdlrtab; /** \brief Error-handler stack */
    size_t             errhdlrtablen; /** \brief Depth of error-handler stack */

    struct event *eventtab; /** \brief Table of transction's events */
    size_t        eventtablen; /** \brief Number of events */
    size_t        eventtabsiz; /** \brief Maximum number of events */

    struct component com[LAST_COMPONENT]; /** \brief Registered components */
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
 * \brief Retrieve component structure by 'name enumerator'
 */
struct component *
log_get_component_by_name(struct log *log, enum component_name component);

/**
 * \brief Inject an event into log
 */
int
log_inject_event(struct log *log, enum component_name component, unsigned short call,
                                                                 unsigned short cookie);

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

