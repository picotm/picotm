/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "log.h"
#include <stdlib.h>
#include <picotm/picotm-module.h>
#include "module.h"
#include "table.h"

int
log_init(struct log* self)
{
    self->eventtab = NULL;
    self->eventtablen = 0;
    self->eventtabsiz = 0;

    return 0;
}

void
log_uninit(struct log* self)
{
    free(self->eventtab);
}

int
log_inject_event(struct log* self, unsigned long module, unsigned long call,
                 uintptr_t cookie)
{
    if (self->eventtablen >= self->eventtabsiz) {

        size_t eventtabsiz = self->eventtabsiz + 1;

        void* tmp = tabresize(self->eventtab,
                              self->eventtabsiz, eventtabsiz,
                              sizeof(self->eventtab[0]));
        if (!tmp) {
            return -1;
        }
        self->eventtab = tmp;
        self->eventtabsiz = eventtabsiz;
    }

    struct event* event = self->eventtab + self->eventtablen;

    event->cookie = cookie;
    event->module = module;
    event->call = call;

    return self->eventtablen++;
}

int
log_apply_events(struct log* self, const struct module* module, bool noundo)
{
    /* Apply events in chronological order */

    const struct event* event = self->eventtab;
    const struct event* event_end = self->eventtab + self->eventtablen;

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

    self->eventtablen = 0;

    return 0;
}

int
log_undo_events(struct log* self, const struct module* module, bool noundo)
{
    /* Undo events in reversed-chronological order */

    const struct event* event = self->eventtab + self->eventtablen;
    const struct event* event_end = self->eventtab;

    while (event > event_end) {
        --event;
        int res = module_undo_events(module + event->module, event, 1);
        if (res < 0) {
            break;
        }
    }

    self->eventtablen = 0;

    return 0;
}
