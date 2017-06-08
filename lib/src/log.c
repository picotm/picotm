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

void
log_append_event(struct log* self, unsigned long module, unsigned long call,
                 uintptr_t cookie, struct picotm_error* error)
{
    if (self->eventtablen >= self->eventtabsiz) {

        size_t eventtabsiz = self->eventtabsiz + 1;

        void* tmp = tabresize(self->eventtab,
                              self->eventtabsiz, eventtabsiz,
                              sizeof(self->eventtab[0]),
                              error);
        if (picotm_error_is_set(error)) {
            return;
        }
        self->eventtab = tmp;
        self->eventtabsiz = eventtabsiz;
    }

    struct picotm_event* event = self->eventtab + self->eventtablen;

    event->cookie = cookie;
    event->module = module;
    event->call = call;

    ++self->eventtablen;
}

int
log_apply_events(struct log* self, const struct module* module, bool noundo,
                 struct picotm_error* error)
{
    /* Apply events in chronological order */

    const struct picotm_event* event = self->eventtab;
    const struct picotm_event* event_end = self->eventtab + self->eventtablen;

    while (event < event_end) {

        /* Find consecutive events from same module */

        const struct picotm_event* event2 = event + 1;

        while ((event2 < event_end) && (event->module == event2->module)) {
            ++event2;
        }

        /* Apply vector of events from module */

        ptrdiff_t nevents = event2 - event;

        module_apply_events(module + event->module, event, nevents, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }

        event = event2;
    }

    self->eventtablen = 0;

    return 0;
}

int
log_undo_events(struct log* self, const struct module* module, bool noundo,
                struct picotm_error* error)
{
    /* Undo events in reversed-chronological order */

    const struct picotm_event* event = self->eventtab + self->eventtablen;
    const struct picotm_event* event_end = self->eventtab;

    while (event > event_end) {
        --event;
        module_undo_events(module + event->module, event, 1, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    self->eventtablen = 0;

    return 0;
}
