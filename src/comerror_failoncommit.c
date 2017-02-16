/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

