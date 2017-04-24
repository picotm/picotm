/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <stddef.h>
#include "component.h"

int
component_init(struct component *com, int (*lock)(void*),
                                      int (*unlock)(void*),
                                      int (*validate)(void*, int),
                                      int (*apply_event)(const struct event*, size_t, void*),
                                      int (*undo_event)(const struct event*, size_t, void*),
                                      int (*updatecc)(void*, int),
                                      int (*clearcc)(void*, int),
                                      int (*finish)(void*),
                                      int (*uninit)(void*),
                                      void *data)
{
    assert(com);

    com->lock = lock;
    com->unlock = unlock;
    com->validate = validate;
    com->apply_event = apply_event;
    com->undo_event = undo_event;
    com->updatecc = updatecc;
    com->clearcc = clearcc;
    com->finish = finish;
    com->uninit = uninit;
    com->data = data;

    return 0;
}

void
component_uninit(struct component *com)
{
    assert(com);

    if (com->uninit) {
        com->uninit(com->data);
    }

    com->lock = NULL;
    com->unlock = NULL;
    com->validate = NULL;
    com->apply_event = NULL;
    com->undo_event = NULL;
    com->updatecc = NULL;
    com->clearcc = NULL;
    com->finish = NULL;
    com->uninit = NULL;
    com->data = NULL;
}

void *
component_get_data(const struct component *com)
{
    assert(com);

    return com->data;
}

int
component_lock(const struct component *com)
{
    assert(com);

    return com->lock ? com->lock(com->data) : 0;
}

int
component_unlock(const struct component *com)
{
    assert(com);

    return com->unlock ? com->unlock(com->data) : 0;
}

int
component_validate(const struct component *com, int noundo)
{
    assert(com);

    return com->validate ? com->validate(com->data, noundo) : 0;
}

int
component_apply_events(const struct component *com, const struct event *event, size_t n)
{
    assert(com);
    assert(event || !n);

    return com->apply_event ? com->apply_event(event, n, com->data) : 0;
}

int
component_undo_events(const struct component *com, const struct event *event, size_t n)
{
    assert(com);
    assert(event || !n);

    return com->undo_event ? com->undo_event(event, n, com->data) : 0;
}

int
component_updatecc(const struct component *com, int noundo)
{
    assert(com);

    return com->updatecc ? com->updatecc(com->data, noundo) : 0;
}

int
component_clearcc(const struct component *com, int noundo)
{
    assert(com);

    return com->clearcc ? com->clearcc(com->data, noundo) : 0;
}

int
component_finish(const struct component *com)
{
    assert(com);

    return com->finish ? com->finish(com->data) : 0;
}
