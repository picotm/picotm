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
#include "comerror.h"

int
com_error_exec_failoncommit(struct com_error *data)
{
    return com_error_inject(data, ACTION_FAILONCOMMIT, NULL);
}

int
com_error_apply_failoncommit(struct com_error *data, unsigned int cookie)
{
    assert(data);

    return -1;
}

int
com_error_undo_failoncommit(struct com_error *data, unsigned int cookie)
{
    return 0;
}

