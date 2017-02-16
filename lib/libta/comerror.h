/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMERROR_H
#define COMERROR_H

enum com_error_call
{
    ACTION_PUSH_ERROR_HANDLER = 0,
    ACTION_POP_ERROR_HANDLER,
    ACTION_FAILONCOMMIT
};

struct com_error
{
    stm_error_handler *eventtab;
    size_t             eventtablen;
};

int
com_error_init(struct com_error *comerror);

void
com_error_uninit(struct com_error *comerror);

int
com_error_inject(struct com_error *data, enum com_error_call call,
                                              stm_error_handler func);

int
com_error_apply_event(struct com_error *data, const struct event *event, size_t n);

int
com_error_undo_event(struct com_error *data, const struct event *event, size_t n);

void
com_error_finish(struct com_error *data);

#endif

