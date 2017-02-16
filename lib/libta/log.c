/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <tanger-stm-ext-actions.h>
#include "table.h"
#include "component.h"
#include "log.h"

int
log_init(struct log *log)
{
    int res;
    struct component *com;

    assert(log);

    log->errhdlrtab = NULL;
    log->errhdlrtablen = 0;
    log->eventtab = NULL;
    log->eventtablen = 0;
    log->eventtabsiz = 0;

    res = 0;

    for (com = log->com;
        (com < log->com+sizeof(log->com)/sizeof(log->com[0])) && (res >= 0); ++com) {
        res = component_init(com, NULL, NULL, NULL, NULL,
                                  NULL, NULL, NULL, NULL,
                                  NULL, NULL, NULL, NULL, NULL, NULL);
    }

    return res;
}

int
log_uninit(struct log *log)
{
    struct component *com;

    assert(log);

    for (com = log->com;
         com < log->com+sizeof(log->com)/sizeof(log->com[0]); ++com) {
        component_uninit(com);
    }

    free(log->errhdlrtab);
    free(log->eventtab);

    return 0;
}

struct component *
log_get_component_by_name(struct log *log, enum component_name component)
{
    assert(log);
    assert(component >= 0);
    assert(component < sizeof(log->com)/sizeof(log->com[0]));

    return log->com+component;
}

int
log_inject_event(struct log *log, enum component_name component, unsigned short call,
                                                                 unsigned short cookie)
{
    assert(log);

    if (log->eventtablen >= log->eventtabsiz) {

        void *tmp = tabresize(log->eventtab,
                              log->eventtabsiz,
                              log->eventtabsiz+1, sizeof(log->eventtab[0]));
        if (!tmp) {
            return -1;
        }
        log->eventtab = tmp;

        ++log->eventtabsiz;
    }

    struct event *event = log->eventtab+log->eventtablen;

    event->cookie = cookie;
    event->component = component;
    event->call = call;

    return log->eventtablen++;
}

void
log_clear_events(struct log *log)
{
    assert(log);

    log->eventtablen = 0;
}

/* Lock
 */

static int
log_lock_cb_walk(void *com)
{
    int res = component_lock(com);

    return res < 0 ? res : 1;
}

int
log_lock(struct log *log)
{
    int res;

    assert(log);


    res = tabwalk_1(log->com,
                    sizeof(log->com)/sizeof(log->com[0]),
                    sizeof(log->com[0]),
                    log_lock_cb_walk);

    return res;
}

/* Unlock
 */

static int
log_unlock_cb_walk(void *com)
{
    int res = component_unlock(com);

    return res < 0 ? res : 1;
}

int
log_unlock(struct log *log)
{
    int res;

    assert(log);


    res = tabrwalk_1(log->com,
                     sizeof(log->com)/sizeof(log->com[0]),
                     sizeof(log->com[0]),
                     log_unlock_cb_walk);

    return res;
}

/* Validate
 */

static int
log_tpc_request_cb_walk(void *com, void *noundo)
{
    int res = component_tpc_request(com, (int)noundo);
    return res < 0 ? res : 1;
}

static int
log_validate_cb_walk(void *com, void *noundo)
{
    int res = component_validate(com, (int)noundo);
    return res < 0 ? res : 1;
}

static int
log_tpc_success_cb_walk(void *com, void *noundo)
{
    int res = component_tpc_success(com, (int)noundo);
    return res < 0 ? res : 1;
}

static int
log_tpc_noundo_cb_walk(void *com, void *noundo)
{
    int res = component_tpc_noundo(com, (int)noundo);
    return res < 0 ? res : 1;
}

int
log_validate(struct log *log, int eotx, int noundo)
{
    int res;

    assert(log);

    if (eotx) {
        /* Send commit request to peer */
        res = tabwalk_2(log->com,
                        sizeof(log->com)/sizeof(log->com[0]),
                        sizeof(log->com[0]),
                        log_tpc_request_cb_walk,
                        (void*)noundo);

        if (res < 0) {
            return -1;
        }
    }

    /* Validate local external domains */

    struct component *com;
    int err = 0;

    for (com = log->com;
        (com < log->com+sizeof(log->com)/sizeof(log->com[0])) && !err; ++com) {

        err = component_validate(com, noundo);
    }

    if (err < 0) {
        return -1;
    }

    if (eotx) {
        /* Send commit success to peer; aborts are signalled while the
         * transaction is being aborted.
         */
        res = tabwalk_2(log->com,
                        sizeof(log->com)/sizeof(log->com[0]),
                        sizeof(log->com[0]),
                        log_tpc_success_cb_walk,
                        (void*)noundo);
    } else {
        /* Send noundo to peer.
         */
        res = tabwalk_2(log->com,
                        sizeof(log->com)/sizeof(log->com[0]),
                        sizeof(log->com[0]),
                        log_tpc_noundo_cb_walk,
                        (void*)noundo);
    }

    return res;
}

/* Apply
 */

int
log_apply_events(struct log *log, int noundo)
{
    assert(log);

    /* Apply events in chronological order */

    const struct event *event = log->eventtab;

    while (event < log->eventtab+log->eventtablen) {

        /* Find consectuive events from same component */

        const struct event *event2 = event+1;

        while ((event2 < log->eventtab+log->eventtablen)
                && (event2->component == event->component)) {
            ++event2;
        }

        /* Apply vector of events from component */

        if (component_apply_events(log->com+event->component, event, event2-event) < 0) {

            const enum stm_error_action ea =
                log->errhdlrtablen
                    ? log->errhdlrtab[log->errhdlrtablen-1](event->component,
                                                            event->call,
                                                            event->cookie)
                    : STM_ERR_EXIT;

            switch (ea) {
                case STM_ERR_AGAIN:
                    --event;
                    /* Fall through */
                case STM_ERR_IGNORE:
                    break;
                case STM_ERR_EXIT:
                    /* Fall through */
                default:
                    abort();
            }
        }

        event = event2;
    }

    /* FIXME: Cleanup should be done in finish */
    log->eventtablen = 0;

    return 0;
}

/* Undo
 */

static int
log_tpc_failure_cb_walk(void *com, void *noundo)
{
    int res = component_tpc_failure(com, (int)noundo);
    return res < 0 ? res : 1;
}

int
log_undo_events(struct log *log, int noundo)
{
    assert(log);


    /* Signal abort to peer */

    int res = tabwalk_2(log->com,
                        sizeof(log->com)/sizeof(log->com[0]),
                        sizeof(log->com[0]),
                        log_tpc_failure_cb_walk,
                        (void*)noundo);

    if (res < 0) {
        return -1;
    }

    /* Undo events in reversed-chronological order */

    const struct event *event = log->eventtab+log->eventtablen;

    res = 0;

    while ((event > log->eventtab) && (res >= 0)) {
        --event;
        res = component_undo_events(log->com+event->component, event, 1);
    }

    /* FIXME: Cleanup should be done in finish */
    log->eventtablen = 0;

    return res;
}

/* CC
 */

int
log_updatecc(struct log *log, int noundo)
{
    assert(log);

    /* Update external domains' CC */

    struct component *com;
    int err = 0;

    for (com = log->com;
        (com < log->com+sizeof(log->com)/sizeof(log->com[0])) && !err; ++com) {

        err = component_updatecc(com, noundo);
    }

    return err;
}

int
log_clearcc(struct log *log, int noundo)
{
    assert(log);

    /* Clear external domains' CC */

    struct component *com;
    int err = 0;

    for (com = log->com;
        (com < log->com+sizeof(log->com)/sizeof(log->com[0])) && !err; ++com) {

        err = component_clearcc(com, noundo);
    }

    return err;
}

/* Finish
 */

static int
log_finish_cb_walk(void *com)
{
    return !component_finish(com) ? 1 : -1;
}

int
log_finish(struct log *log)
{
    assert(log);

    int res = tabwalk_1(log->com,
                        sizeof(log->com)/sizeof(log->com[0]),
                        sizeof(log->com[0]),
                        log_finish_cb_walk);

    if (res) {
        return res;
    }

    return 0;
}

#include <stdio.h>

int
event_dump(const struct event *ev)
{
    static const char * const sysname[] = {
        "COMPONENT_ERROR",
        "COMPONENT_ALLOC",
        "COMPONENT_FD",
        "COMPONENT_STREAM",
        "COMPONENT_FS",
        "(unused)",
        "(unused)",
        "(unused)",
        "COMPONENT_USER0",
        "COMPONENT_USER1",
        "COMPONENT_USER2",
        "COMPONENT_USER3"};

    assert(ev);

    fprintf(stderr, "%s %d %d", sysname[ev->component], (int)ev->call,
                                                        (int)ev->cookie);

    return 0;
}

static size_t
log_table_dump(const struct event *eventtab, size_t eventtablen)
{
    const struct event *beg = eventtab;
    const struct event *end = eventtab+eventtablen;

    for (; beg < end; ++beg) {
        fprintf(stderr, "%d: ", beg-eventtab);
        event_dump(beg);
        fprintf(stderr, "\n");
    }

    return 0;
}

