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
#include <stddef.h>
#include <sys/types.h>
#include <tanger-stm-ext-actions.h>
#include "component.h"
#include "log.h"
#include "ext-actions.h"
#include "comerror.h"

int
com_error_exec_pop_commit_error_handler(struct com_error *data)
{
    return com_error_inject(data, ACTION_POP_ERROR_HANDLER, NULL);
}

int
com_error_apply_pop_commit_error_handler(struct com_error *data, unsigned int cookie)
{
    return ext_actions_pop_commit_error_handler();
}

int
com_error_undo_pop_commit_error_handler(struct com_error *data, unsigned int cookie)
{
    return 0;
}

