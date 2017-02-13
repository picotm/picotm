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

#include <assert.h>
#include <sys/types.h>
#include <tanger-stm-ext-actions.h>
#include "ext-actions.h"
#include "comerror.h"

int
com_error_exec_push_commit_error_handler(struct com_error *data, 
                                         stm_error_handler func)
{
    return com_error_inject(data, ACTION_PUSH_ERROR_HANDLER, func);
}

int
com_error_apply_push_commit_error_handler(struct com_error *data, unsigned int cookie)
{
    assert(data);

    return ext_actions_push_commit_error_handler(data->eventtab[cookie]);
}

int
com_error_undo_push_commit_error_handler(struct com_error *data, unsigned int cookie)
{
    return 0;
}

