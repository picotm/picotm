/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <tanger-stm-ext-actions.h>
#include "table.h"
#include "comerror.h"

int
com_error_init(struct com_error *comerror)
{
    assert(comerror);

    comerror->eventtab = NULL;
    comerror->eventtablen = 0;

    return 0;
}

void
com_error_uninit(struct com_error *comerror)
{
    assert(comerror);

    free(comerror->eventtab);
}

int
com_error_inject(struct com_error *data, enum com_error_call call,
                                              stm_error_handler func)
{
    assert(data);

    void *tmp = tabresize(data->eventtab,
                          data->eventtablen,
                          data->eventtablen+1, sizeof(data->eventtab[0]));
    if (!tmp) {
        return -1;
    }
    data->eventtab = tmp;

    data->eventtab[data->eventtablen] = func;

    if (tanger_stm_inject_event(COMPONENT_ERROR, call, data->eventtablen) < 0) {
        return -1;
    }

    return (int)data->eventtablen++;
}

int
com_error_apply_event(struct com_error *data, const struct event *event, size_t n)
{
    extern int com_error_apply_push_commit_error_handler(struct com_error*, unsigned int);
    extern int com_error_apply_pop_commit_error_handler( struct com_error*, unsigned int);
    extern int com_error_apply_failoncommit(             struct com_error*, unsigned int);

    static int (* const apply_func[])(struct com_error*, unsigned int) = {
        com_error_apply_push_commit_error_handler,
        com_error_apply_pop_commit_error_handler,
        com_error_apply_failoncommit};

    assert(event);
    assert(event->call < sizeof(apply_func)/sizeof(apply_func[0]));

    int err = 0;

    while (n && !err) {
        err = apply_func[event->call](data, event->cookie);
        ++event;
        --n;
    }

    return err;
}

int
com_error_undo_event(struct com_error *data, const struct event *event, size_t n)
{
    extern int com_error_undo_push_commit_error_handler(struct com_error*, unsigned int);
    extern int com_error_undo_pop_commit_error_handler( struct com_error*, unsigned int);
    extern int com_error_undo_failoncommit(             struct com_error*, unsigned int);

    static int (* const undo_func[])(struct com_error*, unsigned int) = {
        com_error_undo_push_commit_error_handler,
        com_error_undo_pop_commit_error_handler,
        com_error_undo_failoncommit};

    assert(event);
    assert(event->call < sizeof(undo_func)/sizeof(undo_func[0]));

    int err = 0;

    event += n;

    while (n && !err) {
        --event;
        err = undo_func[event->call](data, event->cookie);
        --n;
    }

    return err;
}

void
com_error_finish(struct com_error *data)
{
    assert(data);

    data->eventtablen = 0;
}

