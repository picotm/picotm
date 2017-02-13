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

