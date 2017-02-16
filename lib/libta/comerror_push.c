/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

