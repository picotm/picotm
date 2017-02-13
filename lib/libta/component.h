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

#ifndef COMPONENT_H
#define COMPONENT_H

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

