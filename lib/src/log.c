/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <picotm/picotm-module.h>
#include "module.h"
#include "table.h"
#include "log.h"

int
log_init(struct log *log)
{
    assert(log);

    log->eventtab = NULL;
    log->eventtablen = 0;
    log->eventtabsiz = 0;

    return 0;
}

int
log_uninit(struct log *log)
{
    assert(log);

    return 0;
}

int
log_inject_event(struct log *log, unsigned long module, unsigned long call,
                 uintptr_t cookie)
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
    event->module = module;
    event->call = call;

    return log->eventtablen++;
}

void
log_clear_events(struct log *log)
{
    assert(log);

    log->eventtablen = 0;
}

int
log_apply_events(struct log *log, struct module* module, int noundo)
{
    assert(log);

    /* Apply events in chronological order */

    const struct event* event = log->eventtab;
    const struct event* event_end = log->eventtab + log->eventtablen;

    while (event < event_end) {

        /* Find consecutive events from same module */

        const struct event* event2 = event + 1;

        while ((event2 < event_end) && (event->module == event2->module)) {
            ++event2;
        }

        /* Apply vector of events from module */

        ptrdiff_t nevents = event2 - event;

        int res = module_apply_events(module + event->module, event, nevents);
        if (res < 0) {
            return -1;
        }

        event = event2;
    }

    /* FIXME: Cleanup should be done in finish */
    log->eventtablen = 0;

    return 0;
}

int
log_undo_events(struct log *log, struct module* module, int noundo)
{
    assert(log);

    /* Undo events in reversed-chronological order */

    const struct event* event = log->eventtab + log->eventtablen;
    const struct event* event_end = log->eventtab;

    while (event > event_end) {
        --event;
        int res = module_undo_events(module + event->module, event, 1);
        if (res < 0) {
            break;
        }
    }

    /* FIXME: Cleanup should be done in finish */
    log->eventtablen = 0;

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

    fprintf(stderr, "%s %d %d", sysname[ev->module], (int)ev->call,
                                                     (int)ev->cookie);

    return 0;
}

#if 0
static size_t
log_table_dump(const struct event *eventtab, size_t eventtablen)
{
    const struct event *beg = eventtab;
    const struct event *end = eventtab+eventtablen;

    for (; beg < end; ++beg) {
        fprintf(stderr, "%zu: ", beg-eventtab);
        event_dump(beg);
        fprintf(stderr, "\n");
    }

    return 0;
}
#endif
