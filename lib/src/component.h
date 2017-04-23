/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPONENT_H
#define COMPONENT_H

#include <stddef.h>

struct event;

struct component
{
    int (*lock)(void*);
    int (*unlock)(void*);
    int (*validate)(void*, int);
    int (*apply_event)(const struct event*, size_t, void*);
    int (*undo_event)(const struct event*, size_t, void*);
    int (*updatecc)(void*, int);
    int (*clearcc)(void*, int);
    int (*finish)(void*);
    int (*uninit)(void*);
    int (*tpc_request)(void*, int);
    int (*tpc_success)(void*, int);
    int (*tpc_failure)(void*, int);
    int (*tpc_noundo)(void*, int);
    void *data;
};

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
                                      int (*tpc_request)(void*, int),
                                      int (*tpc_success)(void*, int),
                                      int (*tpc_failure)(void*, int),
                                      int (*tpc_noundo)(void*, int),
                                      void *data);

void
component_uninit(struct component *com);

void *
component_get_data(const struct component *com);

int
component_lock(const struct component *com);

int
component_unlock(const struct component *com);

int
component_validate(const struct component *com, int noundo);

int
component_apply_events(const struct component *com, const struct event *event, size_t n);

int
component_undo_events(const struct component *com, const struct event *event, size_t n);

int
component_updatecc(const struct component *com, int noundo);

int
component_clearcc(const struct component *com, int noundo);

int
component_finish(const struct component *com);

int
component_tpc_request(const struct component *com, int noundo);

int
component_tpc_success(const struct component *com, int noundo);

int
component_tpc_noundo(const struct component *com, int noundo);

int
component_tpc_failure(const struct component *com, int noundo);

#endif

